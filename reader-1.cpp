#include "./ipc0cp.hpp"

#include <fcntl.h>  // O_RDONLY
#include <unistd.h>

#include <iostream>
#include <cstring>
#include <cassert>


int main() {
    struct ShM {
        char *const c_str = []{
            const int fd = []{
                int fd;
                do {
                    fd = shm_open(SHM_NAME, O_RDONLY, 0666);
                } while (fd == -1);
                return fd;
            }();
 
            void *const addr = mmap(nullptr, SHM_SIZE, PROT_READ, MAP_SHARED, fd, 0);
            close(fd);  // 映射完立即关闭, 对后续操作没啥影响.
            assert("映射共享内存" && addr != MAP_FAILED);

            return static_cast<char *>(addr);
        }();

        ~ShM() { munmap(c_str, SHM_SIZE); }
    } shm;

    while (shm.c_str[0] == '\0')
        continue;

    std::cout << "reader: 开始读\n"
              << "reader: \n" + [&shm]{
                  rbk4::Message_Laser msg;
                  msg.ParseFromArray(static_cast<const void *>(shm.c_str), SHM_SIZE);
                  return msg.DebugString();
              }() + '\n';
}
