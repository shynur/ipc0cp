#include "./ipc0cp.hpp"

#include <fcntl.h>  // O_RDWR
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
                    fd = shm_open(SHM_NAME, O_RDWR, 0666);
                } while (fd == -1);
                return fd;
            }();
 
            while (true) {
                struct stat fshm;
                fstat(fd, &fshm);
                if (fshm.st_size)
                    break;
            }
            void *const addr = mmap(nullptr, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
            close(fd);  // 映射完立即关闭, 对后续操作没啥影响.
            assert("映射共享内存" && addr != MAP_FAILED);

            return static_cast<char *>(addr);
        }();

        ~ShM() { munmap(c_str, SHM_SIZE); }
    } shm;

    volatile auto& flag = shm.c_str[0];
    flag = 1;  // 告诉 writer 我已经准备好读取数据了.

    // 等待数据到来.
    for (volatile auto& flag = shm.c_str[1]; flag == 0; )
        continue;

    std::cout << "reader: 开始读\n"
              << "reader: \n\n" + [&shm]{
                  rbk4::Message_Laser msg;
                  msg.ParseFromArray(static_cast<const void *>(shm.c_str+2), SHM_SIZE-2);
                  return msg.DebugString();
              }() + '\n';
}
