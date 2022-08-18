#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <filesystem>
#include <fstream>

namespace iopp::test {
    std::filesystem::path files = "files";
    std::filesystem::path file_iota = files / "iota";

    constexpr size_t operator"" _Ki(unsigned long long s) { return s << 10ULL; }
    constexpr size_t iota_size = 56_Ki;

    std::string load(std::filesystem::path const& path) {
        std::ifstream ifs(path, std::ios::in | std::ios::binary);
        return std::string(std::istreambuf_iterator<char>(ifs), {});
    }

    template<typename InputStream>
    void ensure_iota(InputStream& in, size_t const begin, size_t const end) {
        for(size_t i = begin; i < end; i++) {
            CHECK(in.good()); // make sure stream is still good
            CHECK(in.tellg() == i - begin); // make sure read position is correct

            auto const c = in.get();
            CHECK(c == (i & 0xFF));
        }
    }

    template<typename InputStream>
    void ensure_eof(InputStream& in) {
        CHECK(in.get() == std::char_traits<char>::eof());
        CHECK(!in.good());
    }
}
