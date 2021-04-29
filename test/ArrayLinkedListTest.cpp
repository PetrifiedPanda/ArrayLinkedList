#include <gtest/gtest.h>

#include "../ArrayLinkedList.h"

struct ArrayLinkedListTest : public testing::Test {
    ArrayLinkedList<int> list;

    virtual void SetUp() override {
        for (int i = 0; i < 50; ++i)
            list.push_back(i);

        for (int i = 0; i < 50; ++i)
            list.emplace_back(i);

        list.push_back(10000);
        // Should create 3 nodes
    }
};

/*
Tests the given iterator type (must be a forward iterator)
The list should be unchanged after the function call (despite the list being changed in the process)
The functions should take an ArrayLinkedList<int>& and return begin() and end() for that list
They are workarounds for the method deduction not working anymore
*/
template <typename ItType, typename Func1, typename Func2>
void forward_iterator_test(ArrayLinkedList<int>& list, Func1 begin_func, Func2 end_func) {
    ItType it = begin_func(list);

    for (int i = 0; i < 50; ++i) {
        EXPECT_EQ(*it, i);
        ++it;
    }

    // Push back item while iterating
    list.push_back(30);

    for (int i = 0; i < 50; ++i) {
        EXPECT_EQ(*it, i);
        ++it;
    }

    EXPECT_EQ(*it, 10000);
    ++it;
    EXPECT_EQ(*it, 30);

    ItType itCopy = it;
    ++itCopy;
    EXPECT_EQ(itCopy, end_func(list));
    ----it;

    // Go through backwards to test decrementing

    for (int i = 49; i > -1; --i) {
        EXPECT_EQ(*it, i);
        --it;
    }

    // revert the push_back from before so the list is in the same state as before the function call
    list.pop_back(); 
    EXPECT_EQ(list.back(), 10000);

    for (int i = 49; i > -1; --i) {
        EXPECT_EQ(*it, i);
        if (i != 0)
            --it;
    }

    EXPECT_EQ(it, begin_func(list));
}


TEST_F(ArrayLinkedListTest, ForwardIterators) {
    forward_iterator_test<ArrayLinkedList<int>::iterator>(list, 
    [] (ArrayLinkedList<int>& param) {
        return param.begin();
    },
    [] (ArrayLinkedList<int>& param) {
        return param.end();
    });

    forward_iterator_test<ArrayLinkedList<int>::const_iterator>(list,
    [] (const ArrayLinkedList<int>& param) {
        return param.cbegin();
    },
    [] (const ArrayLinkedList<int>& param) {
        return param.cend();
    });

    auto it = list.begin();
    *it = 2;
    EXPECT_EQ(*it, 2);
    EXPECT_EQ(list.front(), 2);
}

/*
This is essentially the same as forward_iterator_test, but tests for reverse iterators
*/
template <typename ItType, typename Func1, typename Func2>
void reverse_iterator_test(ArrayLinkedList<int>& list, Func1 begin_func, Func2 end_func) {
    ItType it = begin_func(list);
    
    EXPECT_EQ(*it, 10000);
    ++it;


    for (int i = 49; i > -1; --i) { // Error in this loop
        EXPECT_EQ(*it, i);
        ++it;
    }

    ASSERT_EQ(*it, 49);

    for (int i = 49; i > -1; --i) {
        EXPECT_EQ(*it, i);
        ++it;
    }

    EXPECT_EQ(it, end_func(list));
}

TEST_F(ArrayLinkedListTest, ReverseIterators) {
    reverse_iterator_test<ArrayLinkedList<int>::reverse_iterator>(list,
    [] (ArrayLinkedList<int>& param) {
        return param.rbegin();
    },
    [] (ArrayLinkedList<int>& param) {
        return param.rend();
    });

    reverse_iterator_test<ArrayLinkedList<int>::const_reverse_iterator>(list,
    [] (const ArrayLinkedList<int>& param) {
        return param.crbegin();
    },
    [] (const ArrayLinkedList<int>& param) {
        return param.crend();
    });

    auto it = list.rbegin();
    *it = 2;
    EXPECT_EQ(*it, 2);
    EXPECT_EQ(list.back(), 2);
}

