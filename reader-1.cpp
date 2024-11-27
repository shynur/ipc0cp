#include "./ipc0cp.hpp"
#include <cassert>

#include <iostream>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
 
int main() {
    struct ShM {
        char *const c_str = []{
            const int fd = []{
                int fd;
                while ((fd = shm_open(SHM_NAME, O_RDONLY, 0666)) == -1)
                    continue;
                return fd;
            }();
 
            void *const addr = mmap(nullptr, SHM_SIZE, PROT_READ, MAP_SHARED, fd, 0);
            assert("映射共享内存" && addr != MAP_FAILED);

            return static_cast<char *>(addr);
        }();

        ~ShM() { munmap(c_str, SHM_SIZE); }
    } shm;

    while (shm.c_str[0] == '\0')
        continue;
         
    std::cout << "开始读... "
              << shm.c_str
              << std::endl;
}
