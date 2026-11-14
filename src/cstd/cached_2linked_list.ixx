module;

#include <cassert>
#include <optional>
#include <span>
#include "macro.h"

export module cached_2linked_list;

import simple_cstd;
import costd;

export namespace c2l_list {
    template <typename ElemT>
    concept LinkedC2ListElement = requires(ElemT element) {
        { ElemT::create_empty_in_linked2_cached_list() } -> std::same_as<ElemT>;
        { element.destruct_in_linked2_cached_list() } -> std::same_as<void>;
        { element.is_empty_in_linked2_cached_list() } -> std::same_as<bool>;
#ifndef NDEBUG
        { element.is_valid_new_in_linked2_cached_list() } -> std::same_as<bool>;
        { element.mark_unnew_in_linked2_cached_list() } -> std::same_as<void>;
#endif
    };

    template <typename LabelT>
    concept LinkedC2ListLabel = requires(LabelT label, size_t idx) {
        LabelT{};
        { label.is_new() } -> std::same_as<bool>;
        { label.mark_added() } -> std::same_as<void>;
        { label.set_idx(idx) } -> std::same_as<void>;
    };

    struct Base {
        enum class OffsetSide { PREV, NEXT, NONE };
        enum class RemoveMode { AUTO, DESTRUCT, EMPTY };
    };

    template <typename, typename, typename>
    struct AbsCList;

    struct UnindexedLabel {
        bool isAdded = false;

        constexpr UnindexedLabel() noexcept = default;

        NODISCARD constexpr bool is_new() const noexcept {
            return isAdded;
        }

        constexpr void mark_added() noexcept {
            isAdded = true;
        }

        static constexpr void set_idx(size_t) noexcept {
        }
    };

    struct IndexedLabel {
        static constexpr size_t unaddedSIdx = static_cast<size_t>(-2);
        static constexpr size_t addedSIdx = static_cast<size_t>(-1);

        constexpr IndexedLabel() noexcept = default;

        size_t idx = unaddedSIdx;

        NODISCARD constexpr bool is_new() const noexcept {
            return idx == unaddedSIdx;
        }

        constexpr void mark_added() noexcept {
            idx = addedSIdx;
        }

        constexpr void set_idx(const size_t i) noexcept {
            idx = i;
        }
    };

    template <typename ElementType, typename Label = UnindexedLabel>
    struct BList {
        static_assert(LinkedC2ListLabel<Label> && LinkedC2ListElement<ElementType>);

        template <typename, typename>
        friend struct BList;

        struct Node;

        struct BaseNode {
            Node* nextNode = nullptr;
            Node* prevNode = nullptr;
            ElementType instance;
            Label label{};

            friend std::ostream& operator<<(std::ostream& os, const Node& self) {
                os << "Node{" << self.instance;
                if constexpr (cstd::is_console_writable<const Label&>) {
                    os << ", " << self.label;
                }
                os << "}";
                return os;
            }

        protected:
            // ReSharper disable once CppNonExplicitConvertingConstructor
            constexpr BaseNode(ElementType&& node) noexcept : instance(std::move(node)) {
            }
        };

        struct Node : protected BaseNode {
        private:
            using BaseNode::instance;
            using BaseNode::nextNode;
            using BaseNode::prevNode;

        public:
            using BaseNode::label;

            friend BList;

            template <typename, typename, typename>
            friend struct AbsCList;

            /*implicit*/ constexpr operator const BaseNode&() const& noexcept {
                return *this;
            }

            /*implicit*/ constexpr operator const BaseNode&&() const&& noexcept {
                return std::move(*this);
            }

            /*implicit*/ constexpr operator const BaseNode&() & noexcept {
                return *this;
            }

            /*implicit*/ constexpr operator const BaseNode&&() && noexcept {
                return std::move(*this);
            }

            NODISCARD constexpr bool is_new() const noexcept {
                return label.is_new();
            }

            constexpr Node() noexcept : Node(create_empty_element_instance()) {
            }

