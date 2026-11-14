module;

#include "../../../cstd/gstd_mcr.h"
#include <concepts>

export module label;

import gstd;
import simple_cstd;
import costd;

export namespace parser::mcode::lbl {
    template <typename List>
    using ProgPtrByList = List::Node*;
}