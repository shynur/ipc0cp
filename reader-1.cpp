#include "./ipc0cp.hpp"
#include "./laser.pb.h"
#include <google/protobuf/util/json_util.h>
#include <capnp/message.h>
#include "./laser.capnp.h"

#include <iostream>
#include <cstdio>


ShM shm{INIT_OPTIONS.shm_name, INIT_OPTIONS.map_at};

template <class protobuf_msg_t> const protobuf_msg_t& receive() {
    // 等待数据到来.
    while (static_cast<volatile std::uint8_t&>(shm[0]) == 0)
        continue;

    std::cerr << "reader: 开始读\n";
    const auto offset = *static_cast<std::size_t *>(shm.start + 32);
    static_cast<volatile std::uint8_t&>(shm[0]) = 0;  // 告诉 writer 我读完了.

    const auto& msg = reinterpret_cast<protobuf_msg_t&>(shm[offset]);
    std::fprintf(stderr, "reader: 读好了 %p\n", &msg);
    return msg;
}

int main() {
    const auto& header 
        = reinterpret_cast<decltype(capnp::MallocMessageBuilder{}.initRoot<rbk4::MessageHeader>())&>(shm[256]);

    std::cerr << "get: " << header.toString().flatten().cStr() <<'\n';
}

void test_protobuf() {
    static std::string JSON;

    std::fprintf(stderr, "%lu\n\n", 
                 receive<rbk4::Message_Header>().timestamp());
    //google::protobuf::util::MessageToJsonString(receive<rbk4::Message_Header>(), &JSON);
    //std::fprintf(stderr, "%s\n\n", JSON.c_str());

    std::fprintf(stderr, "%lu\n\n", 
                 receive<rbk4::Message_Laser>().header().timestamp());

    for (const auto beam : receive<rbk4::Message_Laser>().beams())
        std::fprintf(stderr, "(%f, %s)\n", 
                     beam.angle(), beam.valid() ? "true" : "false");
}
