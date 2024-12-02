#include "./ipc0cp.hpp"
#include "./laser.pb.h"

#include <iostream>


int main() {
    ShM shm{INIT_OPTIONS.shm_name, INIT_OPTIONS.map_at, true};

    // Arena 放到 256th byte 处.
    const auto arena = new(&shm[256]) google::protobuf::Arena{[&shm]{
        google::protobuf::ArenaOptions options;

        // 把 512nd 字节往后的区域分配给 Arena.
        options.initial_block = reinterpret_cast<char *>(&shm[512]),
        options.initial_block_size = shm.len - 512;

        constexpr auto never = +[] [[noreturn]] { 
            assert(false && "Arena 不应该 realloc");
        };
        options.block_alloc = +[](std::size_t) -> void * { never(); },
        options.block_dealloc = +[](void *, std::size_t) { never(); };

        return options;
    }()};

    std::cerr << "writer: 开始写\n";
    {
        const auto msg = arena->CreateMessage<rbk4::Message_Laser>(arena);
        msg->mutable_header()->set_timestamp(996),
        msg->mutable_header()->set_channel("007 12345678");

        // 把 msg 的 offset 放在 32nd 处:
        *static_cast<std::size_t *>(shm.start + 32)
          = reinterpret_cast<char *>(msg) - reinterpret_cast<char *>(shm.start);

        static_cast<volatile std::uint8_t&>(shm[0]) = 1;  // 告诉 reader 可以读数据了.
    }
    std::cerr << "writer: 写好了\n";

    // 等 reader 读完.
    while (static_cast<volatile std::uint8_t&>(shm[1]) == 0)
        continue;
}
