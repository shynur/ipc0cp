#include "ipc0cp.hpp"
#include "ipcator.hpp"
#include "laser.fbs.hpp"

auto name_passer = "/ipc0cp-target"_shm[sizeof(Descriptor)];
auto flatbuf_allocator = IPCator_flatbuf<Monotonic_ShM_Buffer>{};
std::atomic_ref flag{
    *(unsigned char *)map_shm<true>("/ipc0cp-shm-atom", 1u)
};

void add_epoch_then_send(auto& builder, const auto& obj) {
    builder.Finish(rbk::CreateMessageLaser(
        builder,
        std::chrono::duration_cast<std::chrono::nanoseconds>(
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

    flag.store(1, std::memory_order_release);
    while (flag.load(std::memory_order_acquire) == 1)
        continue;
}

int main() {
    std::setbuf(stdout, 0);
    std::this_thread::sleep_for(0.15s);

    for (auto _ : std::views::iota(0) | std::views::take(num_to_send)) {
        auto builder = flatbuffers::FlatBufferBuilder{256, &flatbuf_allocator};
        // builder.ForceDefaults(true);
        auto beams = builder.CreateVector(std::vector{
            rbk::CreateMessageBeam(builder, 9.96, false),
        });
        auto header = rbk::CreateMessageHeader(
            builder, builder.CreateString("1! 5!  Bro here rap for U~"), beams
        );
        add_epoch_then_send(builder, header);
    }
}
