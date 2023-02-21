#include "pool.h"

PoolAllocator::PoolAllocator(const std::size_t block_size, std::initializer_list<std::size_t> sizes)
    : m_block_size(block_size)
    , m_storage(block_size * sizes.size())
    , m_used_map(sizes.size())
{
    std::vector<std::size_t> sorted(sizes);
    std::sort(sorted.begin(), sorted.end());

    for (std::size_t i = 0; i < sorted.size(); ++i) {
        m_used_map[i].resize(block_size / sorted[i]);
    }
}

// size of element in block from size of block
std::size_t PoolAllocator::element_size(const std::size_t size_of_block) const
{
    return m_block_size / size_of_block;
}

void * PoolAllocator::allocate(const std::size_t _element_size)
{
    auto iter = std::lower_bound(
            m_used_map.begin(),
            m_used_map.end(),
            _element_size,
            [this](std::vector<bool> & vector, std::size_t element_size) {
                return m_block_size / vector.size() < element_size;
            });
    while (iter != m_used_map.end() && element_size(iter->size()) == _element_size) {
        auto iter_in_block = std::find(iter->begin(), iter->end(), false);
        if (iter_in_block != iter->end()) {
            *iter_in_block = true;
            auto offset_of_block = (iter - m_used_map.begin()) * m_block_size;
            auto offset_of_element_in_block = (iter_in_block - iter->begin()) * element_size(iter->size());
            return &m_storage[offset_of_block + offset_of_element_in_block];
        }
        ++iter;
    }
    throw std::bad_alloc{};
}

void PoolAllocator::deallocate(const void * _ptr)
{
    auto b_ptr = static_cast<const std::byte *>(_ptr);
    const auto begin = m_storage.data();

    std::less_equal<const std::byte *> cmp;
    if (cmp(begin, b_ptr) && cmp(b_ptr, &m_storage.back())) {
        const std::size_t
                offset = b_ptr - begin,
                block_number = offset / m_block_size;

        m_used_map[block_number][offset % m_block_size / element_size(m_used_map[block_number].size())] = false;
    }
}
