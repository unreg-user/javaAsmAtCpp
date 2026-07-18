module;

#include <algorithm>
#include <cassert>
#include <span>


#include "../macro.h"
#define NAME_IN_L2C_LIST(name) name##_in_linked2_cached_list

export module cached_2linked_list;

export namespace c2l_list {
    template <typename ElementType, typename Self>
    struct Node {
        Self* nextNode = nullptr;
        Self* prevNode = nullptr;
        size_t sourceIndex = -2;
        ElementType node;

        NODISCARD constexpr bool isNew() const noexcept {
            return sourceIndex == -2;
        }

        constexpr void markAdded() noexcept {
            sourceIndex = -1;
        }

        // ReSharper disable once CppNonExplicitConvertingConstructor
        constexpr Node(ElementType&& node) : node(std::move(node)) noexcept {
            assert(node.NAME_IN_L2C_LIST(is_valid_new)());
            node.NAME_IN_L2C_LIST(mark_unnew)();
        }

        constexpr Node() : Node(ElementType::NAME_IN_L2C_LIST(create_empty)()) noexcept {
        }

        ~Node() noexcept {
            node.NAME_IN_L2C_LIST(destruct)();
        }

        NODISCARD constexpr bool is_empty() const noexcept {
            return node.NAME_IN_L2C_LIST(is_empty)();
        }

        NODISCARD constexpr Self* get_next_nearstnext_present_node_and_destruct_empty() const noexcept {
            auto* nextI = nextNode;
            if (nextI) {
                bool changed = false;
                while (nextI && nextI->is_empty()) {
                    changed = true;
                    auto nextNextI = nextI->nextNode;
                    delete nextI;
                    nextI = nextNextI;
                }
                if (changed) {
                    nextNode = nextI;
                    if (nextI) {
                        nextI->prevNode = this;
                    }
                }
            }
            return nextI;
        }
    };

    template <typename ElementType>
    struct NodeImpl : Node<ElementType, NodeImpl<ElementType>>{
    };

    struct Base {
        enum class OffsetSide { PREV, NEXT, NONE };

        enum class DestructMode { DESTRUCT, CLEAR, NONE };
    };

    template <typename ElementType>
    struct AdderList;

    template <typename ElementType, typename NodeType = NodeImpl<ElementType>, typename AdderListType = AdderList<ElementType>>
    struct List : protected Base {
        using ElementT = ElementType;
        using NodeT = NodeType;
        using AdderListT = AdderListType;

        using Nodes = std::span<NodeType*>;

    protected:
        NodeType* first = nullptr;
        NodeType* last = nullptr;
        Nodes cache = {};
        size_t dynamicSize = 0;

        constexpr void destruct_cache() noexcept {
            delete[] cache.data();
            cache = {};
        }

        constexpr void deep_destruct_cache() noexcept {
            for (auto node : cache) {
                delete node;
            }
            destruct_cache();
        }

    public:
        explicit constexpr List() noexcept = default;

        template <bool sourceIndexUpdate = true>
        constexpr auto to_span() noexcept {
            destruct_cache();
            auto* array = new Node*[dynamicSize];
            auto* j = array;
            auto* node = first;
            if (node) {
                for (size_t i = 0; i < dynamicSize;) {
                    *j = node;
                    if constexpr (sourceIndexUpdate) {
                        node.sourceIndex = i;
                    }
                    node = node->get_next_nearstnext_present_node_and_destruct_empty();
                    ++i;
                    ++j;
                }
            }
            cache = {array, dynamicSize};
            return cache;
        }

        ~List() noexcept {
            to_span<false>();
            deep_destruct_cache();
        }

        NODISCARD constexpr auto get_cache_span() const noexcept {
            return cache;
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

        template <OffsetSide side>
        NODISCARD constexpr NodeType* get_offsetnearst_cache_node(const size_t index) const noexcept {
            auto* node = cache[index];
            do {
                if constexpr (side == OffsetSide::PREV) {
                    node = node->prevNode;
                } else if constexpr (side == OffsetSide::NEXT) {
                    node = node->nextNode;
                } else if constexpr (side == OffsetSide::NONE) {
                    return node;
                }
            } while (node && node->is_empty());
            return node;
        }

        template <bool inStart = false>
        constexpr void append(ElementT&& element) noexcept {
            append<inStart>(new Node{std::move(element)});
        }

        template <bool before = false>
        constexpr void insert(ElementT&& element, const size_t index) noexcept {
            assert(index < cache.size());
            insert<before>(new Node{std::move(element)}, cache[index]);
        }

    protected:
        template <bool inStart = false>
        constexpr void append(NodeType* node) noexcept {
            if (!first && node->is_empty()) {
                first = node;
                last = node;
                cache = {node, 1};
            } else {
                if constexpr (inStart) {
                    connect(node, first);
                    first = node;
                } else {
                    connect(last, node);
                    last = node;
                }
            }
            dynamicSize++;
        }

        template <bool before = false>
        constexpr void insert(NodeType* node, const size_t index) noexcept {
            assert(index < cache.size());
            insert<before>(node, cache[index]);
        }

        template <bool before = false>
        constexpr void insert(NodeType* node, NodeType* sideNode) noexcept {
            assert(node && sideNode);

            if constexpr (before) {
                auto& nextNode = sideNode;
                if (first == nextNode) {
                    first = node;
                } else {
                    connect(nextNode->prevNode, node)
                }
                connect(node, nextNode);
            } else {
                auto& prevNode = sideNode;
                if (last == prevNode) {
                    last = node;
                } else {
                    connect(node, prevNode->nextNode)
                }
                connect(prevNode, node);
            }

            dynamicSize++;
        }

        static constexpr void connect(NodeType* left, NodeType* right) noexcept {
            left->nextNode = right;
            right->prevNode = left;
        }
    };

    template <typename Element>
    struct AdderList : List<Element> {
        explicit constexpr Cached2LinkedList() noexcept = default;

    }
} // namespace c2l_list
