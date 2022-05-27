#include "allocator.h"
#include "math.h"

#include <iostream>

PoolAllocator::PoolAllocator(const unsigned int min_p_, const unsigned int max_p_)
    : min_p(min_p_)
    , max_p(max_p_)
{
    m_storage.resize(1 << max_p);
    freePositions.resize(max_p - min_p + 1);
    freePositions[0].push_back(0);
    m_used_map.resize(1 << (max_p - min_p + 1), {freePositions[0].end(), 0, Condition::NonExist});
    m_used_map[0] = {--freePositions[0].end(), 0, Condition::Free};
    for (int level = max_p_ - min_p_; level >= 0; level--) {
        size_t number = (1 << level) - 1;
        for (size_t i = number, now = 0; i <= 2 * number; i++, now += (1 << (min_p_ + max_p_ - min_p_ - level))) {
            m_used_map[i].place_in_cache = now;
        }
    }
}
//в allocate нахожу первый свободный блок размера, наиболее близкого к нужному. Произвожу split блоков до нужного размера.
void * PoolAllocator::allocate(const std::size_t n)
{
    unsigned int powerN = upper_bin_power(n);
    if (max_p < powerN) {
        throw std::bad_alloc{};
    }
    int number = (1 << (max_p - powerN)) - 1;
    int currentNumber = number;
    int currentLayer = max_p - powerN;
    while (currentNumber >= 0) {
        if (!freePositions[currentLayer].empty()) {
            int i = currentNumber + (freePositions[currentLayer].front() / (1 << (max_p - currentLayer)));
            freePositions[currentLayer].erase(m_used_map[i].map_iter);
            while (!(i >= number && i <= 2 * number)) {
                m_used_map[i].condition = Condition::Split;
                m_used_map[2 * i + 2].condition = Condition::Free;
                //добавить в нужный слой новый элемент
                //добавить на него итератотор в мапу
                freePositions[++currentLayer].push_back(m_used_map[2 * i + 2].place_in_cache);
                m_used_map[2 * i + 2].map_iter = --freePositions[currentLayer].end();
                i = 2 * i + 1;
            }
            m_used_map[i].condition = Condition::Ocupated;
            return &m_storage[m_used_map[i].place_in_cache];
        }
        currentNumber = (currentNumber + 1) / 2 - 1;
        currentLayer--;
    }
    throw std::bad_alloc{};
}

//при удалении блока, в дереве он помечается Free.
// Иду вверх по дереву.
// Если 2 ребенка Split-родителя являются Free, то делаю их NonExist, чтобы Split-родители становились Free
void PoolAllocator::deallocate(const void * ptr)
{
    size_t pos = static_cast<const std::byte *>(ptr) - &m_storage[0];
    long long posInMap = (pos / (1 << min_p)) + (1 << (max_p - min_p)) - 1;
    int currentLayer = max_p - min_p;
    while (m_used_map[posInMap].condition != Condition::Ocupated) {
        posInMap = (posInMap - 1) / 2;
        currentLayer--;
    }
    m_used_map[posInMap].condition = Condition::Free;
    freePositions[currentLayer].push_back(m_used_map[posInMap].place_in_cache);
    m_used_map[posInMap].map_iter = --freePositions[currentLayer].end();
    posInMap = (posInMap - 1) / 2;
    currentLayer--;
    while ((m_used_map[posInMap * 2 + 1].condition == Condition::Free) && (m_used_map[posInMap * 2 + 2].condition == Condition::Free)) {
        m_used_map[posInMap * 2 + 1].condition = Condition::NonExist;
        freePositions[currentLayer + 1].erase(m_used_map[posInMap * 2 + 1].map_iter);
        m_used_map[posInMap * 2 + 2].condition = Condition::NonExist;
        freePositions[currentLayer + 1].erase(m_used_map[posInMap * 2 + 2].map_iter);
        m_used_map[posInMap].condition = Condition::Free;
        freePositions[currentLayer].push_back(m_used_map[posInMap].place_in_cache);
        m_used_map[posInMap].map_iter = --freePositions[currentLayer].end();
        posInMap = (posInMap - 1) / 2;
        currentLayer--;
    }
}

int main()
{

}
