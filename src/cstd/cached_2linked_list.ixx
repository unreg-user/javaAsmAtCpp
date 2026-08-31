module;

#include <cassert>
#include <iostream>
#include <span>
#include "macro.h"

export module cached_2linked_list;

import simple_cstd;

export namespace c2l_list {
    template <typename ElementType>
    concept CanBeElement = requires(ElementType element) {
        { ElementType::create_empty_in_linked2_cached_list() } -> std::convertible_to<ElementType>;
        { element.destruct_in_linked2_cached_list() } -> std::same_as<void>;
        { element.is_empty_in_linked2_cached_list() } -> std::convertible_to<bool>;
#ifndef NDEBUG
        { element.is_valid_new_in_linked2_cached_list() } -> std::convertible_to<bool>;
        { element.mark_unnew_in_linked2_cached_list() } -> std::same_as<void>;
#endif
    };

    struct Base {
        enum class OffsetSide { PREV, NEXT, NONE };
        enum class RemoveMode { AUTO, DESTRUCT, EMPTY };
    };

    template <typename ElementType>
    struct CList;

    template <typename ElementType>
    struct BList {
        template <typename>
        friend struct BList;

        struct Node;

        struct NodeBase {
            static constexpr size_t unaddedSIdx = static_cast<size_t>(-2);
            static constexpr size_t addedSIdx = static_cast<size_t>(-1);

            ElementType instance;
            Node* nextNode = nullptr;
            Node* prevNode = nullptr;
            size_t sourceIndex = unaddedSIdx;

            friend std::ostream& operator<<(std::ostream& os, const Node& self) {
                os << "Node{" << self.instance << ", " << self.sourceIndex << "}";
                return os;
            }

        protected:
            // ReSharper disable once CppNonExplicitConvertingConstructor
            constexpr NodeBase(ElementType&& node) noexcept : instance(std::move(node)) {
            }
        };

        struct Node : protected NodeBase {
        private:
            using NodeBase::instance;
            using NodeBase::nextNode;
            using NodeBase::prevNode;
            using NodeBase::sourceIndex;

        public:
            using NodeBase::addedSIdx;
            using NodeBase::unaddedSIdx;

            friend BList;
            friend CList<ElementType>;

            NODISCARD constexpr const NodeBase& get_data() const noexcept {
                return *this;
            }

            NODISCARD constexpr bool is_new() const noexcept {
                return sourceIndex == unaddedSIdx;
            }

            constexpr Node() noexcept : Node(create_empty_element_instance()) {
            }

            // ReSharper disable once CppNonExplicitConvertingConstructor
            constexpr Node(ElementType&& node) noexcept : NodeBase(std::move(node)) {
#ifndef NDEBUG
                assert(node.is_valid_new_in_linked2_cached_list());
                node.mark_unnew_in_linked2_cached_list();
#endif
            }

            ~Node() noexcept {
                instance.destruct_in_linked2_cached_list();
            }

            NODISCARD constexpr bool is_empty() const noexcept {
                return instance.is_empty_in_linked2_cached_list();
            }

        private:
            constexpr void mark_added() noexcept {
                sourceIndex = addedSIdx;
            }

            constexpr void set_to_empty() noexcept {
                instance.destruct_in_linked2_cached_list();
                instance = create_empty_element_instance();
            }

            NODISCARD constexpr static ElementType create_empty_element_instance() noexcept {
                return ElementType::create_empty_in_linked2_cached_list();
            }

            template <bool nextIncludesSelf = false>
            NODISCARD constexpr Node* clear_next_empty_and_get_next_p() noexcept {
                auto* nextI = nextIncludesSelf ? this : nextNode;
                if (nextI) {
                    size_t cleared = 0;
                    while (nextI && nextI->is_empty()) {
                        cleared++;
                        auto nextNextI = nextI->nextNode;
                        delete nextI;
                        nextI = nextNextI;
                    }
                    if (cleared > 0) {
                        if constexpr (nextIncludesSelf) {
                            nextNode = nextI;
                            if (nextI) {
                                connect(this, nextI);
                            }
                        } else {
                            nextI->prevNode = nullptr;
                        }
                    }
                }
                return nextI;
            }
        };

    protected:
        Node* first = nullptr;
        Node* last = nullptr;
        size_t dynamicSize = 0;

    public:
        BList() noexcept = default;

        DEL_CPY(BList);

