#include "./ipc0cp.hpp"
#include "./laser.pb.h"

#include <iostream>


int main() {
    ShM shm{INIT_OPTIONS.shm_name, INIT_OPTIONS.map_at};

    // 等待数据到来.
    while (static_cast<volatile std::uint8_t&>(shm[0]) == 0)
        continue;

    std::cerr << "reader: 开始读\n"
              << [&shm]{
                   const auto offset = *static_cast<std::size_t *>(shm.start + 32);
                   static_cast<volatile std::uint8_t&>(shm[1]) = 1;  // 告诉 writer 我读完了.
                   return static_cast<rbk4::Message_Laser *>(shm.start + offset);
                 }()->header().channel()
              << '\n';
}
