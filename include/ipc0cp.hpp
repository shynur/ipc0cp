#include "ipcator.hpp"
#include "flatbuffers/flatbuffers.h"

#define IPC0CP_LOG_ALLO_OR_DEALLOC(color)  void(  \
    std::clog <<  \
        std::source_location::current().function_name() + "\n"s  \
        + std::vformat(  \
            (color == "green"sv ? "\033[32m" : "\033[31m")  \
            + "\tsize={}, &area={}\033[0m"s,  \
            std::make_format_args(size, (const void *const&)area)  \
        ) + '\n'  \
)

template <IPCator ipcator_t>
struct IPCator_flatbuf: flatbuffers::Allocator {
    ipcator_t ipcator;

    std::uint8_t *allocate(std::size_t size) FLATBUFFERS_OVERRIDE {
        auto area = this->ipcator.allocate(size);
        IPC0CP_LOG_ALLO_OR_DEALLOC("green");
        return (std::uint8_t *)area;
    }

    void deallocate(std::uint8_t *area, std::size_t size) FLATBUFFERS_OVERRIDE {
        IPC0CP_LOG_ALLO_OR_DEALLOC("red");
        this->ipcator.deallocate(area, size);
    }
};


struct Descriptor {
    std::array<char, 248> shm_name;
    const std::size_t offset;
    Descriptor(const std::string_view shm_name, const std::ptrdiff_t offset)
    : shm_name{}, offset(offset) {
        for (auto i = 0u; i != std::size(this->shm_name) - 1; ++i)
            this->shm_name[i] = shm_name[i];
    }
};
