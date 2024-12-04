#include <sys/mman.h>  // mmap, shm_open
#include <sys/stat.h>  // fstat
#include <fcntl.h>  // O_{CREAT,RDWR,RDONLY}
#include <cassert>
#include <unistd.h>  // ftruncate
#include <string_view>
#include <string>
#include <cstring>


// Writer 和 readers 需要事先约定好共享内存的位置:
constexpr struct {
    const char *const shm_name = "/Wryyyyyy"; 
    void *const map_at = static_cast<void *>(nullptr) + 0x500000u;
} INIT_OPTIONS{};


template<bool creat = false>
struct ShM {
    const std::string name;
    void *const start;  // 共享内存要映射到进程地址空间的位置.
    std::size_t len = 4096;  // 只有 writer 需要关心 length; 必须是页表大小的整数倍.

    ShM(const std::string_view name, void *const start): name{name}, start{start} {
        const auto fd = [name=this->name.c_str()]{
            if constexpr (creat)
                return shm_open(name, O_CREAT | O_RDWR, 0666);
            else
                // 忙等, 直到 writer 创建好共享内存.
                while (true)
                    if (const auto fd = shm_open(name, O_RDWR, 0666); fd != -1)
                        return fd;
        }();
        assert("创建共享内存" && fd != -1);

        if constexpr (creat) {
            const auto result_resize = ftruncate(fd, this->len);
            assert("设置共享内存大小" && result_resize != -1);
        } else  // 等待 writer 设置好共享内存的大小.
            for (struct stat fshm; true; )
                if (fstat(fd, &fshm); fshm.st_size)
                    break;

        const auto *const result_mmap = mmap(start, this->len, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FIXED, fd, 0);
        close(fd);  // 映射完立即关闭, 对后续操作没啥影响.
        assert("映射共享内存" && result_mmap != MAP_FAILED);

        // 实际不需要清零.
        std::memset(start, 0, this->len);
    }

    ~ShM() {
        if constexpr (creat)
            shm_unlink(this->name.c_str());
            // 此后, 新的 `shm_open' 尝试都将失败;
            // 当所有 mmap 都取消映射后, 共享内存将被 deallocate.

        munmap(this->start, this->len);
    }

    std::uint8_t& operator[](const std::size_t i) const {
        assert(i < this->len);
        return static_cast<std::uint8_t *>(this->start)[i];
    }
};
