#include <atomic>
#include <fstream>
#include <iostream>
#include <opus/opus.h>
#include <tbb/concurrent_queue.h>
#include <thread>

#include <thermite.hpp>
#include <thermite/voice_client.hpp>

// this is a terrible hack.
namespace thermite
{
namespace discord
{
namespace detail
{

uv_loop_t* getUvLoop();

}
}
}

thermite::discord::voice_client client;
OpusEncoder* encoder;
std::ifstream file;
bool speaking;
uv_work_t work;
tbb::concurrent_bounded_queue<std::vector<unsigned char>> buffers;

#define SAMPLE_RATE 48000
#define CHANNELS 2
#define FRAME_MILLIS 20
#define SAMPLE_BYTES (CHANNELS * sizeof(opus_int16))
#define FRAME_SAMPLES_PER_CHANNEL (SAMPLE_RATE / 1000 * FRAME_MILLIS)
#define FRAME_SAMPLES (FRAME_SAMPLES_PER_CHANNEL * CHANNELS)
#define FRAME_BYTES (FRAME_SAMPLES_PER_CHANNEL * SAMPLE_BYTES)

#define MAX_DATA_SIZE 2048

void transcode(uv_work_t*)
{
    std::cout << "transcoding a frame" << std::endl;

    char data[FRAME_BYTES];
    opus_int16* pcm = reinterpret_cast<opus_int16*>(data);
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
    ssize_t encoded_length = opus_encode(
        encoder,
        pcm,
        FRAME_SAMPLES_PER_CHANNEL,
        encoded.data(),
        MAX_DATA_SIZE);

    encoded.resize(encoded_length);
    if (buffers.size() < buffers.capacity())
        buffers.push(std::move(encoded));
}

void do_send(uv_work_t*, int)
{
    std::vector<unsigned char> buffer;
    buffers.pop(buffer);
    client.sendOpusFrame(buffer, FRAME_MILLIS);
}

void send_voice(uv_timer_t*)
{
    if (!speaking)
    {
        speaking = true;
        client.set_speaking(speaking);
    }

    std::cout << "sending data" << std::endl;

   /*uv_queue_work(
        thermite::discord::detail::getUvLoop(),
        &work,
        &transcode,
        &do_send);*/
}

int main(int argc, char* argv[])
{
    std::cout << "OPUS FRAME SIZE (ms): "
        << FRAME_SAMPLES_PER_CHANNEL << std::endl;

    if (argc < 7) {
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

    file.open(fileName);
    buffers.set_capacity(25);

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

    opus_encoder_ctl(encoder, OPUS_SET_BITRATE(OPUS_BITRATE_MAX));
    opus_encoder_ctl(encoder, OPUS_SET_COMPLEXITY(10));
    opus_encoder_ctl(encoder, OPUS_SET_SIGNAL(OPUS_SIGNAL_MUSIC));
    opus_encoder_ctl(encoder, OPUS_SET_PACKET_LOSS_PERC(20));

    uv_timer_t timer;
    uv_timer_init(thermite::discord::detail::getUvLoop(), &timer);

    speaking = false;
    uv_timer_start(&timer, &send_voice, 5000, FRAME_MILLIS);
    client.connect(server_id, user_id, endpoint, session, token);

    thermite::start_event_loop();

    return 0;
}
