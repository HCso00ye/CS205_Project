// pcm_conversion.h

#ifndef W2R_H
#define W2R_H

#include <string>
#include "rwFile.h" // Assuming this header file contains the declarations for fileReader and fileWriter

// Struct to hold PCM configuration data
struct pcmConfig
{
    int sample_rate;
    int depth;
    int channels;
    long num_samples;
};

// Function declarations
void decodeFile(fileReader &in, fileWriter &out, pcmConfig &config);
void writeJosn(pcmConfig &config, std::string &path);
void wav_to_raw();

#endif // PCM_CONVERSION_H