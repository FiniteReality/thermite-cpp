#include <fstream>
#include "pstream.h"
#include <thread>
#include <opus/opus.h>
#include <tbb/concurrent_queue.h>

#include <thermite/discord/voice_client.hpp>
#include <thermite/extra/pplx_extras.hpp>

constexpr int SAMPLE_RATE = 48000;
constexpr int SAMPLES_PER_MS = SAMPLE_RATE / 1000;
constexpr int CHANNELS = 2;
constexpr int FRAME_MILLIS = 20;
constexpr int FRAME_SIZE = SAMPLES_PER_MS * FRAME_MILLIS;
constexpr int FRAME_BYTES = FRAME_SIZE * CHANNELS * sizeof(opus_int16);

constexpr int MAX_DATA_SIZE = 1<<16;

tbb::concurrent_bounded_queue<std::vector<uint8_t>> transcoded_frames;

void transcode_work(std::string file)
{
    int status;
    OpusEncoder* encoder = opus_encoder_create(
        SAMPLE_RATE,
        CHANNELS,
        OPUS_APPLICATION_AUDIO,
        &status);

    std::ifstream input_file;

    if (file != "-")
        input_file.open(file, std::ios_base::binary);

    std::istream& input = input_file.is_open() ? input_file : std::cin;

    if (status != OPUS_OK)
    {
        throw std::runtime_error("Couldn't create opus encoder " +
            std::to_string(status));
    }

    opus_encoder_ctl(encoder, OPUS_SET_SIGNAL(OPUS_SIGNAL_MUSIC));
    opus_encoder_ctl(encoder, OPUS_SET_BITRATE(320000));
    opus_encoder_ctl(encoder, OPUS_SET_COMPLEXITY(10));
    opus_encoder_ctl(encoder, OPUS_SET_DTX(1));
    opus_encoder_ctl(encoder, OPUS_SET_INBAND_FEC(1));
    opus_encoder_ctl(encoder, OPUS_SET_PACKET_LOSS_PERC(5));

    while (true)
    {
        char data[FRAME_BYTES];
        std::vector<uint8_t> frame(MAX_DATA_SIZE);

        input.read(data, FRAME_BYTES);
        int read = input.gcount();
        if (read < FRAME_BYTES)
        {
            input.clear();
            input.seekg(0);
            input.read(data + read, FRAME_BYTES - read);
        }

        opus_int16* pcm = reinterpret_cast<opus_int16*>(data);
        opus_int32 encoded_length = opus_encode(
            encoder, pcm, FRAME_SIZE, frame.data(), MAX_DATA_SIZE);

        if (encoded_length > 0)
        {
            frame.resize(encoded_length);

            transcoded_frames.push(std::move(frame));
        }
    }
}

void transmit_work(pplx::task_completion_event<void>& connected,
    thermite::discord::voice_client& client)
{
    pplx::create_task(connected).wait();
    while (true)
    {
        std::vector<uint8_t> frame{
            0xF8, 0xFF, 0xFE,
            0xF8, 0xFF, 0xFE,
            0xF8, 0xFF, 0xFE,
            0xF8, 0xFF, 0xFE,
            0xF8, 0xFF, 0xFE};

        if (transcoded_frames.try_pop(frame))
        {
            int samples = opus_packet_get_samples_per_frame(frame.data(),
                SAMPLE_RATE);
            client.transmit_frame(thermite::memory::memory(frame), samples)
                .then([](std::chrono::milliseconds time)
                {
                    return pplx::wait_for(
                        std::chrono::milliseconds(FRAME_MILLIS) - time);
                })
                .wait();
        }
    }
}

pplx::task<int> main_async(thermite::discord::voice_client& client,
    pplx::task_completion_event<void> connected)
{
    pplx::task_completion_event<void> promise;

    return client.start()
        .then([]
        {
            return pplx::wait_for(std::chrono::milliseconds(5000));
        })
        .then([&client](std::chrono::milliseconds)
        {
            return client.set_speaking(true);
        })
        .then([connected, promise]
        {
            connected.set();
            return pplx::create_task(promise);
        })
        .then([&client]()
        {
            return client.stop();
        })
        .then([promise]
        {
            return 0;
        });
}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " filename" << std::endl;
        return 1;
    }

    std::string file{argv[1]};

    transcoded_frames.set_capacity(250);

    redi::ipstream bot("python3 ./bot.py");

    std::cout << "getting info..." << std::endl;

    std::string guild_id; std::getline(bot, guild_id);
    std::string user_id; std::getline(bot, user_id);
    std::string endpoint; std::getline(bot, endpoint);
    std::string session; std::getline(bot, session);
    std::string token; std::getline(bot, token);

    std::cout
        << "===== Information from bot =====\n"
        << "guild id:   " << guild_id << '\n'
        << "user id:    " << user_id << '\n'
        << "endpoint:   " << endpoint << '\n'
        << "session id: " << session << '\n'
        << "token:      " << token << '\n'
        << std::endl;

    if (token.size() == 0)
        return 1;

    pplx::task_completion_event<void> connected;

    thermite::discord::voice_client client
    {
        guild_id, user_id, endpoint, session, token
    };

    std::thread transcode_thread{transcode_work, file};
    std::thread transmit_thread{transmit_work,
        std::ref(connected), std::ref(client)};

    try
    {
        return main_async(client, connected).get();
    }
    catch (...)
    {
        bot.close();
        throw;
    }
}
