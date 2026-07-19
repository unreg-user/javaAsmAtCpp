module;

#include <algorithm>
#include <cassert>
#include <span>


#include "../macro.h"
#define NAME_IN_L2C_LIST(name) name##_in_linked2_cached_list

export module cached_2linked_list;

export namespace c2l_list {
    struct Base {
        enum class OffsetSide { PREV, NEXT, NONE };
        enum class DestructMode { DESTRUCT, CLEAR, NONE };
    };

    template <typename ElementType>
    struct BList {
        template <typename, bool>
        friend struct AdderList;

        struct Node;

        struct NodeBase {
            static constexpr size_t unaddedSIdx = static_cast<size_t>(-2);
            static constexpr size_t addedSIdx = static_cast<size_t>(-1);

            Node* nextNode = nullptr;
            Node* prevNode = nullptr;
            size_t sourceIndex = unaddedSIdx;
            ElementType instance;
        };

        struct Node : protected NodeBase {
        private:
            using NodeBase::nextNode;
            using NodeBase::prevNode;
            using NodeBase::sourceIndex;
            using NodeBase::instance;
        public:
            using NodeBase::unaddedSIdx;
            using NodeBase::addedSIdx;

            friend BList;

            NODISCARD constexpr const NodeBase& getData() const noexcept {
                return *this;
            }

            NODISCARD constexpr bool isNew() const noexcept {
                return sourceIndex == unaddedSIdx;
            }

            constexpr void markAdded() noexcept {
                sourceIndex = addedSIdx;
            }

            // ReSharper disable once CppNonExplicitConvertingConstructor
            constexpr Node(ElementType&& node) noexcept : instance(std::move(node)) {
                assert(node.NAME_IN_L2C_LIST(is_valid_new)());
                node.NAME_IN_L2C_LIST(mark_unnew)();
            }

            constexpr Node() noexcept : Node(create_empty_element_instance()) {
            }

            ~Node() noexcept {
                instance.NAME_IN_L2C_LIST(destruct)();
            }

            NODISCARD constexpr bool is_empty() const noexcept {
                return instance.NAME_IN_L2C_LIST(is_empty)();
            }

            NODISCARD constexpr void mark_empty() noexcept {
                create_empty_element_instance();
            }

        private:
            NODISCARD constexpr static ElementType create_empty_element_instance() noexcept {
                return ElementType::NAME_IN_L2C_LIST(create_empty)();
            }

            NODISCARD constexpr Node* get_next_nearstnext_present_node_and_destruct_empty() noexcept {
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
                            connect(this, nextI);
                        }
                    }
                }
                return nextI;
            }
        };

        ~BList() noexcept {
            auto* node = first;
            if (node) {
                do {
                    auto* nextNode = node->nextNode;
                    delete node;
                    node = nextNode;
                } while (node);
            }
        }

        template <bool inStart = false>
        constexpr void append(ElementType&& element) noexcept {
            append<inStart>(new Node{std::move(element)});
        }

    protected:
        Node* first = nullptr;
        Node* last = nullptr;
        size_t dynamicSize = 0;

    public:
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
                if constexpr (inStart) {
                    connect(otherList.last, first);
                    first = otherList.first;
                } else {
                    connect(last, otherList.first);
                    last = otherList.last;
                }

                otherList.first = nullptr;
                dynamicSize += otherList.dynamicSize;
            }
        }

        constexpr void clearEmpty() noexcept {
            auto* node = first;
            while (node) {
                node = node->get_next_nearstnext_present_node_and_destruct_empty();
            }
        }

    protected:
        template <bool inStart = false>
        constexpr void append(const Node* node) noexcept {
            if (!first && node->is_empty()) {
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
            dynamicSize++;
        }

        template <bool before = false>
        constexpr void insert(const Node* node, const Node* sideNode) noexcept {
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

            dynamicSize++;
        }

        template <bool before = false>
        constexpr void insert(BList&& otherList, const Node* sideNode) noexcept {
            assert(sideNode);

            if constexpr (before) {
                auto& nextNode = sideNode;
                if (first == nextNode) {
                    append<before>(otherList);
                    return;
                }

                connect(nextNode->prevNode, otherList, nextNode);
            } else {
                auto& prevNode = sideNode;
                if (last == prevNode) {
                    append<before>(otherList);
                    return;
                }

                connect(prevNode, otherList, prevNode->nextNode);
            }

            otherList.first = nullptr;
            dynamicSize += otherList.dynamicSize;
        }

        static constexpr void remove(const Node* node) {
            connect(node->prevNode, node->nextNode);
            delete node;
        }

        static constexpr void markRemove(const Node* node) {
            node->is_empty()
        }

        static constexpr void connect(const Node* left, const Node* right) noexcept {
            left->nextNode = right;
            right->prevNode = left;
        }

        static constexpr void connect(const Node* left, const Node* middle, const Node* right) noexcept {
            connect(left, middle);
            connect(middle, right);
        }

        static constexpr void connect(const Node* left, const BList& middle, const Node* right) noexcept {
            connect(left, middle.last);
            connect(middle.first, right);
        }
    };

    template <typename ElementType>
    struct CList : BList<ElementType> {
    protected:
        using AdderListType = BList<ElementType>;
        using AdderListType::dynamicSize;
        using AdderListType::first;
        using AdderListType::last;
        using typename AdderListType::Node;

    public:
        using Nodes = std::span<Node*>;

    protected:
        Nodes cache = {};

    public:
        explicit constexpr CList() noexcept = default;

        template <bool sourceIndexUpdate = true>
        constexpr auto to_span() noexcept {
            auto* array = new Node*[dynamicSize];
            auto* j = array;
            auto* node = first;
            if (node) {
                for (size_t i = 0; i < dynamicSize;) {
                    *j = node;
                    if constexpr (sourceIndexUpdate) {
                        node->sourceIndex = i;
                    }
                    node = node->get_next_nearstnext_present_node_and_destruct_empty();
                    ++i;
                    ++j;
                }
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
        constexpr void insert(ElementType&& element, const size_t index) noexcept {
            assert(index < cache.size());
            insert<before>(new Node{std::move(element)}, cache[index]);
        }

    protected:
        template <bool before = false>
        constexpr void insert(const Node* node, const size_t index) noexcept {
            assert(index < cache.size());
            insert<before>(node, cache[index]);
        }
    };
} // namespace c2l_list
