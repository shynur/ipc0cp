#include "./ipc0cp.hpp"

#include <fcntl.h>  // O_{CREAT,RDWR,RDONLY}
#include <unistd.h>

#include <iostream>
#include <cstring>
#include <cassert>
#include <type_traits>


int main() {
    const struct ShM {
        char *const c_str = []{
            const int fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
            assert("打开共享内存" && fd != -1);

            assert("设置共享内存大小" && ftruncate(fd, SHM_SIZE) != -1);
            //                           ^^^^^^^^^^^^^^^^^^^^^^^

            void *const addr = mmap(nullptr, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
            close(fd);  // 映射完立即关闭, 对后续操作没啥影响.
            assert("映射共享内存" && addr != MAP_FAILED);

            return static_cast<char *>(addr);
        }();

        ShM() {
            // 不需要清零, 因为 `ftruncate' 自带清零行为.
            // std::memset(c_str, 0, SHM_SIZE);
        }
        ~ShM() {
            shm_unlink(SHM_NAME);
            // 至此, 当所有 mmap 都取消映射后, 共享内存将被 deallocate.
            // 新的 `shm_open' 尝试都将失败.

            munmap(c_str, SHM_SIZE);
        }
    } shm;

    google::protobuf::Arena arena{[&shm]{
        google::protobuf::ArenaOptions options;

        // 把 100th 字节往后的区域分配给 Arena.
        options.initial_block = shm.c_str + 100,
        options.initial_block_size = SHM_SIZE - 100;

        constexpr auto never = +[] [[noreturn]] { 
            assert(false && "Arena 不应该 realloc");
        };
        options.block_alloc = +[](std::size_t) -> void * { never(); },
        options.block_dealloc = +[](void *, std::size_t) { never(); };

        return options;
    }()};

    // 等待读者.
    for (volatile auto& flag = shm.c_str[0]; flag == 0; )
        continue;

    std::cout << "writer: 开始写\n";
    {
        const auto msg = rbk4::Message_Laser{}.New(&arena);

        std::cerr << msg << " : " << static_cast<void *>(shm.c_str) << '\n';

        msg->mutable_header()->set_sequence(996);
        *reinterpret_cast<std::size_t *>(shm.c_str + 2)
            = reinterpret_cast<char *>(msg) - shm.c_str;

        // 告诉 reader 可以读数据了.
        volatile auto& flag = shm.c_str[1];
        flag = 1;
    }
    std::cout << "writer: 写好了\n";
}
