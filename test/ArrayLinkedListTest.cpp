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
*/
template <typename ItType>
void iterator_test(ArrayLinkedList<int>& list) {
    ItType it = list.begin();

    for (int i = 0; i < 50; ++i) {
        ASSERT_EQ(*it, i);
        ++it;
    }

    // Push back item while iterating
    list.push_back(30);

    for (int i = 0; i < 50; ++i) {
        ASSERT_EQ(*it, i);
        ++it;
    }

    ASSERT_EQ(*it, 10000);
    ++it;
    ASSERT_EQ(*it, 30);

    ItType itCopy = it;
    ++itCopy;
    ASSERT_EQ(itCopy, list.end());
    ----it;

    // Go through backwards to test decrementing

    for (int i = 49; i > -1; --i) {
        ASSERT_EQ(*it, i);
        --it;
    }

    // revert the push_back from before so the list is in the same state as before the function call
    list.pop_back(); 
    ASSERT_EQ(list.back(), 10000);

    for (int i = 49; i > -1; --i) {
        ASSERT_EQ(*it, i);
        if (i != 0)
            --it;
    }

    ASSERT_EQ(it, list.begin());
}


TEST_F(ArrayLinkedListTest, Iterators) {
    iterator_test<ArrayLinkedList<int>::iterator>(list);

    iterator_test<ArrayLinkedList<int>::const_iterator>(list);
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
    iterator_test<ArrayLinkedList<int>::iterator>(list);
}

TEST_F(ArrayLinkedListTest, Copy) {
    ArrayLinkedList<int> copy(list);

    iterator_test<ArrayLinkedList<int>::iterator>(copy);
    // Change copy and check if the original has changed
    for (int& key : copy)
        key = 100;

    for (int i = 0; i < 142; ++i)
        copy.push_back(100);
    
    iterator_test<ArrayLinkedList<int>::iterator>(list);

    ArrayLinkedList<int> copy_of_copy(copy);
    // Assign copy to list again to test copy assignment (case that other.size() < this->size() in _copy_same_node_size())
    copy = list; 
    iterator_test<ArrayLinkedList<int>::iterator>(copy);

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
    iterator_test<ArrayLinkedList<int>::iterator>(list);
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
*/
template <typename ItType>
void erase_iterator_template(ArrayLinkedList<int>& list) {
    size_t prev_size = list.size();
    ItType it = list.find(40);
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
}

TEST_F(ArrayLinkedListTest, Erase) {
    ArrayLinkedList<int> list_copy(list);
    erase_iterator_template<ArrayLinkedList<int>::iterator>(list);
    list = std::move(list_copy);
    erase_iterator_template<ArrayLinkedList<int>::const_iterator>(list);
}

TEST_F(ArrayLinkedListTest, Size) {
    size_t prev_size = list.size();
    EXPECT_EQ(prev_size, 101);

    for (int i = 0; i < prev_size; ++i) {
        EXPECT_EQ(list.size(), prev_size - i);
        list.pop_back();
    }

    EXPECT_EQ(list.size(), 0);
}