#include "./ipc0cp.hpp"
#include "./laser.pb.h"

#include <cstdio>
#include <type_traits>


ShM<true> shm{INIT_OPTIONS.shm_name, INIT_OPTIONS.map_at};

void send(const auto *const msg) {
    std::fprintf(stderr, "writer: 开始写 %p\n", msg);

    // 把 msg 的 offset 放在 shm[32] 处:
    *static_cast<std::size_t *>(shm.start + 32)
        = reinterpret_cast<char *>(const_cast<std::decay_t<decltype(*msg)> *>(msg))
          - reinterpret_cast<char *>(shm.start);

    static_cast<volatile std::uint8_t&>(shm[0]) = 1;  // 告诉 reader 可以读数据了.

    std::fprintf(stderr, "writer: 写好了 %p\n", msg);

    // 等 reader 读完.
    while (static_cast<volatile std::uint8_t&>(shm[0]))
        continue;
}

int main() {
    // Arena 放到 shm[256] 处.
    auto *const arena = new(&shm[256]) google::protobuf::Arena{[]{
        google::protobuf::ArenaOptions options;

        // 把 shm[512+] 的区域分配给 Arena.
        options.initial_block = reinterpret_cast<char *>(&shm[512]),
        options.initial_block_size = shm.len - 512;

        constexpr auto never = +[] [[noreturn]] { 
            assert(false && "Arena 不应该 realloc");
        };
        options.block_alloc = +[](std::size_t) -> void * { never(); },
        options.block_dealloc = +[](void *, std::size_t) { never(); };

        return options;
    }()};

    // 测试被直接包含的 non-string 字段:
    const auto test_direct_u64 = arena->CreateMessage<rbk4::Message_Header>(arena);
    test_direct_u64->set_timestamp(996);
    send(test_direct_u64);

    // 测试被间接包含的 non-string 字段:
    const auto test_indirect_u64 = arena->CreateMessage<rbk4::Message_Laser>(arena);
    test_indirect_u64->mutable_header()->set_timestamp(007);
    send(test_indirect_u64);

    // 测试被间接包含的 repeated 字段:
    const auto test_indirect_repeared = arena->CreateMessage<rbk4::Message_Laser>(arena);
    for (auto i = 0; i != 10; ++i) {
        const auto beam = test_indirect_repeared->add_beams();
        beam->set_angle(i * 10),
        beam->set_valid(i % 2);
    }
    send(test_indirect_repeared);
}
