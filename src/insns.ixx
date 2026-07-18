module;

#include <cassert>
#include <span>

#include <variant>
#include <vector>
#include "macro.h"

#define GET_ASBT_INSN_FM(fmName) deref_visit([](const AbstractInsnNode& _node) noexcept { return _node.fmName; })
#define SET_ASBT_INSN_FIELD(fieldName, ...)                                                                            \
    deref_visit([](AbstractInsnNode& _node) noexcept { _node.fieldName = __VA_ARGS__; })
#define NAME_IN_L2C_LIST(name) name##_in_linked2_cached_list

export module insns;

import cstd_variant;
import cached_2linked_list;

export {
    struct AbsInsnNode;
    struct AbstractInsnNode {
        friend AbsInsnNode;
    private:
#if DEBUG
        bool isNew = true;
#endif
    };

    template <typename NodeType>
    struct Cached2LinkedList;

    using InsnList = Cached2LinkedList<AbsInsnNode>;

    struct AbsInsnNode : cstd::variant_of_nullable_ptrs<> {
        friend InsnList;

    private:
#if DEBUG
        NODISCARD constexpr bool NAME_IN_L2C_LIST(mark_unnew)() noexcept {
            return this->SET_ASBT_INSN_FIELD(isNew, false);
        }

        NODISCARD constexpr bool NAME_IN_L2C_LIST(is_valid_new)(this auto&& self) noexcept {
            return self.is_not_null() && this->GET_ASBT_INSN_FM(isNew);
        }
#endif

        NODISCARD constexpr void NAME_IN_L2C_LIST(destruct)() noexcept {
            destruct_deref();
        }

        NODISCARD constexpr bool NAME_IN_L2C_LIST(is_empty)() const noexcept {
            return is_null();
        }

        NODISCARD static constexpr AbsInsnNode NAME_IN_L2C_LIST(create_empty)() noexcept {
            return {nullptr};
        }
    };

    template struct Cached2LinkedList<AbsInsnNode>;
}
