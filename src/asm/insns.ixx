module;

#include <iostream>
#include "../cstd/macro.h"
#include "insns_mcr.h"

export module insns;

import cstd_variant;
import cached_2linked_list;

namespace insn {
    export {
        struct AbsInsnNode;

        struct AbstractInsnNode {
            friend AbsInsnNode;

            constexpr std::ostream& operatorKK(this auto&& self, std::ostream& os) noexcept {
                return os << self;
            }

        private:
#ifndef NDEBUG
            bool isNew = true;
#endif
        };

        struct TestNode : AbstractInsnNode {
            size_t uniqueId;

            TestNode() noexcept : AbstractInsnNode() {
                static size_t q = 0;
                uniqueId = ++q;
            }

            constexpr friend std::ostream& operator<<(std::ostream& os, const TestNode& self) noexcept {
                os << "TestNode[" << self.uniqueId << "]";
                return os;
            }
        };
    }

    using AbsInsnNodePType = cstd::variant_of_nullable_ptrs<TestNode>;

    export {
        struct AbsInsnNode : AbsInsnNodePType {
            template <typename>
            friend struct c2l_list::BList;

        protected:
            using AbsInsnNodePType::AbsInsnNodePType;

        private:
#ifndef NDEBUG
            constexpr void NAME_IN_L2C_LIST(mark_unnew)() const noexcept {
                this->SET_ASBT_INSN_FIELD(isNew, false);
            }

            NODISCARD constexpr bool NAME_IN_L2C_LIST(is_valid_new)(this auto&& self) noexcept {
                return self.is_not_null() && self.GET_ASBT_INSN_FM(isNew);
            }
#endif

            constexpr void NAME_IN_L2C_LIST(destruct)() const noexcept {
                this->destruct_deref();
            }

            NODISCARD constexpr bool NAME_IN_L2C_LIST(is_empty)() const noexcept {
                return this->is_null();
            }

            NODISCARD static constexpr AbsInsnNode NAME_IN_L2C_LIST(create_empty)() noexcept {
                return {nullptr};
            }
        };

        using CList = c2l_list::CList<AbsInsnNode>;
        using BList = c2l_list::BList<AbsInsnNode>;
    }
}