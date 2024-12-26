#include "ipc0cp.hpp"
#include "ipcator.hpp"
#include "laser.fbs.hpp"
#include "flatbuffers/minireflect.h"

auto rd = ShM_Reader{};

int main() {
    std::setbuf(stdout, 0);
    auto _ = +"/ipc0cp-writer-done"_shm;
    auto objd = rd.template read<Descriptor>("/ipc0cp-target-name"sv, 0);
    auto _ = "/ipc0cp-reader-done"_shm[1];

    const auto json = flatbuffers::FlatBufferToString(
        &rd.template read<std::uint8_t>(
            std::string_view{objd.shm_name},
            objd.offset
        ),
        rbk::MessageLaserTypeTable()
    );
    std::cout << "\033[96m"
              << json
              << "\033[0m"
              << std::endl;
}
