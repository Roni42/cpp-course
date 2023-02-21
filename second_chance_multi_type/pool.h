#pragma once

#include <algorithm>
#include <cstddef>
#include <functional>
#include <initializer_list>
#include <new>
#include <vector>

class PoolAllocator
{
private:
    static constexpr std::size_t npos = static_cast<std::size_t>(-1);

    const std::size_t m_block_size;
    std::vector<std::byte> m_storage;
    std::vector<std::vector<bool>> m_used_map;

    std::size_t element_size(const std::size_t size_of_block) const;

public:
    PoolAllocator(const std::size_t block_size, std::initializer_list<std::size_t> sizes);
    void * allocate(const std::size_t _vector);
    void deallocate(const void * _ptr);
};
