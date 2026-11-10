#pragma once

#define TYPE_WRAPPER_LAMBDA(...) CPP_KOSTYL decltype([] {__VA_ARGS__}())::InstanceT
#define CONCEPT static constexpr bool
#define COWR_INST CPP_KOSTYL CONCEPT Result
#define COWR_USE CPP_KOSTYL template Result