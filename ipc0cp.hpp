#include <sys/mman.h>
#include "./laser.pb.h"
#include <sys/stat.h>
#include <google/protobuf/arena.h>


constexpr auto SHM_NAME = "/Wryyyyyy";
constexpr auto SHM_SIZE = 4096u;
