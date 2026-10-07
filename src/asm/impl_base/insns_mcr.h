#pragma once

#define GET_ASBT_INSN_FM(fmName) deref_visit([](const AbstractInsnNode& _node) noexcept { return _node.fmName; })
#define SET_ASBT_INSN_FIELD(fieldName, ...)                                                                            \
deref_visit([](AbstractInsnNode& _node) noexcept { _node.fieldName = __VA_ARGS__; })
#define NAME_IN_L2C_LIST(name) name##_in_linked2_cached_list