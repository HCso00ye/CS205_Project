
#include <fstream>
#include <iostream>
#include <string>

#include "../IOFile/rwFile.h"

using namespace std;

struct pcmConfig {
    int sample_rate;
    int depth;
    int channels;
    long num_samples;
};

void decodeFile(fileReader &in, fileWriter &out, pcmConfig &config) {
    // check RIFF header
    if (in.readBigUInt(32) != 0x52494646) {
        throw runtime_error("Invalid RIFF file header (Wav2flac::encodeFile)");
    }

    int fileSize = in.readLittleUInt(32);

    // check WAVE header
    if (in.readBigUInt(32) != 0x57415645) {
        throw runtime_error("Invalid WAVE file header (Wav2flac::encodeFile)");
    }

    // check fmt chunk
    if (in.readBigUInt(32) != 0x666d7420) {
        throw runtime_error("Unrecognized WAV file chunk 'fmt ' (Wav2flac::encodeFile)");
    }

    // check fmt chunk size
    if (in.readLittleUInt(32) != 16) {
        throw runtime_error("Invalid WAV file chunk size (Wav2flac::encodeFile)");
    }

    // check format
    if (in.readLittleUInt(16) != 1) {
        throw runtime_error("Invalid WAV file format (Wav2flac::encodeFile)");
    }

    // check channel count
    int numChannels = in.readLittleUInt(16);
    if (numChannels <= 0 || numChannels > 8) {
        throw runtime_error("Invalid WAV file channel count (Wav2flac::encodeFile)");
    }
    config.channels = numChannels;

    // check sample rate
    int sampleRate = in.readLittleUInt(32);
    if (sampleRate <= 0 || sampleRate >= (1 << 20)) {
        throw runtime_error("Invalid WAV file sample rate (Wav2flac::encodeFile)");
    }
    config.sample_rate = sampleRate;

    // check byte rate
    int byteRate = in.readLittleUInt(32);

    // check block align
    int blockAlign = in.readLittleUInt(16);

    // check bits per sample
    int sampleDepth = in.readLittleUInt(16);
    config.depth = sampleDepth;

    // check data chunk header
    if (in.readBigUInt(32) != 0x64617461) {
        throw runtime_error("Invalid WAV file data chunk header 'data' (Wav2flac::encodeFile)");
    }

    // check data chunk size
    int sampleDataLen = in.readLittleUInt(32);
    if (sampleDataLen <= 0 || sampleDataLen % (numChannels * (sampleDepth / 8)) != 0) {
        throw runtime_error("Invalid length of audio sample data");
    }
    config.num_samples = sampleDataLen / (numChannels * (sampleDepth / 8));
    cout << "Wav2pcm::decodeFile: header all parsed" << endl;

    for (int i = 0; i < sampleDataLen; i++) {
        out.writeLittleInt(in.readLittleUInt(8), 8);
    }

    cout << "Wav2pcm::decodeFile: all finished" << endl;
}

void writeJosn(pcmConfig &config, string &path) {
    ofstream jsonFile(path, ios::out | ios::trunc);
    if (!jsonFile.is_open()) {
        throw runtime_error("Error opening json file (main::main)");
    }
    jsonFile << "{\n";
    jsonFile << "\t\"sample_rate\": " << config.sample_rate << ",\n";
    jsonFile << "\t\"depth\": " << config.depth << ",\n";
    jsonFile << "\t\"channels\": " << config.channels << ",\n";
    jsonFile << "\t\"num_samples\": " << config.num_samples << "\n";
    jsonFile << "}";
    jsonFile.close();
}

int main() {
    string inputFileName = "ovs.wav";
    string outputFileName = "test.pcm";
    pcmConfig config;
    ifstream inputFile(inputFileName, ios::in | ios::binary);
    ofstream OutputFile(outputFileName, ios::out | ios::trunc | ios::binary);
    if (!inputFile.is_open()) {
        throw runtime_error("Error opening input file (main::main)");
    }
    if (!OutputFile.is_open()) {
        throw runtime_error("Error opening output file (main::main)");
    }
    fileReader reader(inputFile);
    fileWriter writer(OutputFile);
    decodeFile(reader, writer, config);

    // 将config写入json文件
    string jsonPath = "test.json";
    writeJosn(config, jsonPath);

    return 0;
}