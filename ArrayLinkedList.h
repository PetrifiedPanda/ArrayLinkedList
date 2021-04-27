#pragma once

#include <cstddef>

template <typename T>
class ArrayLinkedList {
    class Node {
       public:
        T* keys;

        Node* next;
        Node* prev;

        Node(size_t alloc_size, Node* prev = nullptr) :
            keys(new T[alloc_size]), 
            next(nullptr), 
            prev(prev) {}

        ~Node() {
            delete[] keys;
        }
    };

    static const size_t s_default_node_size_ = 50;

    Node* head_;
    Node* tail_;

    size_t node_size_;
    size_t node_count_;
    size_t tail_size_;

    // Utility for copying, moving and freeing (Used in Constructors, and copy / move assignment operators)

    static void free_following_nodes(Node* start) {
        Node* it = start;
        while (it != nullptr) {
            Node* tmp = it;
            it = it->next;
            delete tmp;
        }
    }

    void _free() {
        free_following_nodes(head_);
    }

    static void copy_arr(T* to, T* from, size_t size) {
        for (size_t i = 0; i < size; ++i)
            to[i] = from[i];
    }

    void append_following_nodes(Node* copy_begin) {
        if (head_ == nullptr)
            head_ = tail_ = new Node(node_size_);
        else {
            tail_->next = new Node(node_size_, tail_);
            tail_ = tail_->next;
        }

        for (Node* it = copy_begin; it != nullptr; it = it->next) {
            if (it->next != nullptr) {
                copy_arr(tail_->keys, it->keys, node_size_);
                tail_->next = new Node(node_size_, tail_);
                tail_ = tail_->next;
            } else {
                copy_arr(tail_->keys, it->keys, tail_size_);
            }
        }
    }

    /*
    If 2 lists have the same node size, we only need to copy the contents of the nodes of the other list into this list 
    and append the other nodes, or delete the nodes that are too much
    */
    void _copy_same_node_size(const ArrayLinkedList<T>& other) {
        node_count_ = other.node_count_;
        tail_size_ = other.tail_size_;
        Node* it = head_;
        Node* other_it = other.head_;

        while (it != nullptr && other_it != nullptr) {
            size_t copy_size = other_it->next == nullptr ? tail_size_ : node_size_;
            copy_arr(it->keys, other_it->keys, copy_size);

            it = it->next;
            other_it = other_it->next;
        }

        if (other_it == nullptr && it != nullptr) {
            tail_ = it->prev;
            tail_->next = nullptr;
            free_following_nodes(it);
        } else if (other_it != nullptr && it == nullptr) {
            append_following_nodes(other_it);
        }
    }

    void _copy(const ArrayLinkedList<T>& other) {
        node_size_ = other.node_size_;
        node_count_ = other.node_count_;
        tail_size_ = other.tail_size_;

        // This is so head does not stay uninitialised in the function call
        head_ = nullptr;
        append_following_nodes(other.head_);
    }

    void _move(ArrayLinkedList<T>&& other) {
        node_size_ = other.node_size_;
        node_count_ = other.node_count_;
        tail_size_ = other.tail_size_;
        head_ = other.head_;
        tail_ = other.tail_;

        other.head_ = nullptr;
        other.tail_ = nullptr;
        other.node_count_ = 0;
        other.tail_size_ = 0;
    }

    // Constructors and Assignment operators

   public:
    ArrayLinkedList(size_t node_size = s_default_node_size_) :
        head_(nullptr),
        tail_(nullptr),
        node_size_(node_size),
        node_count_(0),
        tail_size_(0) {}

    ArrayLinkedList(const ArrayLinkedList<T>& other) {
        _copy(other);
    }

    ArrayLinkedList(ArrayLinkedList<T>&& other) {
        _move(std::move(other));
    }

    ~ArrayLinkedList() {
        _free();
    }

    ArrayLinkedList<T>& operator=(const ArrayLinkedList<T>& other) {
        if (node_size_ == other.node_size_)
            _copy_same_node_size(other);
        else {
            _free();
            _copy(other);
        }
        return *this;
    }

    ArrayLinkedList<T>& operator=(ArrayLinkedList<T>&& other) {
        _free();
        _move(std::move(other));
        return *this;
    }

