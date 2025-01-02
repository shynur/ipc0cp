#include "ipc0cp.hpp"
#include "ipcator.hpp"
#include "laser.fbs.hpp"
#include "flatbuffers/minireflect.h"

auto rd = ShM_Reader{};
std::atomic_ref flag{
    *(unsigned char *)map_shm<false, true>("/ipc0cp-shm-atom").addr
};

int main() {
    std::setbuf(stdout, 0);

    for (auto _ : std::views::iota(0) | std::views::take(num_to_send)) {
        while (flag.load(std::memory_order_acquire) == 0)
            continue;

        auto& objd = rd.template read<Descriptor>("/ipc0cp-target"sv, 0);
        auto pobj = &rd.template read<std::uint8_t>(
            std::string_view{objd.shm_name.data()},
            objd.offset
        );
        auto at_read = std::chrono::high_resolution_clock::now();

        flag.store(0, std::memory_order_release);
        /****************************************************************/
        auto at_send = rbk::GetMessageLaser(pobj)->timestamp();
        const auto json = flatbuffers::FlatBufferToString(
            pobj,
            rbk::MessageLaserTypeTable()
        );
        std::cout << "After "
                  << std::chrono::duration_cast<std::chrono::nanoseconds>(
                        at_read.time_since_epoch()
                    ).count() - at_send << "ns: "
                  << "\033[96m"
                  << json
                  << "\033[0m"
                  << std::endl;
    }
}
