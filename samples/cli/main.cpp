#include <thermite.hpp>
#include <thermite/voice_client.hpp>

#include <atomic>
#include <fstream>
#include <iostream>
#include <opus/opus.h>
#include <tbb/concurrent_queue.h>
#include <thread>

// this is a terrible hack.
namespace thermite
{
namespace discord
{
namespace detail
{

// TODO: move this into the main lib somehow to make things easier
uv_loop_t* getUvLoop();

}
}
}

thermite::discord::voice_client client;
OpusEncoder* encoder;
std::ifstream file;
bool speaking;
std::atomic<bool> transcode;

tbb::concurrent_bounded_queue<std::vector<unsigned char>> frames;

constexpr int SAMPLE_RATE = 48000;
constexpr int SAMPLES_PER_MS = SAMPLE_RATE / 1000;
constexpr int CHANNELS = 2;
constexpr int FRAME_MILLIS = 20;
constexpr int FRAME_SIZE = SAMPLES_PER_MS * FRAME_MILLIS;
constexpr int FRAME_BYTES = FRAME_SIZE * CHANNELS * sizeof(opus_int16);

constexpr int MAX_DATA_SIZE = 4096;

void transcode_thread()
{
    while (transcode)
    {
        char data[FRAME_BYTES];
        file.read(data, FRAME_BYTES);
        size_t read = file.gcount();

        if (read < FRAME_BYTES)
        {
            std::cout << "ran over, looping around" << std::endl;
            file.clear();
            file.seekg(0);
            file.read(data + read, FRAME_BYTES - read);
        }

        std::vector<unsigned char> encoded(MAX_DATA_SIZE);
        opus_int16* pcm = reinterpret_cast<opus_int16*>(data);
        opus_int32 encoded_length = opus_encode(
            encoder,
            pcm,
            FRAME_SIZE,
            encoded.data(),
            MAX_DATA_SIZE);

        if (encoded_length > 0)
        {
            encoded.resize(encoded_length);

            frames.push(std::move(encoded));
        }
        else
        {
            std::cout << "opus encode error: " << encoded_length << std::endl;
        }
    }
    std::cout << "transcode thread finished, returning" << std::endl;
}

void send_voice(uv_timer_t*)
{
    if (!speaking)
    {
        speaking = true;
        client.set_speaking(speaking);
    }

    std::vector<unsigned char> data{0xF8, 0xFF, 0xFE};
    if (!frames.try_pop(data))
    {
        std::cout << "failed to pop, sending silence" << std::endl;
    }

    int samples = opus_packet_get_samples_per_frame(data.data(), SAMPLE_RATE);
    client.send_opus_packet(data, samples);
}

int main(int argc, char* argv[])
{
    std::cout << "OPUS FRAME SIZE (ms): "
        << FRAME_SIZE << std::endl;

    if (argc < 7)
    {
        std::cerr << "Usage: " << argv[0]
            << " <server_id> <user_id> <endpoint> <session> <token> <file>"
            << std::endl;

        return 1;
    }

    std::string server_id{argv[1]};
    std::string user_id{argv[2]};
    std::string endpoint{argv[3]};
    std::string session{argv[4]};
    std::string token{argv[5]};
    std::string fileName{argv[6]};

    file.open(fileName, std::ios_base::binary);
    frames.set_capacity(250);

    int status;
    encoder = opus_encoder_create(
        SAMPLE_RATE,
        CHANNELS,
        OPUS_APPLICATION_AUDIO,
        &status);
    if (status != OPUS_OK)
    {
        std::cerr << "Failed to init opus encoder (" << status << ")"
            << std::endl;
        return 1;
    }

    opus_encoder_ctl(encoder, OPUS_SET_SIGNAL(OPUS_SIGNAL_MUSIC));
    opus_encoder_ctl(encoder, OPUS_SET_PACKET_LOSS_PERC(35));
    opus_encoder_ctl(encoder, OPUS_SET_INBAND_FEC(1));
    opus_encoder_ctl(encoder, OPUS_SET_BITRATE(96 * 1024));

    uv_timer_t timer;
    uv_timer_init(thermite::discord::detail::getUvLoop(), &timer);

    speaking = false;
    transcode = true;
    uv_timer_start(&timer, &send_voice, 5000, FRAME_MILLIS);
    client.connect(server_id, user_id, endpoint, session, token);
    std::thread thread{transcode_thread};

    thermite::start_event_loop();
    transcode = false;
    thread.join();

    return 0;
}
