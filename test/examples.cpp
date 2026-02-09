// nb: we don't actually test anything here, we just make sure the examples compile as advertised

#include <iopp/file_input_stream.hpp>
#include <iopp/file_output_stream.hpp>

int example_file_io(int argc, char** argv) {
	if(argc < 3) return -1; // usage: [INPUT] [OUTPUT]

	// read the last 123 characters from the input file and write them to the output file
	char buf[123];
	
	{
        iopp::FileInputStream fin(argv[1]);
        fin.seekg(123, std::ios::end);
        fin.read(buf, 123);
        if(fin.gcount() == 123) {
            return 0; // success
        } else {
            return 1; // error
        }
	}
	{
		iopp::FileOutputStream fout(argv[2]);
		fout.write(buf, 123);
	}
}

#include <iopp/file_input_stream.hpp>
#include <iopp/file_output_stream.hpp>
#include <iopp/stream_output_iterator.hpp>

int example_stream_iterators(int argc, char** argv) {
	if(argc < 3) return -1; // usage: [INPUT] [OUTPUT]

	// perform a byte-wise copy of the input file to the output file
    iopp::FileInputStream fin(argv[1]);
    iopp::FileOutputStream fout(argv[2]);

    iopp::StreamOutputIterator out(fout);
    for(char const c : fin) *out++ = c;
    // alternatively: std::copy(fin.begin(), fin.end(), iopp::StreamOutputIterator(fout));
    
    return 0;
}

#include <iopp/bitwise_io.hpp>

int example_bitwise_io(int argc, char** argv) {
	if(argc < 2) return -1; // usage: [OUTPUT]
    
    // do some bitwise output to the output file
    {
        iopp::FileOutputStream fout(argv[1]);
        auto bits = iopp::bitwise_output_to(fout);
        bits.write(0); // write a 0-bit
        bits.write(1); // write a 1-bit
        bits.write(0b1011, 4); // write the 4 lowest bits of the given word
    }
    
    // and now read it back bitwise
    bool ok = true;
    {
        iopp::FileInputStream fin(argv[1]);
        auto bits = iopp::bitwise_input_from(fin);
        
        bool const b1 = bits.read();  // read a single bit
        bool const b2 = bits.read();  // read a single bit
        auto const n  = bits.read(4); // read 4 bits into the low bits of the returned word
        
        ok = (b1 == 0 && b2 == 1 && n == 0b1011);
    }
    
    return ok ? 0 : 1;
}

void example_bitwise_input_iterators(int argc, char** argv) {
    iopp::FileInputStream fin(argv[1]);
    auto bits = iopp::bitwise_input_from(fin.begin(), fin.end());
    size_t num_bits_read = 0;
    while(bits) {
        bits.read();
        ++num_bits_read;
    }
}

#include <algorithm>
#include <iopp/file_output_stream.hpp>
#include <iopp/stream_output_iterator.hpp>
#include <iopp/memory_mapped_file.hpp>

int example_mmap(int argc, char** argv) {
	if(argc < 3) return -1; // usage: [OUTPUT]
    
    if(iopp::MemoryMappedFile::available()) { // test if we can do this on this OS
        // memory-map the input file
        iopp::MemoryMappedFile mmap(argv[1]);
        char const* bytes = (char const*)mmap.data();

        // perform a byte-wise copy to the output
        iopp::FileOutputStream fout(argv[2]);
        std::copy(bytes, bytes + mmap.size(), iopp::StreamOutputIterator(fout));

        return 0;
    } else {
        return -1;
    }
}

#include <iostream>
#include <iopp/file_input_stream.hpp>
#include <iopp/util/overlapping_blocks.hpp>

int example_overlapping_blocks(int argc, char** argv) {
    if(argc < 2) return -1; // usage: [INPUT]

    iopp::FileInputStream fin(argv[1]);
    iopp::OverlappingBlocks block(fin, 100, 10); // blockwise processing using blocks of 100 bytes and an overlap of 10 bytes
    do {
        // identify what block we are processing
        if(block.first()) {
            std::cout << "Processing the first block with offset: " << block.offset() << std::endl;
        } else if(block.last()) {
            std::cout << "Processing the last block with offset: " << block.offset() << std::endl;
        } else {
            std::cout << "Processing a block with offset: " << block.offset() << std::endl;
        }

        // print the current block
        std::cout << "\tThe content of the current block is:    ";
        for(size_t i = 0; i < block.size(); i++) {
            std::cout << block[i];
        }
        std::cout << std::endl;

        // print the overlap from the previous block
        std::cout << "\tThe overlap from the previous block is: ";
        for(ssize_t i = 0; i < block.overlap(); i++) {
            std::cout << block[-i];
        }
        std::cout << std::endl;
    } while(block.advance()); // advance returns false when the last block has been processed

    return 0;
}

int main(int argc, char** argv) {
    example_file_io(argc, argv);
    example_stream_iterators(argc, argv);
    example_bitwise_io(argc, argv);
    example_bitwise_input_iterators(argc, argv);
    example_mmap(argc, argv);
    example_overlapping_blocks(argc, argv);
    return 0;
}
