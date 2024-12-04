#include "./ipc0cp.hpp"
#include "./laser.pb.h"
#include <google/protobuf/arena.h>
#include <capnp/message.h>
#include "./laser.capnp.h"

#include <thread>
#include <cstdio>
#include <type_traits>


using namespace std::chrono_literals;


ShM<true> shm{INIT_OPTIONS.shm_name, INIT_OPTIONS.map_at};


void send(const auto& msg) {
    std::fprintf(stderr, "writer: 开始写 %p\n", &msg);

    // 把 msg 的 offset 放在 shm[32] 处:
    reinterpret_cast<std::size_t&>(shm[32])
        = reinterpret_cast<char *>(const_cast<std::decay_t<decltype(msg)> *>(&msg))
          - static_cast<char *>(shm+0);

    static_cast<volatile std::uint8_t&>(shm[0]) = 1;  // 告诉 reader 可以读数据了.

    std::fprintf(stderr, "writer: 写好了 %p\n", &msg);

    // 等 reader 读完.
    while (static_cast<volatile std::uint8_t&>(shm[0]))
        continue;
}

void test_protobuf() {
    // Arena 放到 shm[256] 处.
    auto& arena = *new(shm + 256) google::protobuf::Arena{[]{
        google::protobuf::ArenaOptions options;

        // 把 shm[512+] 的区域分配给 Arena.
        options.initial_block = static_cast<char *>(shm + 512),
        options.initial_block_size = shm.len - 512;

        constexpr auto never = +[] [[noreturn]] { 
            assert(false && "Arena 不应该 realloc");
        };
        options.block_alloc = +[](std::size_t) -> void * { never(); },
        options.block_dealloc = +[](void *, std::size_t) { never(); };

        return options;
    }()};

    // 测试被直接包含的 non-string 字段:
    auto& test_direct_u64 = *arena.CreateMessage<rbk4::Message_Header>(&arena);
    test_direct_u64.set_timestamp(996);
    send(test_direct_u64);

    // 测试被间接包含的 non-string 字段:
    auto& test_indirect_u64 = *arena.CreateMessage<rbk4::Message_Laser>(&arena);
    test_indirect_u64.mutable_header()->set_timestamp(007);
    send(test_indirect_u64);

    // 测试被间接包含的 repeated 字段:
    auto& test_indirect_repeared = *arena.CreateMessage<rbk4::Message_Laser>(&arena);
    for (auto i = 0; i != 3; ++i) {
        const auto beam = test_indirect_repeared.add_beams();
        beam->set_angle(i * 10),
        beam->set_valid(i % 2);
    }
    send(test_indirect_repeared);
}

void test_capnproto();

int main() {
    test_capnproto();
    std::this_thread::sleep_for(0.5s);
}

void test_capnproto() {
    auto& header
        = reinterpret_cast<decltype(capnp::MallocMessageBuilder{}.initRoot<rbk4::MessageHeader>())&>(shm[256]) 
        = capnp::MallocMessageBuilder{kj::ArrayPtr{static_cast<capnp::word *>(shm + 512), 100}}
          .initRoot<rbk4::MessageHeader>();
    header.setChannel("123"),
    header.setTimestamp(996);

    std::cerr << "post: " << header.toString().flatten().cStr() <<'\n';
}
