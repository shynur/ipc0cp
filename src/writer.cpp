#include "ipc0cp.hpp"
#include "ipcator.hpp"
#include "laser.fbs.hpp"

std::atomic_ref flag{
    *(unsigned char *)map_shm<true>("/ipc0cp-shm-atom", 1u)
};
auto name_passer = "/ipc0cp-target"_shm[sizeof(Descriptor)];
auto flatbuf_allocator = IPCator_flatbuf<Monotonic_ShM_Buffer>{};

void add_epoch_then_send(auto& builder, const auto& obj) {
    builder.Finish(rbk::CreateMessageLaser(
        builder,
        std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::high_resolution_clock::now().time_since_epoch()
        ).count(),
        obj
    ));

    auto *const buf = builder.GetBufferPointer();
    auto& target_shm = *flatbuf_allocator.ipcator
                       .upstream_resource()
                       ->last_inserted;
    new (std::data(name_passer)) Descriptor{
        target_shm.get_name(),
        buf - std::to_address(std::begin(target_shm)),
    };

    flag.store(~(-1 << num_readers), std::memory_order_release);
    while (flag.load(std::memory_order_acquire))
        continue;
}

int main() {
    for (auto _ : std::views::iota(0) | std::views::take(num_to_send)) {
        auto builder = flatbuffers::FlatBufferBuilder{65536, &flatbuf_allocator};
        // builder.ForceDefaults(true);
        auto beams = builder.CreateVector(std::vector{
            rbk::CreateMessageBeam(builder, 9.96, false),
        });
        auto header = rbk::CreateMessageHeader(
            builder, builder.CreateString(std::string(30'000, '6')), beams
        );
        add_epoch_then_send(builder, header);
    }
}