    // Getters

    size_t node_size() const {
        return node_size_;
    }

    size_t size() const {
        if (node_count_ == 0)
            return 0;
        else
            return (node_count_ - 1) * node_size_ + tail_size_;
    }

    bool empty() const {
        return size() == 0;
    }

    T& front() {
        return head_->keys[0];
    }

    const T& front() const {
        return head_->keys[0];
    }

    T& back() {
        return tail_->keys[tail_size_ - 1];
    }

    const T& back() const {
        return tail_->keys[tail_size_ - 1];
    }

    void clear() {
        _free();
        head_ = tail_ = nullptr;
    }

   private:
    T& get_item_at_index(size_t index) const {
        if (index < size()) {
            size_t node_number = index / node_size_;

            Node* it = head_;
            for (size_t i = 0; i < node_number; ++i)
                it = it->next;
            
            return it->keys[index - node_number * node_size_];
        } else {
            throw std::runtime_error("Index out of bounds");
        }
    }

   public:
    T& at(size_t index) {
        return get_item_at_index(index);
    }

    const T& at(size_t index) const {
        return get_item_at_index(index);
    }

    void resize(size_t new_size, const T& fill_item = T()) {
	    if (new_size < size()) {
            while (size()  > new_size) {
                if (size() - tail_size_ >= new_size) {
                    remove_last_node();
                } else {
                    size_t size_difference = size() - new_size;
                    tail_size_ -= size_difference;
                }
            }
        } else {
            while (size() < new_size) {
                if (tail_size_ == node_size_) {
                    tail_->next = new Node(node_size_, tail_);
                    tail_ = tail_->next;
                    ++node_count_;
                    tail_size_ = 0;
                }
                tail_->keys[tail_size_] = fill_item;
                ++tail_size_;
            }
        }
    }
   private:
    /*
    Implements logic for appending a new element. The actual insertion is passed as a function that takes the node where the key is to be inserted
    so the same logic isn't repeated in three separate functions
    */
    template <typename Function>
    void push_back_template(Function func) {
        if (head_ == nullptr) {
            head_ = tail_ = new Node(node_size_);
            node_count_ = 1;
            func(head_);
        } else if (tail_size_ < node_size_) {
            func(tail_);
        } else {
            tail_->next = new Node(node_size_, tail_);
            ++node_count_;
            tail_ = tail_->next;
            tail_size_ = 0;
            func(tail_);
        }
    }

   public:

    void push_back(const T& key) {
        push_back_template([&](Node* node) {
            node->keys[tail_size_] = key;
            ++tail_size_;
        });
    }

    void push_back(T&& key) {
        push_back_template([&](Node* node) {
            node->keys[tail_size_] = std::move(key);
            ++tail_size_;
        });
    }

    template <typename... Args>
    void emplace_back(Args... args) {
        push_back_template([&](Node* node) {
            node->keys[tail_size_] = T(args...);
            ++tail_size_;
        });
    }

   private:

    void remove_last_node() {
        Node* prev_tail = tail_;
        tail_ = tail_->prev;
        if (tail_ == nullptr) {
            tail_size_ = 0;
            head_ = nullptr;
        } else {
            tail_->next = nullptr;
            tail_size_ = node_size_;
        }
        delete prev_tail;
        --node_count_;
    }

   public:

    void pop_back() {
        if (tail_size_ > 1) {
            --tail_size_;
        }
        else {
            remove_last_node();
        }
    }

    class const_iterator {
        friend class ArrayLinkedList<T>;
       protected:
        Node* current_node_;
        size_t index_;
        size_t node_size_;
        const size_t* tail_size_;

        const_iterator(Node* currentNode, size_t index, size_t node_size, const size_t* tail_size) : 
            current_node_(currentNode), 
            index_(index),
            node_size_(node_size),
            tail_size_(tail_size) {}
       public:
        const_iterator() : 
            current_node_(nullptr), 
            index_(0), 
            node_size_(0), 
            tail_size_(nullptr) {}

        const_iterator& operator++() {
            if ((current_node_->next == nullptr && index_ < *tail_size_ - 1) || (current_node_->next != nullptr && index_ < node_size_ - 1)) {
                ++index_;
            } else {
                current_node_ = current_node_->next;
                index_ = 0;
            }
            return *this;
        }

