#include <sys/mman.h>  // mmap, shm_open
#include <sys/stat.h>  // fstat
#include <fcntl.h>  // O_{CREAT,RDWR,RDONLY}
#include <cassert>
#include <google/protobuf/arena.h>
#include <unistd.h>  // ftruncate


// Writer 和 readers 需要事先约定好共享内存的位置:
constexpr struct {const char *const shm_name; void *const map_at;} INIT_OPTIONS{
    .shm_name = "/Wryyyyyy",
    .map_at = static_cast<void *>(nullptr) + 0x500000u,
};


struct ShM {
    const std::string name;
    void *const start;  // 共享内存要映射到进程地址空间的位置.
    std::size_t len = 4096;  // 只有 writer 关心 length.
    const bool constructed_by_writer;

    ShM(const std::string name, void *const start, const bool is_writer = false)
        : name{name}, start{start}, constructed_by_writer{is_writer} {
        const auto fd 
          = is_writer 
            ? shm_open(name.c_str(), O_CREAT | O_RDWR, 0666)
            : [&name]{
                // 忙等, 直到 writer 创建好共享内存.
                while (true)
                    if (const auto fd = shm_open(name.c_str(), O_RDWR, 0666); fd != -1)
                        return fd;
            }();
        assert("创建共享内存" && fd != -1);

        if (is_writer) {
            const auto result_resize = ftruncate(fd, this->len);
            assert("设置共享内存大小" && result_resize != -1);
        } else  // 等待 writer 设置好共享内存的大小.
            for (struct stat fshm; true; )
                if (fstat(fd, &fshm); fshm.st_size)
                    break;

        const auto *const result_mmap = mmap(start, this->len, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FIXED, fd, 0);
        close(fd);  // 映射完立即关闭, 对后续操作没啥影响.
        assert("映射共享内存" && result_mmap != MAP_FAILED);

        // 不需要清零, 因为 `ftruncate' 自带清零行为.
        // std::memset(c_str, 0, SHM_SIZE);
    }

    ~ShM() {
        if (this->constructed_by_writer)
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