        ~BList() noexcept {
            foreach_link_internal([](const Node* node) noexcept { delete node; });
        }

        template <bool inStart = false>
        constexpr void append(ElementType&& element) noexcept {
            append<inStart>(new Node{std::move(element)});
        }

        NODISCARD constexpr auto get_dynamic_size() const noexcept {
            return dynamicSize;
        }

        NODISCARD constexpr auto get_first() const noexcept {
            return first;
        }

        NODISCARD constexpr auto get_last() const noexcept {
            return last;
        }

        template <bool inStart = false>
        constexpr void append(BList&& otherList) noexcept {
            if (otherList.first) {
                if (first) {
                    if constexpr (inStart) {
                        connect(otherList.last, first);
                        first = otherList.first;
                    } else {
                        connect(last, otherList.first);
                        last = otherList.last;
                    }
                } else {
                    first = otherList.first;
                    last = otherList.last;
                }

                otherList.first = nullptr;
                dynamicSize += otherList.dynamicSize;
            }
        }

        constexpr void clear_empty() noexcept {
            if (first)  {
                first = first->template clear_next_empty_and_get_next_p<true>();
                Node* node = first;
                Node* preNode = nullptr;

                while (node) {
                    preNode = node;
                    node = node->clear_next_empty_and_get_next_p();
                }
                last = preNode;
            }
        }

        template <bool inStart = false, bool ignoreIfElse = true>
        constexpr void pop() noexcept {
            pop_internal<inStart, ignoreIfElse, Base::RemoveMode::AUTO>();
        }

        friend std::ostream& operator<<(std::ostream& os, const BList& self) {
            os << "BList[";
            self.foreach_link_internal([&](const Node* node) { os << *node << ", "; });
            os << "]";
            return os;
        }

        constexpr void foreach_link(this auto&& self, auto&& callable) noexcept {
            self.foreachLinkInternal([&](Node* node) { callable(node->get_data()); });
        }

    protected:
        constexpr void foreach_link_internal(this auto&& self, auto&& callable) noexcept {
            auto* node = self.first;
            if (node) {
                do {
                    auto* nextNode = node->nextNode;
                    callable(node);
                    node = nextNode;
                } while (node);
            }
        }

        template <bool inStart = false>
        constexpr void append(Node* const node) noexcept {
            assert(node);

            if (!first) {
                first = node;
                last = node;
            } else {
                if constexpr (inStart) {
                    connect(node, first);
                    first = node;
                } else {
                    connect(last, node);
                    last = node;
                }
            }
            node->mark_added();
            ++dynamicSize;
        }

        template <bool before = false>
        constexpr void insert(Node* const node, Node* const sideNode) noexcept {
            assert(node && sideNode);

            if constexpr (before) {
                auto& nextNode = sideNode;
                if (first == nextNode) {
                    append<before>(node);
                    return;
                }

                connect(nextNode->prevNode, node, nextNode);
            } else {
                auto& prevNode = sideNode;
                if (last == prevNode) {
                    append<before>(node);
                    return;
                }

                connect(prevNode, node, prevNode->nextNode);
            }

            node->mark_added();
            ++dynamicSize;
        }

        template <bool before = false>
        constexpr void insert(BList&& otherList, Node* const sideNode) noexcept {
            assert(sideNode);

            if constexpr (before) {
                auto& nextNode = sideNode;
                if (first == nextNode) {
                    append<before>(std::move(otherList));
                    return;
                }

                connect(nextNode->prevNode, otherList, nextNode);
            } else {
                auto& prevNode = sideNode;
                if (last == prevNode) {
                    append<before>(std::move(otherList));
                    return;
                }

                connect(prevNode, otherList, prevNode->nextNode);
            }

            otherList.first = nullptr;
            dynamicSize += otherList.dynamicSize;
        }

        template <bool inStart = false, bool ignoreIfElse = true, Base::RemoveMode mode = Base::RemoveMode::AUTO>
        constexpr void pop_internal() noexcept {
            if constexpr (!ignoreIfElse)
                assert(first);

            if (first) {
                Node* deleted;
                if constexpr (inStart) {
                    deleted = first;
                    first = first->nextNode;
                    if (first) {
                        first->prevNode = nullptr;
                    } else {
                        last = nullptr;
                    }
                } else {
                    deleted = last;
                    last = last->prevNode;
                    if (last) {
                        last->nextNode = nullptr;
                    } else {
                        first = nullptr;
                    }
                }
                remove_unchecked_internal<mode>(deleted);

                --dynamicSize;
            }
        }