TEST_F(ArrayLinkedListTest, Resize) {
    int fill_item = 69;
    size_t added_elements = 50;
    size_t prev_list_size = list.size();
    list.resize(list.size() + added_elements, fill_item);

    auto it = list.begin();

    while (*it != fill_item)
        ++it;

    size_t count = 0;
    while (it != list.end()) {
        EXPECT_EQ(*it, fill_item);
        ++count;
        ++it;
    }

    EXPECT_EQ(count, added_elements);

    list.resize(prev_list_size);

    // Run the iterator test again after resizing the list to the same size as before
    forward_iterator_test<ArrayLinkedList<int>::iterator>(list, 
    [](ArrayLinkedList<int>& param) {
        return param.begin();
    },
    [](ArrayLinkedList<int>& param) {
        return param.end();
    });

    list.resize(0);
    EXPECT_EQ(list.size(), 0);
    EXPECT_EQ(list.begin(), list.end());
}

TEST_F(ArrayLinkedListTest, Copy) {
    ArrayLinkedList<int> copy(list);

    forward_iterator_test<ArrayLinkedList<int>::iterator>(copy,
    [](ArrayLinkedList<int>& param) {
        return param.begin();
    },
    [](ArrayLinkedList<int>& param) {
        return param.end();
    });

    // Change copy and check if the original has changed
    for (int& key : copy)
        key = 100;

    for (int i = 0; i < 142; ++i)
        copy.push_back(100);
    
    forward_iterator_test<ArrayLinkedList<int>::iterator>(list, 
    [](ArrayLinkedList<int>& param) {
        return param.begin();
    },
    [](ArrayLinkedList<int>& param) {
        return param.end();
    });

    ArrayLinkedList<int> copy_of_copy(copy);
    // Assign copy to list again to test copy assignment (case that other.size() < this->size() in _copy_same_node_size())
    copy = list; 
    forward_iterator_test<ArrayLinkedList<int>::iterator>(copy, 
    [](ArrayLinkedList<int>& param) {
        return param.begin();
    },
    [](ArrayLinkedList<int>& param) {
        return param.end();
    });

    // case that other.size() > this->size() in _copy_same_node_size()
    copy = copy_of_copy;

    for (const int& key : copy)
        EXPECT_EQ(key, 100);

    // clear copy and assign copy_of_copy again to test if assignment works with empty lists as well
    copy.clear();
    copy = copy_of_copy;

    for (const int& key : copy)
        EXPECT_EQ(key, 100);
}

TEST_F(ArrayLinkedListTest, Move) {
    // Assuming that copy is correct we can compare the move to the copy
    ArrayLinkedList<int> copy(list); 
    ArrayLinkedList<int> move(std::move(list));

    // Check if list is in a valid state (should be empty)
    EXPECT_EQ(list.size(), 0);

    auto copy_it = copy.begin();
    auto move_it = move.begin();
    while (copy_it != copy.end() && move_it != move.end()) {
        EXPECT_EQ(*move_it, *copy_it);
        ++copy_it;
        ++move_it;
    }

    // fill list to test move assignment into non-empty list
    for (int i = 0; i < 231; ++i)
        list.push_back(rand());

    list = std::move(move);
    forward_iterator_test<ArrayLinkedList<int>::iterator>(list, 
    [](ArrayLinkedList<int>& param) {
        return param.begin();
    },
    [](ArrayLinkedList<int>& param) {
        return param.end();
    });
}

TEST_F(ArrayLinkedListTest, Contains) {
    for (int i = 0; i < 50; ++i)
        EXPECT_TRUE(list.contains(i));

    EXPECT_TRUE(list.contains(10000));

    EXPECT_FALSE(list.contains(6234));
}

TEST_F(ArrayLinkedListTest, Find) {
    auto it = list.find(3);
    EXPECT_EQ(*it, 3);
    
    --it;
    EXPECT_EQ(*it, 2);

    ++++it;
    EXPECT_EQ(*it, 4);

    it = list.find(10000);
    EXPECT_EQ(*it, 10000);
    auto itCopy = it;
    ++itCopy;
    EXPECT_EQ(itCopy, list.end());

    auto invalid_it = list.find(234243);
    EXPECT_EQ(invalid_it, list.end());
}

