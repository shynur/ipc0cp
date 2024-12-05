#include "ipc0cp.hpp"
#include "laser-by-flatc.hpp"
#include "flatbuffers/reflection.h"
#include "flatbuffers/minireflect.h"

#include <thread>
#include <cstdio>
#include <type_traits>


using namespace std::chrono_literals;

ShM<true> shm{INIT_OPTIONS.shm_name,};


int main() {
    flatbuffers::FlatBufferBuilder builder{1024, new RBKator{&shm[128]}};
    builder.ForceDefaults(true);
    auto header = rbk::CreateMessageHeader(
        builder, builder.CreateString("我来组成头部"), 42
    );
    auto beam1 = rbk::CreateMessageBeam(builder, 9.96, false);
    auto beams = builder.CreateVector(std::vector{beam1,});
    auto laser = rbk::CreateMessageLaser(builder, header, beams);
    builder.Finish(laser);

    auto *const buf = builder.GetBufferPointer();
    reinterpret_cast<volatile std::ptrdiff_t&>(shm[0])
        = buf - static_cast<std::uint8_t *>(shm+0);

    std::this_thread::sleep_for(0.7s);

    const auto txt = flatbuffers::FlatBufferToString(
        buf,
        rbk::MessageLaserTypeTable()
    );
    std::cerr << "From Writer:\n\t" << txt << '\n';
}
