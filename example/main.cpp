#include "ankerl/cista_adapter.h"

#include "cista/serialization.h"
#include "cista/targets/buf.h"

#include <iostream>

constexpr auto const MODE = // opt. versioning + check sum
    cista::mode::WITH_STATIC_VERSION | cista::mode::WITH_INTEGRITY;

auto main() -> int {
    using string_map = cista::offset::ankerl_map<cista::offset::string, cista::offset::string>;

    cista::buf buf;
    {
        auto map = string_map{};
        map["x"] = "y";
        map[std::string_view{"test"}] = "y";
        cista::serialize<MODE>(buf, map);
    }
    {
        std::cout << "DESERIALIZED\n";
        auto const* const deserialized = cista::deserialize<string_map, MODE>(buf);
        std::cout << deserialized->at("x") << "\n";
        for (auto const& [key, val] : *deserialized) {
            std::cout << key << " => " << val << std::endl;
        }
    }
}
