#include "test.hpp"

#include <algorithm>
#include <bit>
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

#include <iopp/bitwise_io.hpp>

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
        auto tmpfile = std::filesystem::temp_directory_path() / "iopp-test-output";
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
        auto in = fin.begin();
        auto end = fin.end();

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

        SUBCASE("Reset and read again") {
            // read
            while(in != end) CHECK(*in++ == *iota_it++);

            // reset
            fin.seekg(0, std::ios::beg);
            in = fin.begin();
            end = fin.end();
            iota_it = str_iota.begin();

            // read again
            while(in != end) CHECK(*in++ == *iota_it++);
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
            CHECK(sink.num_bits_written() == 1);
            CHECK(target.size() == 1);
            CHECK(target[0] == 0);
        }

        SUBCASE("single set bit") {
            sink.write(1);
            sink.flush();
            CHECK(sink.num_bits_written() == 1);
            CHECK(target.size() == 1);
            CHECK(target[0] == 1);
        }
        
        SUBCASE("max bits packed") {
            sink.write(PACK_WORD_MAX, PACK_WORD_BITS);
            CHECK(sink.num_bits_written() == PACK_WORD_BITS);
            CHECK(target.size() == 1);
            CHECK(target[0] == PACK_WORD_MAX);
        }
        
        SUBCASE("max bits packed and single set bit") {
            sink.write(PACK_WORD_MAX, PACK_WORD_BITS);
            sink.write(1);
            sink.flush();
            CHECK(sink.num_bits_written() == PACK_WORD_BITS + 1);
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
            CHECK(sink.num_bits_written() == PACK_WORD_BITS + 17);
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

    constexpr size_t decode_finalizer(PackWord const x) {
        constexpr size_t FINALIZER_BITS = std::bit_width(PACK_WORD_BITS - 1);
        constexpr size_t FINALIZER_LSH = (PACK_WORD_BITS - FINALIZER_BITS) - 1;
        size_t const f = ((x >> FINALIZER_LSH) + 1) % PACK_WORD_BITS;
        return f ? f : PACK_WORD_BITS; // if finalizer is zero, it means we completely filled up the previous word
    }

    template<typename Src>
    void check_ones_onebyone(Src& src, size_t const num) {
        for(size_t i = 0; i < num; i++) {
            CHECK(!src.eof());
            CHECK(src.read() == 1);
        }
        CHECK(src.eof());
    }

    template<typename Src>
    void check_ones_whilegood(Src& src, size_t const num) {
        size_t sum = 0;
        while(src) sum += src.read();
        CHECK(sum == num);
    }

    TEST_CASE("BitwiseIO") {
        std::vector<PackWord> target;
        auto const finalizer_bits = std::bit_width(PACK_WORD_BITS - 1);
        auto const payload_bits = PACK_WORD_BITS - finalizer_bits;
        auto const payload_max = (1ULL << payload_bits) - 1;

        SUBCASE("empty") {
            {
                auto sink = BitPacker(std::back_inserter(target));
                CHECK(sink.num_bits_written() == 0);
            }
            CHECK(target.size() == 0);

            auto src = BitUnpacker(target.begin(), target.end());
            CHECK(src.eof());
        }

        SUBCASE("max_singleword_payload") {
            {
                auto sink = BitPacker(std::back_inserter(target));
                sink.write(payload_max, payload_bits - 1);
                CHECK(sink.num_bits_written() == payload_bits - 1);
            }
            REQUIRE(target.size() == 1);
            REQUIRE(decode_finalizer(target[0]) == payload_bits - 1);

            {
                auto src = BitUnpacker(target.begin(), target.end());
                check_ones_onebyone(src, payload_bits - 1);
            }
            {
                auto src = BitUnpacker(target.begin(), target.end());
                check_ones_whilegood(src, payload_bits - 1);
            }
        }

        SUBCASE("min_multiword_payload") {
            {
                auto sink = BitPacker(std::back_inserter(target));
                sink.write((payload_max << 1) | 1ULL, payload_bits);
                CHECK(sink.num_bits_written() == payload_bits);
            }
            REQUIRE(target.size() == 2);
            REQUIRE(decode_finalizer(target[1]) == payload_bits);

            {
                auto src = BitUnpacker(target.begin(), target.end());
                check_ones_onebyone(src, payload_bits);
            }
            {
                auto src = BitUnpacker(target.begin(), target.end());
                check_ones_whilegood(src, payload_bits);
            }
        }

        SUBCASE("max_possible_payload") {
            {
                auto sink = BitPacker(std::back_inserter(target));
                sink.write(PACK_WORD_MAX, PACK_WORD_BITS);
                CHECK(sink.num_bits_written() == PACK_WORD_BITS);
            }
            REQUIRE(target.size() == 2); // extra word required
            REQUIRE(target[0] == PACK_WORD_MAX);
            REQUIRE(decode_finalizer(target[1]) == PACK_WORD_BITS);

            {
                auto src = BitUnpacker(target.begin(), target.end());
                check_ones_onebyone(src, PACK_WORD_BITS);
            }
            {
                auto src = BitUnpacker(target.begin(), target.end());
                check_ones_whilegood(src, PACK_WORD_BITS);
            }
        }

        SUBCASE("encode_decode") {
            std::vector<size_t> bits = { 64, 64, 64, 8, 64, 32, 24, 16, 15, 64, 9, 37 };
            size_t const cyc = bits.size();
            std::vector<uint64_t> mask(cyc);
            for(size_t i = 0; i < cyc; i++) mask[i] = UINT64_MAX >> (64 - bits[i]);
            
            // encode
            {
                size_t exp_total = 0;
                auto sink = BitPacker(std::back_inserter(target));
                for(uint64_t i = 0; i < iota_size; i++) {
                    auto const j = i % cyc;
                    uint64_t const x = i & mask[j];
                    sink.write(x, bits[j]);
                    exp_total += bits[j];
                }
                CHECK(sink.num_bits_written() == exp_total);
            }

            // decode
            {
                auto src = BitUnpacker(target.begin(), target.end());
                for(uint64_t i = 0; i < iota_size; i++) {
                    auto const j = i % cyc;
                    uint64_t const x = i & mask[j];

                    CHECK(!src.eof());
                    uint64_t const y = src.read(bits[j]);
                    CHECK(x == y);
                }
                CHECK(src.eof());
            }
        }

        SUBCASE("bitwise_file_io") {
            std::vector<size_t> bits = { 64, 64, 64, 8, 64, 32, 24, 16, 15, 64, 9, 37 };
            size_t const cyc = bits.size();
            std::vector<uint64_t> mask(cyc);
            for(size_t i = 0; i < cyc; i++) mask[i] = UINT64_MAX >> (64 - bits[i]);

            auto tmpfile = std::filesystem::temp_directory_path() / "iopp-bitwise-test-output";
            {
                FileOutputStream fos(tmpfile);
                size_t exp_total = 0;
                auto sink = bitwise_output_to(fos);
                for(uint64_t i = 0; i < iota_size; i++) {
                    auto const j = i % cyc;
                    uint64_t const x = i & mask[j];
                    sink.write(x, bits[j]);
                    exp_total += bits[j];
                }
                CHECK(sink.num_bits_written() == exp_total);
            }
            {
                FileInputStream fis(tmpfile);
                auto src = bitwise_input_from(fis.begin(), fis.end());
                for(uint64_t i = 0; i < iota_size; i++) {
                    auto const j = i % cyc;
                    uint64_t const x = i & mask[j];

                    CHECK(!src.eof());
                    uint64_t const y = src.read(bits[j]);
                    CHECK(x == y);
                }
                CHECK(src.eof());
            }
        }
    }

    TEST_CASE("load_file") {
        auto str_iota = load(file_iota);
        auto str_loaded = load_file_str(file_iota);
        CHECK(str_loaded == str_iota);
    }
}

}