TEST_F(ArrayLinkedListTest, Indexing) {
    for (size_t i = 0; i < list.size(); ++i) {
        auto& key = list.at(i);
        if (i < 50)
            EXPECT_EQ(key, i);
        else if (i < 100)
            EXPECT_EQ(key, i - 50);
        else
            EXPECT_EQ(key, 10000);

        key = 100;
    }

    // For testing the const implementation of at()
    const auto& list_ref = list;
    for (size_t i = 0; i < list.size(); ++i) {
        EXPECT_EQ(list_ref.at(i), 100);
    }
}

/*
This is the erase test abstracted for different iterator types (only forward iterators) to make
testing the versions of erase with different iterators easier to test
the function should take a reference to an ArrayLinkedList<int> and an int to search for
*/
template <typename ItType, typename Func>
void erase_iterator_template(ArrayLinkedList<int>& list, Func find_func) {
    size_t prev_size = list.size();
    ItType it = find_func(list, 40);
    ItType after_it = list.erase(it);
    EXPECT_EQ(*after_it, 41);

    for (int i = 41; i < 50; ++i) {
        EXPECT_EQ(*after_it, i);
        ++after_it;
    }

    for (int i = 0; i < 50; ++i) {
        EXPECT_EQ(*after_it, i);
        ++after_it;
    }

    EXPECT_EQ(*after_it, 10000);
    ++after_it;
    EXPECT_EQ(after_it, list.end());

    EXPECT_EQ(list.size(), prev_size - 1);

    list.resize(1);
    list.erase(list.begin());
    EXPECT_EQ(list.size(), 0);
    EXPECT_EQ(list.begin(), list.end());
}

TEST_F(ArrayLinkedListTest, Erase) {
    ArrayLinkedList<int> list_copy(list);
    erase_iterator_template<ArrayLinkedList<int>::iterator>(list, [](ArrayLinkedList<int>& param, int to_search) {
        return param.find(to_search);
    });
    list = std::move(list_copy);
    erase_iterator_template<ArrayLinkedList<int>::const_iterator>(list, [](const ArrayLinkedList<int>& param, int to_search) {
        return param.find(to_search);
    });
}

TEST_F(ArrayLinkedListTest, Size) {
    size_t prev_size = list.size();
    EXPECT_EQ(prev_size, 101);

    for (int i = 0; i < prev_size; ++i) {
        EXPECT_EQ(list.size(), prev_size - i);
        list.pop_back();
    }

    EXPECT_EQ(list.size(), 0);

    for (int i = 0; i < prev_size; ++i) {
        EXPECT_EQ(list.size(), i);
        list.push_back(rand());
    }

    EXPECT_EQ(list.size(), prev_size);

    ArrayLinkedList<int> other;
    size_t other_size = 5;
    for (size_t i = 0; i < other_size; ++i)
        other.push_back(rand());

    EXPECT_EQ(other.size(), other_size);

    ArrayLinkedList<int> list_copy = list;
    EXPECT_EQ(list_copy.size(), list.size());
    list = other;
    EXPECT_EQ(list.size(), other.size());

    list = std::move(list_copy);
    EXPECT_EQ(list_copy.size(), 0);
    EXPECT_EQ(list.size(), prev_size);

    list = std::move(other);
    EXPECT_EQ(other.size(), 0);
    EXPECT_EQ(list.size(), other_size);
}

TEST_F(ArrayLinkedListTest, InitializerList) {
    auto init_list = {1, 2, 3, 4, 5, 6};

    // Constructor test
    ArrayLinkedList<int> test_list(init_list);
    EXPECT_EQ(test_list.size(), init_list.size());

    auto init_list_it = init_list.begin();
    auto list_it = test_list.begin();
    while (init_list_it != init_list.end() && list_it != test_list.end()) {
        EXPECT_EQ(*list_it, *init_list_it);
        ++init_list_it;
        ++list_it;
    }

    EXPECT_EQ(init_list_it, init_list.end());
    EXPECT_EQ(list_it, test_list.end());

    // Assignment operator test
    list = init_list;

    init_list_it = init_list.begin();
    list_it = list.begin();
    while (init_list_it != init_list.end() && list_it != list.end()) {
        EXPECT_EQ(*list_it, *init_list_it);
        ++init_list_it;
        ++list_it;
    }

    EXPECT_EQ(init_list_it, init_list.end());
    EXPECT_EQ(list_it, list.end());
}