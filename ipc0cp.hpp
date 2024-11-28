#include <sys/mman.h>
#include "./laser.pb.h"
#include <sys/stat.h>


constexpr auto SHM_NAME = "/Wryyyyyy";
constexpr auto SHM_SIZE = 4096u;
