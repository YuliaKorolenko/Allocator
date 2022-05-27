#pragma once

#include <cstddef>
#include <list>
#include <new>
#include <vector>

class PoolAllocator
{
public:
    PoolAllocator(unsigned /*min_p*/, unsigned /*max_p*/);

    void * allocate(std::size_t /*n*/);

    void deallocate(const void * /*ptr*/);

private:
    enum class Condition
    {
        Split,
        Free,
        Ocupated,
        NonExist
    };
    struct InformOfMap
    {
        std::list<int>::iterator map_iter;
        size_t place_in_cache;
        Condition condition;
    };
    std::vector<std::byte> m_storage;
    std::vector<InformOfMap> m_used_map;
    std::vector<std::list<int>> freePositions;
    const unsigned int min_p;
    const unsigned int max_p;
    constexpr unsigned upper_bin_power(const std::size_t n) const
    {
        unsigned i = min_p;
        while ((static_cast<std::size_t>(1) << i) < n) {
            ++i;
        }
        return i;
    }
};
