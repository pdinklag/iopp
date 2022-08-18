#include "test.hpp"

#include <algorithm>
#include <fstream>
#include <iterator>
#include <random>
#include <sstream>

#include <iopp/file_input_stream.hpp>
#include <iopp/file_output_stream.hpp>
#include <iopp/load_file.hpp>
#include <iopp/memory_mapped_file.hpp>
#include <iopp/stream_input_iterator.hpp>
#include <iopp/stream_output_iterator.hpp>

#include <iopp/util/bit_packer.hpp>
#include <iopp/util/bit_unpacker.hpp>
#include <iopp/util/char_packer.hpp>
#include <iopp/util/char_unpacker.hpp>

namespace iopp::test {

TEST_SUITE("io") {
    TEST_CASE("MemoryMappedFile") {
        if constexpr(MemoryMappedFile::available()) {
            std::string str_iota = load(file_iota);
            MemoryMappedFile mmap(file_iota);
            CHECK(mmap.size() == iota_size);

            char const* data = (char const*)mmap.data();
            for(size_t i = 0; i < iota_size; i++) {
                CHECK(data[i] == str_iota[i]);
            }
        }
    }

    TEST_CASE("FileInputStream") {
        SUBCASE("read fully") {
            FileInputStream in(file_iota);
            ensure_iota(in, 0, iota_size);
            ensure_eof(in);
        }

        SUBCASE("read prefix") {
            FileInputStream in(file_iota, 0, 24_Ki);
            ensure_iota(in, 0, 24_Ki);
            ensure_eof(in);
        }

        SUBCASE("read suffix") {
            FileInputStream in(file_iota, 24_Ki);
            ensure_iota(in, 24_Ki, iota_size);
            ensure_eof(in);
        }

        SUBCASE("read substring") {
            FileInputStream in(file_iota, 8_Ki, 24_Ki);
            ensure_iota(in, 8_Ki, 24_Ki);
            ensure_eof(in);
        }

        SUBCASE("read after std::move") {
            FileInputStream in;
            {
                FileInputStream init(file_iota);
                in = std::move(init);
            }
            ensure_iota(in, 0, iota_size);
            ensure_eof(in);
        }

        SUBCASE("seek from beginning") {
            FileInputStream in(file_iota);
            in.seekg(0x1234, std::ios::beg);
            CHECK(in.good());
            CHECK(in.tellg() == 0x1234);

            constexpr size_t expect = 0x1234 & 0xFF;
            CHECK(in.get() == expect);
        }

        SUBCASE("seek from end") {
            FileInputStream in(file_iota);
            in.seekg(-0x1234, std::ios::end);
            CHECK(in.good());
            CHECK(in.tellg() == iota_size - 0x1234);

            constexpr size_t expect = (iota_size - 0x1234) & 0xFF;
            CHECK(in.get() == expect);
        }

        SUBCASE("seek from offset") {
            FileInputStream in(file_iota);
            in.seekg(0x1234, std::ios::beg);
            in.seekg(0x1234, std::ios::cur);
            CHECK(in.good());
            CHECK(in.tellg() == 0x2468);

            constexpr size_t expect = 0x2468 & 0xFF;
            CHECK(in.get() == expect);
        }

        SUBCASE("seek in substring") {
            FileInputStream in(file_iota, 0x1234, 0x2468);
            in.seekg(0x123, std::ios::beg);
            CHECK(in.tellg() == 0x123);
            {
                constexpr size_t expect = (0x1234 + 0x123) & 0xFF;
                CHECK(in.get() == expect);
            }

            in.seekg(-0x123, std::ios::end);
            CHECK(in.tellg() == 0x1234 - 0x123);
            {
                constexpr size_t expect = (0x2468 - 0x123) & 0xFF;
                CHECK(in.get() == expect);
            }
        }

        SUBCASE("non-existing file") {
            auto fpath = std::filesystem::temp_directory_path() / "____isurehopethisfiledoesntexist";
            REQUIRE(!std::filesystem::exists(fpath));
            CHECK_THROWS(FileInputStream(fpath));
        }
    }

    TEST_CASE("FileOutputStream") {
        // generate a random string
        auto tmpfile = std::filesystem::temp_directory_path() / "tdc-test-output";
        std::filesystem::remove(tmpfile);

        std::string str_iota = load(file_iota);
        {
            FileOutputStream out(tmpfile);
            for(size_t i = 0; i < iota_size; i++) {
                CHECK(out.tellp() == i);
                out.put(str_iota[i]);
            }
        }
        CHECK(load(tmpfile) == str_iota);

        std::filesystem::remove(tmpfile);
    }

    TEST_CASE("StreamInputIterator") {
        std::string str_iota = load(file_iota);
        auto iota_it = str_iota.begin();
        FileInputStream fin(file_iota);
        StreamInputIterator<FileInputStream> in(fin);
        StreamInputIterator<FileInputStream> end;

        SUBCASE("Read + prefix increment") {
            while(in != end) {
                auto const c = *iota_it++;
                CHECK(*in == c);
                ++in;
            }
        }

        SUBCASE("Postfix increment + read") {
            while(in != end) {
                auto const c = *iota_it++;
                CHECK(*in++ == c);
            }
        }
    }

    TEST_CASE("StreamOutputIterator") {
        std::string str_iota = load(file_iota);
        std::ostringstream stream;
        std::copy(str_iota.begin(), str_iota.end(), StreamOutputIterator(stream));
        stream.flush();
        CHECK(stream.str() == str_iota);
    }

    TEST_CASE("CharPacking") {
        std::string s;

        {
            auto out = CharUnpacker(std::back_inserter(s));
            *out++ = 0x74'75'64'6F'63'6F'6D'70ULL;
            *out++ = 0x3D'61'77'65'73'6F'6D'65ULL;
        }
        CHECK(s == "tudocomp=awesome");

        {
            std::vector<PackWord> v;
            std::copy(CharPacker(s.begin(), s.end()), {}, std::back_inserter(v));

            CHECK(v.size() == 2);
            CHECK(v[0] == 0x74'75'64'6F'63'6F'6D'70ULL);
            CHECK(v[1] == 0x3D'61'77'65'73'6F'6D'65ULL);
        }
    }

    TEST_CASE("BitPacker") {
        std::vector<PackWord> target;
        auto sink = BitPacker(std::back_inserter(target));

        CHECK(target.empty());
        
        SUBCASE("single clear bit") {
            sink.write(0);
            sink.flush();
            CHECK(target.size() == 1);
            CHECK(target[0] == 0);
        }

        SUBCASE("single set bit") {
            sink.write(1);
            sink.flush();
            CHECK(target.size() == 1);
            CHECK(target[0] == 1);
        }
        
        SUBCASE("max bits packed") {
            sink.write(PACK_WORD_MAX, PACK_WORD_BITS);
            CHECK(target.size() == 1);
            CHECK(target[0] == PACK_WORD_MAX);
        }
        
        SUBCASE("max bits packed and single set bit") {
            sink.write(PACK_WORD_MAX, PACK_WORD_BITS);
            sink.write(1);
            sink.flush();
            CHECK(target.size() == 2);
            CHECK(target[0] == PACK_WORD_MAX);
            CHECK(target[1] == 1);
        }

        SUBCASE("pack_pos") {
            CHECK(sink.pack_pos() == 0);
            sink.write(0, 12);
            CHECK(sink.pack_pos() == 12);
            sink.write(PACK_WORD_MAX, 5);
            CHECK(sink.pack_pos() == 17);
            sink.write(PACK_WORD_MAX, PACK_WORD_BITS);
            CHECK(sink.pack_pos() == 17); // full circle
        }
    }

    TEST_CASE("BitUnpacker") {
        PackWord data[] = { PACK_WORD_MAX, 1ULL };
        auto src = BitUnpacker(data);

        SUBCASE("single bit") {
            CHECK(src.read() == 1);
        }

        SUBCASE("max bits packed and single bits") {
            CHECK(src.read(PACK_WORD_BITS) == PACK_WORD_MAX);
            CHECK(src.read() == 1);
            CHECK(src.read() == 0);
        }

        SUBCASE("pack_pos") {
            PackWord data2[] = { PACK_WORD_MAX, PACK_WORD_MAX };
            auto src2 = BitUnpacker(data2);
            CHECK(src2.pack_pos() == 0);
            src2.read(12);
            CHECK(src2.pack_pos() == 12);
            src2.read(5);
            CHECK(src2.pack_pos() == 17);
            src.read(PACK_WORD_BITS);
            CHECK(src2.pack_pos() == 17); // full circle
        }
    }

    TEST_CASE("load_file") {
        auto str_iota = load(file_iota);
        auto str_loaded = load_file_str(file_iota);
        CHECK(str_loaded == str_iota);
    }
}

}
