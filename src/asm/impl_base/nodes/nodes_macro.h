#pragma once

#define C_FLAG(name, value) static constexpr bool name = value;
#define C_FLAG_E(name) C_FLAG(name, true)
#define C_FLAG_D(name) C_FLAG(name, false)