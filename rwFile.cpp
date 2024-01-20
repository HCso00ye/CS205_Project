
#include "rwFile.h"

#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <string>

using namespace std;

unsigned int fileReader::readBigUInt(int n) {
    if (n > 32) {
        throw runtime_error("n should be less than 32 (fileReader::readBigUInt)");
    }
    while (inputBufferLength < n) {
        unsigned int temp = input.get();
        // cout<< "temp: " << temp << endl;
        if (temp == EOF) {
            throw runtime_error("Reached end of file");
        }
        inputBuffer = (inputBuffer << 8) | temp;
        inputBufferLength += 8;
        // cout << "inputBuffer<n: " << (inputBufferLength<n) << endl;
    }
    inputBufferLength -= n;
    unsigned int result = (unsigned int)(inputBuffer >> inputBufferLength);
    inputBuffer &= (1 << inputBufferLength) - 1;
    if (n < 32) {
        result &= (1 << n) - 1;
    }
    return result;
}

unsigned int fileReader::readLittleUInt(int n) {
    if (n > 32) {
        throw runtime_error("n should be less than 32 (fileReader::readLittleUInt)");
    }
    while (inputBufferLength < n) {
        unsigned int temp = input.get();
        if (temp == EOF) {
            throw runtime_error("Reached end of file");
        }
        inputBuffer |= temp << (inputBufferLength);
        inputBufferLength += 8;
    }
    inputBufferLength -= n;
    unsigned int result = (unsigned int)(inputBuffer & ((1ull << n) - 1));
    inputBuffer >>= n;
    if (n < 32) {
        result &= (1 << n) - 1;
    }
    return result;
}

void fileWriter::writeBigInt(unsigned int data, int n) {
    if (n > 32) {
        std::cout << "n: " << n << std::endl;
        throw std::runtime_error("n should be less than 32 (fileWriter::writeBigInt)");
    }

    outputBuffer = (outputBuffer << n) | (data & ((1ull << n) - 1));
    outputBufferLength += n;
    while (outputBufferLength >= 8) {
        outputBufferLength -= 8;
        unsigned char temp = (outputBuffer >> outputBufferLength);
        output.put(temp);
        CRC8 ^= temp;
        CRC16 ^= temp << 8;
        for (int i = 0; i < 8; i++) {
            CRC8 = (CRC8 << 1) ^ ((CRC8 >> 7) * 0x107);
            CRC16 = (CRC16 << 1) ^ ((CRC16 >> 15) * 0x18005);
        }
        outputBuffer &= (1 << outputBufferLength) - 1;
    }
}

void fileWriter::writeLittleInt(unsigned int data, int n) {
    if (n % 8 != 0) {
        throw std::runtime_error("n should be a multiple of 8 (fileWriter::writeLittleInt)");
    }

    for (int i = 0; i < n / 8; ++i) {
        output.put((char)(data >> (i * 8)));
    }
}
void fileWriter::alignByte() {
    // This function should only be used in wav2flac
    if (outputBufferLength > 0) {
        writeBigInt(0, 8 - outputBufferLength);
    }
}

void fileWriter::closeWriter() {
    alignByte();
    output.flush();
    output.close();
}

void fileReader::closeReader() {
    input.close();
}