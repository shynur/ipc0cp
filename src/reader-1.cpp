#include "ipc0cp.hpp"
#include "laser-by-flatc.hpp"
#include "flatbuffers/reflection.h"
#include "flatbuffers/minireflect.h"

#include <iostream>
#include <cstdio>


ShM shm{INIT_OPTIONS.shm_name,};


int main() {
    auto *const buf
        = static_cast<std::uint8_t *>(shm+0)
          + reinterpret_cast<volatile std::ptrdiff_t&>(shm[0]);
    const auto txt = flatbuffers::FlatBufferToString(
        buf,
        rbk::MessageLaserTypeTable()
    );
    std::cerr << "From Reader:\n\t" << txt << '\n';


    auto laser = rbk::GetMutableMessageLaser(buf);

    auto channel = laser->mutable_header()->mutable_channel()->c_str();
    std::strncpy(const_cast<char *>(channel), "007", 4);

    laser->mutable_beams()->GetMutableObject(0)->mutate_valid(true);
}