            // ReSharper disable once CppNonExplicitConvertingConstructor
            constexpr Node(ElementType&& node) noexcept : BaseNode(std::move(node)) {
#ifndef NDEBUG
                assert(instance.is_valid_new_in_linked2_cached_list());
                instance.mark_unnew_in_linked2_cached_list();
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
                return label.mark_added();
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

                if constexpr (!nextIncludesSelf) {
                    if (!nextI)
                        return nullptr;
                }

                size_t cleared = 0;
                while (nextI && nextI->is_empty()) {
                    cleared++;
                    auto nextNextI = nextI->nextNode;
                    delete nextI;
                    nextI = nextNextI;
                }
                if (cleared > 0) {
                    if constexpr (nextIncludesSelf) {
                        if (nextI) {
                            nextI->prevNode = nullptr;
                        }
                    } else {
                        if (nextI) {
                            connect(this, nextI);
                        } else {
                            this->nextNode = nullptr;
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

        constexpr void destruct_nodes() noexcept {
            foreach_link_internal([](const Node* node) noexcept { delete node; });
        }

        constexpr void clear_fls() noexcept {
            first = nullptr;
            last = nullptr;
            dynamicSize = 0;
        }

    public:
        constexpr BList() noexcept = default;

        DEL_CPY(BList);

        constexpr ~BList() noexcept {
            destruct_nodes();
        }

        constexpr BList(BList&& other) noexcept : first(other.first), last(other.last), dynamicSize(other.dynamicSize) {
            other.clear_fls();
        }

    protected:
        constexpr BList(Node* first, Node* last, const size_t dynamicSize) noexcept :
            first(first), last(last), dynamicSize(dynamicSize) {
        }

    public:
        BList& operator=(BList&& other) noexcept {
            if (this != &other) {
                destruct_nodes();

                first = other.first;
                last = other.last;
                dynamicSize = other.dynamicSize;

                other.clear_fls();
            }
            return *this;
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

                dynamicSize += otherList.dynamicSize;
                otherList.clear_fls();
            }
        }

        /** updates cache */
        template <typename Self>
        constexpr void clear_empty(this Self&& self) noexcept {
            if constexpr (requires {
                              { self.to_array() } -> std::same_as<void>;
                          }) {
                self.to_array();
            } else if (self.first) {
                self.first = self.first->template clear_next_empty_and_get_next_p<true>();
                Node* node = self.first;
                Node* preNode = nullptr;

                while (node) {
                    preNode = node;
                    node = node->clear_next_empty_and_get_next_p();
                }
                self.last = preNode;
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
            self.foreach_link_internal([&](const Node* node) { callable(node->instance); });
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

            if (otherList.first) {
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

                dynamicSize += otherList.dynamicSize;
                otherList.clear_fls();
            }
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
                if (!node->is_new()) {
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

    template <typename Nodes, typename ElementType, typename Label = UnindexedLabel>
    struct AbsCList : BList<ElementType, Label> {
        using BListT = BList<ElementType, Label>;
        using typename BListT::BaseNode;
        using typename BListT::Node;

    protected:
        using BListT::dynamicSize;
        using BListT::first;
        using BListT::last;
        Nodes cache{};

        constexpr AbsCList(Node* first, Node* last, const size_t dynamicSize, Nodes&& cache) noexcept :
            BListT{first, last, dynamicSize}, cache(std::move(cache)) {
        }

    public:
        constexpr AbsCList() noexcept = default;

        explicit constexpr AbsCList(BListT&& bList) noexcept : BListT{std::move(bList)} {
        }

        template <bool sourceIdxUpdate = true>
        constexpr auto to_array(this auto&& self) noexcept {
            self.clear_cache();
            auto array = self.alloc_new_nodes_array();
            if (first) {
                first = first->template clear_next_empty_and_get_next_p<true>();
                auto* node = first;
                if (node) {
                    for (size_t i = 0; i < dynamicSize; ++i) {
                        array[i] = node;
                        if constexpr (sourceIdxUpdate) {
                            node->sourceIdx = i;
                        }
                        node = node->clear_next_empty_and_get_next_p();
                    }
                    last = array[dynamicSize - 1];
                }
            }
            return self.return_cache_by_array(std::move(array));
        }

        NODISCARD constexpr decltype(auto) get_cache(this auto&& self) noexcept {
            return std::forward_like<decltype(self)>(self.cache);
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
            BListT::template insert<before>(new Node{std::move(element)}, cache[index]);
        }

        template <bool before = false>
        constexpr void insert(BListT&& list, const size_t index) noexcept {
            assert(index < cache.size());
            BListT::template insert<before>(std::move(list), cache[index]);
        }

        constexpr void remove(const size_t index) noexcept {
            auto node = cache[index];

            if (node == first) {
                BListT::template pop_internal<true, false, Base::RemoveMode::EMPTY>();
            } else if (node == last) {
                BListT::template pop_internal<false, false, Base::RemoveMode::EMPTY>();
            } else {
                BListT::connect(node->prevNode, node->nextNode);
                BListT::template remove_unchecked_internal<Base::RemoveMode::EMPTY>(node);
                --dynamicSize;
            }
        }

        constexpr AbsCList(AbsCList&& other) noexcept : BListT(std::move(other)), cache(other.cache) {
            other.cache = {};
        }

        constexpr AbsCList& operator=(this auto&& self, AbsCList&& other) noexcept {
            if (&self != &other) {
                self.clear_cache();
                BListT::operator=(std::move(other));
                self.cache = std::move(other.cache);
            }
            return self; // NOLINT(*-unconventional-assign-operator)
        }

    protected:
        template <bool before = false>
        constexpr void insert(Node* const node, const size_t index) noexcept {
            assert(index < cache.size());
            BListT::template insert<before>(node, cache[index]);
        }
    };

    template <typename, typename>
    struct CList;

    template <typename ElementType, typename Label = UnindexedLabel>
    struct CListB : AbsCList<cstd::impl_dest_u_vec<typename BList<ElementType, Label>::Node*>, ElementType, Label> {
        using VecElT = BList<ElementType, Label>::Node*;

    protected:
        using RawVecT = std::vector<VecElT>;
        using VecT = cstd::impl_dest_u_vec<VecElT>;

    private:
        using Base = AbsCList<VecT, ElementType, Label>;

    protected:
        using Base::BListT;
        using Base::cache;
        using Base::dynamicSize;
        using Base::first;
        using Base::last;
        size_t vecDynamicSize = 0;

    public:
        using Base::Base;
        using typename Base::BaseNode;
        using typename Base::Node;
        using Base::operator=;

        constexpr CListB() noexcept = default;

        constexpr ~CListB() noexcept {
            std::destroy_at(&cache.v);
        }

        constexpr explicit operator CList<ElementType, Label>() && {
            auto result =
                    CList<ElementType, Label>{first, last, dynamicSize, std::span<VecElT>{cache.begin(), dynamicSize}};
            cache = {};
            this->clear_fls();
            return result;
        }

        constexpr void resize(size_t size) noexcept {
            cache.v.resize(size);
            first = getNodeAt(0);
            last = getNodeAt(size - 1);
        }

        constexpr void push_back(ElementType&& element) noexcept {
            auto vecRealSize = cache.v.size();
            assert(vecDynamicSize < vecRealSize);

            auto& currentNodeRef = cache.v[vecDynamicSize];
            currentNodeRef = new Node{std::move(element)};
            if (vecDynamicSize > 0) {
                this->connect(cache.v[vecDynamicSize - 1], currentNodeRef);
            } else {
                first = currentNodeRef;
            }

            ++vecDynamicSize;
            ++dynamicSize;

            if (vecDynamicSize == vecRealSize) {
                last = currentNodeRef;
            }

            currentNodeRef->mark_added();
        }

        constexpr auto getNodeAt(size_t i) noexcept {
            return cache.v[i];
        }

    protected:
        constexpr void clear_cache() noexcept {
            cache.v.clear();
        }

        constexpr auto alloc_new_nodes_array() noexcept {
            vecDynamicSize = dynamicSize;
            RawVecT vec{};
            vec.resize(dynamicSize);
            return vec;
        }

        constexpr auto return_cache_by_array(RawVecT&& array) noexcept {
            cache = VecT{std::move(array)};
            return cache;
        }
    };

    template <typename ElementType, typename Label = UnindexedLabel>
    struct CList : AbsCList<std::span<typename BList<ElementType, Label>::Node*>, ElementType, Label> {
        template <typename, typename>
        friend struct CListB;

    private:
        using SpanElT = BList<ElementType, Label>::Node*;
        using SpanT = std::span<SpanElT>;
        using Base = AbsCList<SpanT, ElementType, Label>;

    protected:
        using Base::BListT;
        using Base::cache;
        using Base::dynamicSize;
        using Base::first;
        using Base::last;

    public:
        using Base::Base;
        using typename Base::BaseNode;
        using typename Base::Node;
        using Base::operator=;

        constexpr CList() noexcept = default;

    protected:
        constexpr void clear_cache() noexcept {
            delete[] cache.data();
        }

    public:
        constexpr ~CList() noexcept {
            clear_cache();
        }

    protected:
        constexpr Node** alloc_new_nodes_array() noexcept {
            return new Node*[dynamicSize];
        }

        constexpr auto return_cache_by_array(Node** array) noexcept {
            cache = SpanT{array, dynamicSize};
            return cache;
        }
    };
} // namespace c2l_list
