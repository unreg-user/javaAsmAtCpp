module;

#include <cstdint>
#include "cstd/macro.h"

export module compl_flags;

export struct cflags {
    struct class_file_ptr {
        COM_BOOL checkMemorySize = true;
    };
};