        const_iterator& operator--() {
            if (index_ > 0) {
                --index_;
            } else {
                current_node_ = current_node_->prev;
                index_ = node_size_ - 1;
            }
            return *this;
        }

        bool operator==(const const_iterator& other) const {
            return current_node_ == other.current_node_ && index_ == other.index_;
        }

        bool operator!=(const_iterator other) const {
            return !(*this == other);
        }

        const T& operator*() const {
            return current_node_->keys[index_];
        }

        const T* operator->() const {
            return &current_node_->keys[index_];
        }
    };

    const_iterator cbegin() const {
        return const_iterator(head_, 0, node_size_, &tail_size_);
    }

    const_iterator cend() const {
        return const_iterator(nullptr, 0, node_size_, &tail_size_);
    }

    const_iterator begin() const {
        return cbegin();
    }

    const_iterator end() const {
        return cend();
    }

    class iterator : public const_iterator {
        friend class ArrayLinkedList<T>;

        iterator(Node* currentNode, size_t index, size_t node_size, const size_t* tail_size) : 
            const_iterator(currentNode, index, node_size, tail_size) {}
       public:
        T& operator*() {
            return this->current_node_->keys[this->index_];
        }

        T* operator->() {
            return &this->current_node_->keys[this->index_];
        }
    };

    iterator begin() {
        return iterator(head_, 0, node_size_, &tail_size_);
    }

    iterator end() {
        return iterator(nullptr, 0, node_size_, &tail_size_);
    }

    // TODO:: reverse iterators

    // Utility that needs iterators (find, contains, erase)

   private:
    /*
    Implementation of logic for searching, for use with different iterator types
    Returns a node pointer and the index of the key in the given node
    */
    std::pair<Node*, size_t> find_key(const T& key) const {
        for (Node* it = head_; it != nullptr; it = it->next) {
            size_t size = it->next == nullptr ? tail_size_ : node_size_;
            for (size_t i = 0; i < size; ++i) {
                if (it->keys[i] == key)
                    return std::make_pair(it, i);
            }
        }

        return std::make_pair(nullptr, 0);
    }

   public:
    const_iterator find(const T& key) const {
        auto [node, index] = find_key(key);
        return const_iterator(node, index, node_size_, &tail_size_);
    }

    iterator find(const T& key) {
        auto [node, index] = find_key(key);
        return iterator(node, index, node_size_, &tail_size_);
    }

    bool contains(const T& key) const {
        return find(key) != end();
    }

   private:
    // Shifts every item in this array from the start_index up to size shift_distance places forward
    static void shift_forward(T* arr, size_t start_index, size_t size, size_t shift_distance) {
        for (size_t i = start_index; i < size; ++i)
            arr[i - shift_distance] = std::move(arr[i]);
    }

    /*
    Implements logic for deleting a single item. This abstracts from the returned and passed iterator type in order to
    not implenment the same logic twice
    */
    template <typename ItType>
    ItType erase_template(ItType pos) {
        size_t start_node_size = pos.current_node_->next == nullptr ? tail_size_ : node_size_;
        shift_forward(pos.current_node_->keys, pos.index_ + 1, start_node_size, 1);

        if (pos.current_node_->next != nullptr) {
            Node* it = pos.current_node_;
            while (it->next != nullptr) {
                it->keys[node_size_ - 1] = std::move(it->next->keys[0]);
                shift_forward(it->next->keys, 1, node_size_, 1);

                it = it->next;
            }

            shift_forward(it->keys, 1, tail_size_, 1);
        }

        --tail_size_;
        if (tail_size_ == 0) {
            bool return_end = false;
            if (pos.current_node_ == tail_)
                return_end = true;
            remove_last_node();

            if (return_end)
                return end();
        }
        return pos;
    }

   public:

    // removes the item at the given position and returns an iterator pointing to the item following the removed item
    iterator erase(iterator pos) {
        return erase_template(pos);
    }

    const_iterator erase(const_iterator pos) {
        return erase_template(pos);
    }

    // TODO: Range deletion
};
