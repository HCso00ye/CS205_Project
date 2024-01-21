
#ifndef RWFILE_H
#define RWFILE_H

#include <iostream>
#include <string>

class fileReader {
   private:
    std::ifstream &input;
    unsigned long long inputBuffer;
    int inputBufferLength;

   public:
    explicit fileReader(std::ifstream &inStream) : input(inStream) {
        inputBuffer = 0;
        inputBufferLength = 0;
    }

    unsigned int readBigUInt(int n);

    unsigned int readLittleUInt(int n);

    void closeReader();
};

class fileWriter {
   private:
    std::ofstream &output;
    unsigned long long outputBuffer;
    int outputBufferLength;

   public:
    explicit fileWriter(std::ofstream &outStream) : output(outStream) {
        outputBuffer = 0;
        outputBufferLength = 0;
    }

    void writeLittleInt(unsigned int data, int n);

    void writeBigInt(unsigned int data, int n);

    void alignByte();

    void closeWriter();
};

#endif
