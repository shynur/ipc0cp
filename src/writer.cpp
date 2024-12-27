#include "ipc0cp.hpp"
#include "ipcator.hpp"
#include "laser.fbs.hpp"

auto name_passer = "/ipc0cp-target-name"_shm[sizeof(Descriptor)];
auto flatbuf_allocator = IPCator_flatbuf<Monotonic_ShM_Buffer>{};

int main() {
    std::setbuf(stdout, 0);
    flatbuffers::FlatBufferBuilder builder{0, &flatbuf_allocator};

    builder.ForceDefaults(true);
    auto header = rbk::CreateMessageHeader(
        builder, builder.CreateString("1! 5!  Bro here rap for U~"), 42
    );
    auto beam1 = rbk::CreateMessageBeam(builder, 9.96, false);
    auto beams = builder.CreateVector(std::vector{beam1,});
    auto laser = rbk::CreateMessageLaser(builder, header, beams);
    builder.Finish(laser);

    auto *const buf = builder.GetBufferPointer();
    auto& target_shm = *flatbuf_allocator.ipcator
                       .upstream_resource()
                       ->last_inserted;
    new (std::data(name_passer)) Descriptor{
        target_shm.get_name(),
        buf - std::to_address(std::begin(target_shm)),
    };
    auto _ = "/ipc0cp-writer-done"_shm[1];
    +"/ipc0cp-reader-done"_shm;
}
