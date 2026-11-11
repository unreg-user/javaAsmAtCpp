module;

#include <iostream>
#include "../../../cstd/macro.h"
#include "../../impl_base/insns_mcr.h"

export module insns;

import cstd_variant;
import cached_2linked_list;
import simple_cstd;

namespace insn {
    export {
        struct AnyInsnNode;

        struct AbstractInsnNode {
            friend AnyInsnNode;

            constexpr std::ostream& operatorKK(this auto&& self, std::ostream& os) noexcept {
                return os << self;
            }

        private:
#ifndef NDEBUG
            bool isNew = true;
#endif
        };

        struct EmptyNode {
        };

        struct TestNode : AbstractInsnNode {
            size_t uniqueId;

            constexpr TestNode() noexcept : AbstractInsnNode() {
                static size_t q = 0;
                uniqueId = ++q;
            }

            constexpr friend std::ostream& operator<<(std::ostream& os, const TestNode& self) noexcept {
                os << "TestNode[" << self.uniqueId << "]";
                return os;
            }
        };
    }

    using AnyInsnNodeBase = cstd::variant_of_ptrs_and_little<cstd::ptr_size, cstd::EmptyValueMode::HAS, TestNode>;

    export {
        struct AnyInsnNode : AnyInsnNodeBase {
            using AnyInsnNodeBase::AnyInsnNodeBase;

            template <typename>
            friend struct c2l_list::BList;

        private:
#ifndef NDEBUG
            constexpr void mark_unnew_in_linked2_cached_list() noexcept {
                this->deref_visit(cstd::overload{[](const EmptyNode) noexcept {}, [](AbstractInsnNode& _node) noexcept { _node.isNew = false; }});
            }

            NODISCARD constexpr bool is_valid_new_in_linked2_cached_list() const noexcept {
                return this->deref_visit(cstd::overload{[](const EmptyNode) noexcept { return true; }, [](const AbstractInsnNode& _node) noexcept { return _node.isNew; }});
            }
#endif

            constexpr void destruct_in_linked2_cached_list() noexcept {
                this->destruct_deref();
            }

            NODISCARD constexpr bool is_empty_in_linked2_cached_list() const noexcept {
                return this->is_holds_empty();
            }

            NODISCARD static constexpr AnyInsnNode create_empty_in_linked2_cached_list() noexcept {
                return {};
            }
        };

        using CList = c2l_list::CList<AnyInsnNode>;
        using BList = c2l_list::BList<AnyInsnNode>;
    }
}