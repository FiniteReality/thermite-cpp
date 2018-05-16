#include <fstream>
#include <iostream>
#include <opus/opus.h>

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

#define SAMPLE_RATE 48000
#define CHANNELS 2
#define FRAME_MILLIS 20
#define SAMPLE_BYTES (CHANNELS * sizeof(opus_int16))
#define FRAME_SAMPLES_PER_CHANNEL (SAMPLE_RATE / 1000 * FRAME_MILLIS)
#define FRAME_BYTES (FRAME_SAMPLES_PER_CHANNEL * SAMPLE_BYTES)

#define MAX_DATA_SIZE 4096

void send_voice(uv_timer_t*)
{
    if (!speaking)
    {
        speaking = true;
        client.set_speaking(speaking);
    }

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

    if (encoded_length > 0) {
        encoded.resize(encoded_length);
        client.sendOpusFrame(encoded, FRAME_MILLIS);
    } else {
        std::cout << "opus error: " << encoded_length << std::endl;
    }
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
    opus_encoder_ctl(encoder, OPUS_SET_PACKET_LOSS_PERC(20));
    opus_encoder_ctl(encoder, OPUS_SET_INBAND_FEC(1));
    opus_encoder_ctl(encoder, OPUS_SET_BITRATE(96 * 1024)); // 96kb/s

    uv_timer_t timer;
    uv_timer_init(thermite::discord::detail::getUvLoop(), &timer);

    speaking = false;
    uv_timer_start(&timer, &send_voice, 5000, FRAME_MILLIS);
    client.connect(server_id, user_id, endpoint, session, token);

    thermite::start_event_loop();

    return 0;
}
