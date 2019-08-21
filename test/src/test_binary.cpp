#include "Utils/Binary.hpp"
#include <iostream>
#include <cassert>
#include <cstring>
#include <vector>

using daqling::utilities::Binary;

int main(int, char*[])
{
    // ctors, operator==, operator!=
    {
        const Binary ba;
        assert(ba.size() == 0);
        assert(ba == ba);

        const Binary bb{42}; // XXX: do we have a use case for this?

        const std::vector<uint8_t> bytes = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
        Binary bc{bytes.data(), bytes.size()};
        assert(std::strncmp(reinterpret_cast<const char*>(bytes.data()), static_cast<char*>(bc.data()), bytes.size()) == 0);
        const Binary bd{std::move(bc)};
        assert(bc.data() == nullptr);
        assert(std::strncmp(reinterpret_cast<const char*>(bytes.data()), static_cast<const char*>(bd.data()), bytes.size()) == 0);

        assert(bb != bd);
    }

    // operator=, operator+=
    {
        Binary ba;
        const char* str = "some string";
        const Binary bb{str, std::strlen(str)};
        ba = bb;
        assert(ba == bb);

        Binary bc;
        bc += bb;
        bc += bb;
        assert(std::strncmp("some stringsome string", static_cast<char*>(bc.data()), std::strlen(str) * 2) == 0);
    }
}
