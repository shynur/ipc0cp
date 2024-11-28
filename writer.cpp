#include "./ipc0cp.hpp"

#include <fcntl.h>  // O_{CREAT,RDWR,RDONLY}
#include <unistd.h>

#include <iostream>
#include <cstring>
#include <cassert>

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

    // 等待读者.
    for (volatile auto& flag = shm.c_str[0]; flag == 0; )
        continue;

    std::cout << "writer: 开始写\n";
    {
        rbk4::Message_Laser msg;
        msg.mutable_header()->set_sequence(996);
        msg.SerializeToArray(shm.c_str+2, SHM_SIZE-2);

        volatile auto& flag = shm.c_str[1];
        flag = 1;  // 告诉 reader 可以读数据了.
    }
    std::cout << "writer: 写好了\n";
}