        constexpr void remove(Node* const node) noexcept {
            assert(node);

            if (node == first) {
                pop<true, false>();
            } else if (node == last) {
                pop<false, false>();
            } else {
                connect(node->prevNode, node->nextNode);
                remove_unchecked_internal(node);
                --dynamicSize;
            }
        }

        template <Base::RemoveMode mode = Base::RemoveMode::AUTO>
        static constexpr void remove_unchecked_internal(Node* const node) noexcept {
            if constexpr (mode == Base::RemoveMode::AUTO) {
                if (node->sourceIndex == Node::addedSIdx) {
                    delete node;
                } else {
                    node->set_to_empty();
                }
            } else if constexpr (mode == Base::RemoveMode::DESTRUCT) {
                delete node;
            } else if constexpr (mode == Base::RemoveMode::EMPTY) {
                node->set_to_empty();
            }
        }

        static constexpr void connect(Node* const left, Node* const right) noexcept {
            left->nextNode = right;
            right->prevNode = left;
        }

        static constexpr void connect(Node* const left, Node* const middle, Node* const right) noexcept {
            connect(left, middle);
            connect(middle, right);
        }

        static constexpr void connect(Node* const left, const BList& middle, Node* const right) noexcept {
            connect(left, middle.first);
            connect(middle.last, right);
        }
    };

    template <typename ElementType>
    struct CList : BList<ElementType> {
        using BListType = BList<ElementType>;
        using typename BListType::Node;
        using Nodes = std::span<Node*>;

    protected:
        using BListType::dynamicSize;
        using BListType::first;
        using BListType::last;

        Nodes cache = {};

    public:
        explicit constexpr CList() noexcept = default;
        explicit constexpr CList(BListType&& bList) : BListType{std::move(bList)} {
        }

        ~CList() noexcept {
            delete[] cache.data();
        }

        template <bool sourceIndexUpdate = true>
        constexpr auto to_span() noexcept {
            delete[] cache.data();
            auto* array = new Node*[dynamicSize];
            auto* j = array;
            first = first->template clear_next_empty_and_get_next_p<true>();
            auto* node = first;
            if (node) {
                for (size_t i = 0; i < dynamicSize; ++i, ++j) {
                    *j = node;
                    if constexpr (sourceIndexUpdate) {
                        node->sourceIndex = i;
                    }
                    node = node->clear_next_empty_and_get_next_p();
                }
                last = *(--j);
            }
            cache = {array, dynamicSize};
            return cache;
        }

        NODISCARD constexpr auto get_cache_span() const noexcept {
            return cache;
        }

        template <Base::OffsetSide side>
        NODISCARD constexpr Node* get_offsetnearst_cache_node(const size_t index) const noexcept {
            auto* node = cache[index];
            do {
                if constexpr (side == Base::OffsetSide::PREV) {
                    node = node->prevNode;
                } else if constexpr (side == Base::OffsetSide::NEXT) {
                    node = node->nextNode;
                } else if constexpr (side == Base::OffsetSide::NONE) {
                    return node;
                }
            } while (node && node->is_empty());
            return node;
        }

        template <bool before = false>
        constexpr void insert(const size_t index, ElementType&& element) noexcept {
            assert(index < cache.size());
            insert<before>(cache[index], new Node{std::move(element)});
        }

        template <bool before = false>
        constexpr void insert(const size_t index, BListType&& list) noexcept {
            assert(index < cache.size());
            BListType::template insert<before>(std::move(list), cache[index]);
        }

        constexpr void remove(const size_t index) noexcept {
            auto node = cache[index];

            if (node == first) {
                BListType::template pop_internal<true, false, Base::RemoveMode::EMPTY>();
            } else if (node == last) {
                BListType::template pop_internal<false, false, Base::RemoveMode::EMPTY>();
            } else {
                BListType::connect(node->prevNode, node->nextNode);
                BListType::template remove_unchecked_internal<Base::RemoveMode::EMPTY>(node);
                --dynamicSize;
            }
        }

    protected:
        template <bool before = false>
        constexpr void insert(const size_t index, Node* const node) noexcept {
            assert(index < cache.size());
            BListType::template insert<before>(node, cache[index]);
        }
    };
} // namespace c2l_list
