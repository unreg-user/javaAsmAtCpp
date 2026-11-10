#pragma once

#include "../../../cstd/macro.h"

#define C_FLAG(name, value) COM bool name = value;
#define C_FLAG_E(name) C_FLAG(name, true)
#define C_FLAG_D(name) C_FLAG(name, false)
#define PARSE_LEN(len) COM parser::ParseLength parseLength = len;