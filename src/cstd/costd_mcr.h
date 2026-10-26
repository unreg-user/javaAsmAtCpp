#pragma once

#define TYPE_WRAPPER_LAMBDA(...) decltype([] {__VA_ARGS__}())::InstanceT
#define CONCEPT static constexpr bool