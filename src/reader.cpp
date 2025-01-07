#include "ipc0cp.hpp"
#include "ipcator.hpp"
#include "laser.fbs.hpp"
#include "flatbuffers/minireflect.h"

constexpr auto reader_id = __FILE__[sizeof(__FILE__)-6] - '1';

auto rd = ShM_Reader{};
char final_output[BUFSIZ];

int main() {
    std::setbuf(stdout, final_output);
    std::cout.sync_with_stdio(false);
    std::this_thread::sleep_for(10ms);
    std::atomic_ref flag{
        *(unsigned char *)map_shm<false>("/ipc0cp-shm-atom").addr
    };

    for (auto _ : std::views::iota(0) | std::views::take(num_to_send)) {
        while ((flag.load(std::memory_order_acquire) & (1 << reader_id)) == 0)
            continue;

        auto& objd = rd.template read<Descriptor>("/ipc0cp-target"sv, 0);
        auto pobj = &rd.template read<std::uint8_t>(
            std::string_view{objd.shm_name.data()},
            objd.offset
        );
        /*const auto json = flatbuffers::FlatBufferToString(
            pobj,
            rbk::MessageLaserTypeTable()
        );*/
        auto at_read = std::chrono::high_resolution_clock::now();  // 应该排除该语句的执行时间.

        auto at_send = rbk::GetMessageLaser(pobj)->timestamp();
        std::cout << __FILE__ << ": after "
                  << std::to_string(
                        std::chrono::duration_cast<std::chrono::microseconds>(
                            at_read.time_since_epoch()
                        ).count() - at_send
                    ) << "us... "
                  // + "\033[96m" + json + "\033[0m"
                  << '\n';
        flag.fetch_sub(1 << reader_id, std::memory_order_release);  // 应该排除该语句的执行时间.
    }
}
