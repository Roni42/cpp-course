#pragma once

#include "allocator.h"

#include <cstddef>
#include <list>
#include <new>
#include <ostream>

template <class Key, class KeyProvider, class Allocator>
class Cache
{
    static_assert(std::is_constructible_v<KeyProvider, const Key &>,
                  "KeyProvider has to be constructable from Key");

private:
    struct CashCell
    {
        KeyProvider * key;
        bool is_used;

        CashCell(KeyProvider * _keyp)
            : key(_keyp)
            , is_used(false)
        {
        }

        CashCell(const CashCell & _cell)
            : CashCell(_cell.key)
        {
        }
    };

    const std::size_t m_max_size;
    Allocator m_alloc;
    std::list<CashCell> m_queue;

public:
    template <class... AllocArgs>
    Cache(const std::size_t cache_size, AllocArgs &&... alloc_args)
        : m_max_size(cache_size)
        , m_alloc(std::forward<AllocArgs>(alloc_args)...)
    {
    }

    std::size_t size() const
    {
        return m_queue.size();
    }

    bool empty() const
    {
        return m_queue.empty();
    }

    template <class T>
    T & get(const Key & key);

    std::ostream & print(std::ostream & strm) const;

    friend std::ostream & operator<<(std::ostream & strm, const Cache & cache)
    {
        return cache.print(strm);
    }
};

template <class Key, class KeyProvider, class Allocator>
template <class T>
inline T & Cache<Key, KeyProvider, Allocator>::get(const Key & key)
{
    auto iter = std::find_if(m_queue.begin(), m_queue.end(), [&key](const CashCell & el) {
        return *el.key == key;
    });

    // key is found
    if (iter != m_queue.end()) {
        iter->is_used = true;
        return *static_cast<T *>(iter->key);
    }

    // there is no such key
    if (m_max_size == m_queue.size()) {  // there is no free place
        while (m_queue.back().is_used) { // saving (second chance) life loop
            m_queue.emplace_front(m_queue.back());
            m_queue.pop_back();
        }
        m_alloc.template destroy<KeyProvider>(m_queue.back().key);
        m_queue.pop_back();
    }

    // new in the queue to avoid double cast
    auto new_element = m_alloc.template create<T>(key);
    m_queue.emplace_front(new_element);
    return *new_element;
}

template <class Key, class KeyProvider, class Allocator>
inline std::ostream & Cache<Key, KeyProvider, Allocator>::print(std::ostream & strm) const
{
    for (const auto ptr : m_queue) {
        strm << *ptr.key << "<" << ptr.is_used << ">"
             << " ";
    }
    strm << "\n";
    return strm;
}
