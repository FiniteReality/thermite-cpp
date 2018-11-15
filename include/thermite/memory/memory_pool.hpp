#ifndef _MEMORY_POOL_HPP_
#define _MEMORY_POOL_HPP_

#include <stdexcept>
#include <mutex>
#include <optional>
#include <vector>

#include "span.hpp"

// A memory pool based on the .NET Core memory pool

namespace thermite::memory
{

template <class T>
class memory;

template <class T>
class memory_pool;

template <class T>
class pooled_memory;

namespace detail
{

constexpr inline uint32_t get_bucket_index(uint32_t max_length)
{
    // TODO: use lzcnt on x86
    uint32_t remaining = (max_length - 1) >> 4;

    uint32_t pool_index = 0;
    if (remaining > 0xFFFF) { remaining >>= 16; pool_index = 16; }
    if (remaining > 0xFF) { remaining >>= 8; pool_index += 8; }
    if (remaining > 0xF) { remaining >>= 4; pool_index += 4; }
    if (remaining > 0x3) { remaining >>= 2; pool_index += 2; }
    if (remaining > 0x1) { remaining >>= 1; pool_index += 1; }

    return pool_index + remaining;
}
constexpr inline size_t get_max_size(int position)
{
    return position << 16;
}

template <class T>
class bucket
{
private:
    std::size_t _length;
    std::vector<T*> _buffers;
    std::size_t _index;
    std::mutex _lock;

    explicit bucket(std::size_t length, std::size_t count)
        : _length{length}, _buffers{count}, _index{}, _lock{}
    { }

public:
    std::size_t length() const
    {
        return _length;
    }

    std::optional<pooled_memory<T>> rent_memory()
    {
        T* buffer;
        bool allocate = false;
        {
            std::lock_guard<std::mutex> lock{_lock};

            if (_index < _buffers.size())
            {
                buffer = _buffers[_index];
                _buffers[_index++] = nullptr;
                allocate = buffer == nullptr;
            }
        }

        if (allocate)
            buffer = new T[_length];

        return pooled_memory<T>(buffer, _length);
    }

    void return_memory(pooled_memory<T>&& memory)
    {
        // TODO: use proper exception type
        if (memory.size() != _length)
            throw std::runtime_error("memory was not from this pool");

        {
            std::lock_guard<std::mutex> lock{_lock};

            if (_index != 0)
                _buffers[--_index] = memory.span().data();
        }
    }

    friend memory_pool<T>;
};

}

template <class T>
class pooled_memory
{
private:
    explicit pooled_memory(std::size_t size) noexcept
    : _data(new T[size]), _pool_data(_data, size)
    { }

    explicit pooled_memory(nonstd::span<T> span) noexcept
    : _data(nullptr), _pool_data(span)
    { }

public:
    pooled_memory() = default;

    ~pooled_memory()
    {
        if (_data != nullptr)
            delete[] _data;
    }

    // does not support copying as memory is not always owned by us
    pooled_memory(const pooled_memory&) = delete;
    pooled_memory(pooled_memory&&) = default;
    pooled_memory& operator=(const pooled_memory&) = delete;
    pooled_memory& operator=(pooled_memory &&) = default;

    nonstd::span<T> span() const
    {
        return _pool_data;
    }

    std::size_t size() const
    {
        return _pool_data.size();
    }

private:
    T* _data;
    nonstd::span<T> _pool_data;

    friend detail::bucket<T>;
    friend memory_pool<T>;
    friend memory<T>;
};

template <class T>
class memory_pool
{
private:
    std::vector<detail::bucket<T>> _buckets;

public:

    explicit memory_pool(
        std::size_t max_length = 1024 * 1024,
        std::size_t max_arrays = 50)
        : _buckets()
    {
        uint32_t bucket_count = detail::get_bucket_index(max_length);
        for (int i = 0; i <= bucket_count; i++)
            _buckets.emplace_back(detail::get_max_size(i), max_arrays);
    }

    pooled_memory<T> rent_memory(uint32_t amount)
    {
        uint32_t index = detail::get_bucket_index(amount);
        // if the index was too large, return a custom array
        if (index >= _buckets.size())
            return pooled_memory<T>(amount);

        for (int i = index; (i < index + 2) && (i < _buckets.size()); i++)
        {
            auto buffer = _buckets[i].rent_memory();
            if (buffer)
            {
                return buffer.value();
            }
        }

        return pooled_memory(_buckets[index].length());
    }

    void return_memory(pooled_memory<T>&& buffer)
    {
        uint32_t index = detail::get_bucket_index(buffer.size());
        // if the index was too large, ignore it
        if (index < _buckets.size())
        {
            _buckets[index].return_memory(std::move(buffer));
        }
    }
};

extern memory_pool<uint8_t> shared_pool;

}

#endif /* _MEMORY_POOL_HPP_ */
