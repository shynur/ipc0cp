#include "./ipc0cp.hpp"
#include <cassert>

#include <iostream>
#include <cstring>
#include <fcntl.h>  // O_{CREAT,RDWR,RDONLY}
#include <unistd.h>


int main() {
    const struct ShM {
        char *const c_str = []{
            const int fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
            assert("打开共享内存" && fd != -1);

            assert("设置共享内存大小" && ftruncate(fd, SHM_SIZE) != -1);

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

    std::cout << "开始写... ";
    std::strncpy(shm.c_str, "Hello, readers!", SHM_SIZE);
    std::cout << "写好了, 按 Enter 退出... " << std::endl;
    std::cin.get();
}
