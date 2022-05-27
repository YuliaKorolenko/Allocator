#pragma once

#include <algorithm>
#include <cstddef>
#include <deque>
#include <iostream>
#include <list>
#include <new>
#include <ostream>

template <class Key, class KeyProvider, class Allocator>
class Cache
{
public:
    template <class... AllocArgs>
    Cache(const std::size_t cache_size, AllocArgs &&... alloc_args)
        : m_max_top_size(cache_size)
        , m_max_low_size(cache_size)
        , m_alloc(std::forward<AllocArgs>(alloc_args)...)
    {
    }

    std::size_t size() const
    {
        return top_queue.size() + low_queue.size();
    }

    bool empty() const
    {
        return top_queue.empty() && low_queue.empty();
    }

    template <class T>
    T & get(const Key & key);

    std::ostream & print(std::ostream & strm) const;

    friend std::ostream & operator<<(std::ostream & strm, const Cache & cache)
    {
        return cache.print(strm);
    }

private:
    const std::size_t m_max_top_size;
    const std::size_t m_max_low_size;
    std::list<KeyProvider *> top_queue;
    std::deque<KeyProvider *> low_queue;

    Allocator m_alloc;

    void remove_low()
    {
        m_alloc.template destroy<KeyProvider>(low_queue.back());
        low_queue.pop_back();
    }

    void remove_top()
    {
        auto elem = top_queue.back();
        top_queue.pop_back();
        if (low_queue.size() == m_max_low_size) {
            remove_low();
        }
        low_queue.push_front(elem);
    }
};

template <class Key, class KeyProvider, class Allocator>
template <class T>
inline T & Cache<Key, KeyProvider, Allocator>::get(const Key & key)
{
    auto it = std::find_if(top_queue.begin(), top_queue.end(), [&key](const KeyProvider * elem) {
        return *elem == key;
    });
    if (it != top_queue.end()) {
        auto obj = *it;
        top_queue.erase(it);
        top_queue.insert(top_queue.begin(), obj);
        return static_cast<T &>(*top_queue.front());
    }
    auto it2 = std::find_if(low_queue.begin(), low_queue.end(), [&key](const KeyProvider * elem) {
        return *elem == key;
    });
    if (it2 != low_queue.end()) {
        auto obj = *it2;
        low_queue.erase(it2);
        if (top_queue.size() == m_max_top_size) {
            remove_top();
        }
        top_queue.insert(top_queue.begin(), obj);
        return static_cast<T &>(*top_queue.front());
    }
    if (low_queue.size() == m_max_low_size) {
        remove_low();
    }
    low_queue.push_front(m_alloc.template create<T>(key));
    return static_cast<T &>(*low_queue.front());
}

template <class Key, class KeyProvider, class Allocator>
inline std::ostream & Cache<Key, KeyProvider, Allocator>::print(std::ostream & strm) const
{
    std::cout << "Priority";
    bool first = true;
    for (const auto x : top_queue) {
        if (!first) {
            strm << " ";
        }
        else {
            first = false;
        }
        strm << *x;
    }
    strm << std::endl;

    std ::cout << "NonPriority";
    first = true;
    for (const auto x : low_queue) {
        if (!first) {
            strm << " ";
        }
        else {
            first = false;
        }
        strm << *x;
    }
    strm << std::endl;

    return strm << "Priority: <empty>"
                << "\nRegular: <empty>"
                << "\n";
}
