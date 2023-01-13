#pragma once

#include <algorithm>
#include <iostream>
#include <limits>
#include <numeric>
#include <random>
#include <vector>

namespace additionals {
struct random_struct
{
    random_struct()
        : m_rand_engine(std::random_device{}())
    {
    }
    int get_rand(int to) const
    {
        std::uniform_int_distribution distribution(0, to);
        return distribution(m_rand_engine);
    }

    mutable std::mt19937 m_rand_engine;
}; // end of random_struct

} // namespace additionals

template <class T>
class randomized_queue
{
private:
    template <bool is_const>
    struct Iterator
    {
        friend class randomized_queue;

    private:
        using rand_queue = std::conditional_t<is_const, const randomized_queue, randomized_queue>;

        Iterator(rand_queue * _queue, std::size_t _pos)
            : m_position(_pos)
            , m_map_numbers(_queue->size())
            , m_data(&_queue->m_data)
        {
            std::iota(m_map_numbers.begin(), m_map_numbers.end(), 0);
            std::shuffle(m_map_numbers.begin(), m_map_numbers.end(), _queue->m_random.m_rand_engine);
            m_map_numbers.push_back(npos);
        };

        Iterator(rand_queue * _queue)
            : Iterator(_queue, 0){};

    public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = std::conditional_t<is_const, const T, T>;
        using pointer = value_type *;
        using reference = value_type &;

        Iterator() = default;
        ~Iterator() = default;

        reference operator*() const { return (*m_data)[m_map_numbers[m_position]]; }

        pointer operator->() { return m_data[m_map_numbers[m_position]]; }

        Iterator & operator++()
        {
            ++(this->m_position);
            return *this;
        }

        Iterator operator++(int)
        {
            auto tmp_to_ret = *this;
            operator++();
            return tmp_to_ret;
        }

        friend bool operator==(const Iterator & a, const Iterator & b)
        {
            return equal_data(a, b) &&
                    a.m_position == b.m_position;
        }

        friend bool operator!=(const Iterator & a, const Iterator & b)
        {
            return !(a == b);
        }

    private:
        static constexpr std::size_t npos = std::numeric_limits<unsigned long long>::max();

        std::size_t m_position;
        std::vector<std::size_t> m_map_numbers;
        using type = std::conditional_t<is_const, const std::vector<T>, std::vector<T>>;
        type * m_data;

        static bool equal_data(const Iterator & a, const Iterator & b)
        {
            return a.m_data == b.m_data;
        }
    }; // end of Iterator class

    using iterator = Iterator<false>;
    using const_iterator = Iterator<true>;

    std::vector<T> m_data;
    additionals::random_struct m_random;

public:
    iterator begin()
    {
        return {this};
    }

    iterator end()
    {
        return {this, size()};
    }

    iterator back()
    {
        return {this, size() - 1};
    }

    const_iterator cbegin() const
    {
        return {this};
    }

    const_iterator cend() const
    {
        return {this, size()};
    }

    iterator cback()
    {
        return {this, size() - 1};
    }

    const_iterator begin() const { return cbegin(); }

    const_iterator end() const { return cend(); }

    iterator back() const { return cback(); }

    randomized_queue()
        : m_data(0)
        , m_random()
    {
    }

    ~randomized_queue() = default;

    bool empty() const
    {
        return m_data.empty();
    }

    std::size_t size() const
    {
        return m_data.size();
    }

    template <class T1>
    void enqueue(T1 && el)
    {
        m_data.push_back(std::forward<T1>(el));
    }

    T dequeue()
    {
        auto rand_num = m_random.get_rand(m_data.size() - 1);
        T element_to_return = std::move(m_data[rand_num]);

        m_data[rand_num] = std::move(m_data.back());
        m_data.pop_back();
        return element_to_return;
    }

    const T & sample() const
    {
        return (m_data[m_random.get_rand(m_data.size() - 1)]);
    }
};