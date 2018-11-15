#ifndef _MEMORY_HPP_
#define _MEMORY_HPP_

#include "memory_pool.hpp"

namespace thermite::memory
{

template <typename T>
class memory
{
private:
    pooled_memory<T> _memory;
    memory_pool<T>* _pool;

public:

    memory() = default;

    explicit memory(std::vector<T>& data)
        : _memory{data}
    { }

    explicit memory(std::size_t size, const memory_pool<T>& pool = shared_pool)
        : _memory{pool.rent_memory(size)}, _pool{pool}
    { }

    ~memory()
    {
        if (_pool != nullptr)
            _pool->return_memory(std::move(_memory));
    }

    // does not support copying as memory is not owned by us
    memory(const memory&) = delete;
    memory(memory&&) = default;
    memory& operator=(const memory&) = delete;
    memory& operator=(memory &&) = default;

    pooled_memory<T>& data()
    {
        return _memory;
    }

    std::size_t size() const
    {
        return _memory.size();
    }
};

}

#endif /* _MEMORY_HPP_ */
