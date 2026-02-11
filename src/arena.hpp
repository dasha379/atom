#pragma once

#include <cstddef>
#include <cstdlib>

class ArenaAllocator {
public:
    explicit ArenaAllocator(size_t bytes);
    ~ArenaAllocator();

    template <typename T>
    T* alloc();

    ArenaAllocator(const ArenaAllocator& other) = delete;
    ArenaAllocator operator=(const ArenaAllocator& other) = delete;

private:
    size_t m_size;
    std::byte* m_buffer;
    std::byte* m_offset;
};

template <typename T>
T* ArenaAllocator::alloc(){
    void* offset = m_offset;
    m_offset += sizeof(T);
    return static_cast<T*>(offset);
}