#include "cache.h"

#include <string>

namespace {

struct String
{
    std::string data;
    bool marked = false;

    String(const std::string & key)
        : data(key)
    {
    }

    bool operator==(const std::string & other) const
    {
        return data == other;
    }
};

struct Int
{
    int data;
    bool marked = false;

    Int(const std::string & key)
        : data(key[0] - 'a')
    {
    }

    bool operator==(const std::string & other) const
    {
        return data == other[0] - 'a';
    }
};

using TestCache = Cache<std::string, String, AllocatorWithPool>;

} // anonymous namespace

int main()
{

    PoolAllocator allc(4 * sizeof(String), std::initializer_list<std::size_t>{sizeof(String)});
    void * p1 = allc.allocate(40);
    void * p2 = allc.allocate(40);
    void * p3 = allc.allocate(40);
    void * p4 = allc.allocate(40);
    allc.deallocate(p1);
    allc.deallocate(p2);
    allc.deallocate(p3);
    allc.deallocate(p4);
}
