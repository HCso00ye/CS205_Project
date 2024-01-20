#include <map>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <algorithm>


using namespace std;

enum AudioFormat{
    WAVE_FORMAT_PCM,
    WAVE_FORMAT_ADPCM,
    WAVE_FORMAT_ALAW,
    WAVE_FORMAT_MULAW,
    WAVE_FORMAT_EXTENSIBLE
};

struct WAVHeader {
    uint16_t audio_format_code;
    uint16_t channels;
    uint32_t sample_rate;
    uint32_t bytes_per_sec;
    uint16_t block_align;
    uint16_t bits_per_sample;
    uint16_t has_cb_content;
    uint16_t valid_bits_per_sample;
    uint32_t channel_mask;
    uint16_t cb_size;
    uint16_t sub_format[16];
    uint16_t has_fact_chunk;
    uint32_t fact_chunk_data;
    uint32_t riff_chunk_size;
    uint32_t format_chunk_size;
    uint32_t fact_chunk_size;
    uint32_t data_chunk_size;
    FILE *fi;
    FILE *fo;
};

uint8_t chunk_buf[10000000];
int sample_buf[2][10000000];

int audio_format_code(enum AudioFormat audio_format) {
    switch (audio_format) {
        case WAVE_FORMAT_PCM:
            return 0x0001;
        case WAVE_FORMAT_ADPCM:
            return 0x0002;
        case WAVE_FORMAT_ALAW:
            return 0x0006;
        case WAVE_FORMAT_MULAW:
            return 0x0007;
        case WAVE_FORMAT_EXTENSIBLE:
            return 0xFFFE;
        default:
            return 0;
    }
}

void print_menu() {
    printf("\n");
    printf("Menu:\n");
    printf("\t1. Raw to WAV\n");
    printf("\t2. WAV to FLAC\n");
    printf("\t3. Exit\n");
    printf("\n");
}

std::map<string, string> readConfig(const string& filename) {
    ifstream configFile(filename);
    map<string, string> configValues;
    string line, section;

    while (getline(configFile, line)) {
        if (line[0] == '[') {
            section = line.substr(1, line.find(']') - 1);
            continue;
        }

        istringstream is_line(line);
        string key;
        if (getline(is_line, key, '=')) {
            string value;
            if (getline(is_line, value)) {
                configValues[section + "." + key] = value;
            }
        }
    }

    return configValues;
}


struct WAVHeader read_header() {
    // Initialize header
    struct WAVHeader header;
    char filename[100];
    while (1) {
        printf("Enter the input file path: ");
        scanf("%s", filename);
        header.fi = fopen(filename, "rb");
        if (header.fi == NULL) {
            puts("File does not exist, please input again!");
        } else {
            break;
        }
    }
    // Get file size
    fseek(header.fi, 0, SEEK_END);
    header.data_chunk_size = ftell(header.fi);
    fseek(header.fi, 0, SEEK_SET);
    printf("Successfully read the file with a size %u Bytes\n", header.data_chunk_size);
    // Read the filename of the output file
    printf("Enter the output file path: ");
    scanf("%s", filename);
    header.fo = fopen(filename, "wb");
    // format code
    enum AudioFormat audio_format = WAVE_FORMAT_PCM;
    

    printf("Enter the info file path: ");
    scanf("%s", filename);
    auto config = readConfig(filename);

    int format = stoi(config["info.format"]);
    unsigned int channels = stoi(config["info.channels"]);
    unsigned int sample_rate = stoul(config["info.sample_rate"]);
    unsigned int depth = stoi(config["info.depth"]);

    switch (format) {
        case 1:
            audio_format = WAVE_FORMAT_PCM;
            header.has_fact_chunk = 0;
            break;
        case 2:
            audio_format = WAVE_FORMAT_ADPCM;
            header.has_fact_chunk = 1;
             break;
        case 3:
            audio_format = WAVE_FORMAT_ALAW;
            header.has_fact_chunk = 1;
            break;
        case 4:
            audio_format = WAVE_FORMAT_MULAW;
            header.has_fact_chunk = 1;
            break;
        case 5:
            audio_format = WAVE_FORMAT_EXTENSIBLE;
            header.has_fact_chunk = 0;
            break;
        default:
            puts("Bad input, please input again!");
            break;
    }

    header.audio_format_code = audio_format_code(audio_format);
    header.channels = channels;
    header.sample_rate = sample_rate;
    header.bits_per_sample = depth;

    // Calculate block_align
    header.block_align = header.channels * header.bits_per_sample / 8;
    // Calculate bytes_per_sec
    header.bytes_per_sec = header.sample_rate * header.block_align;
    // Read cbContent
    header.has_cb_content = 0;
    //printf("Does Format chunk contain cbContent? (Y/N): ");
    // char cmd[100];
    // scanf("%s", cmd);
    // if (strcmp(cmd, "Y") == 0) {
    //     header.has_cb_content = 1;
    //     printf("Input the number of valid bits per sample: ");
    //     scanf("%hu", &header.valid_bits_per_sample);
    //     printf("Input the channel_mask (decimal): ");
    //     scanf("%u", &header.channel_mask);
    //     printf("Input the sub format (note: Input 16 bytes in decimal using little-endian byte order, separated by spaces): ");
    //     for (int i = 0; i < 16; i++) {
    //         scanf("%hu", &header.sub_format[i]);
    //     }
    // } else {
    //     header.has_cb_content = 0;
    // }

    // Calculate the format chunk size
    if (header.has_cb_content) {
        // We don't handle that cbSize is zero
        header.cb_size = 22;
        header.format_chunk_size = 40;
    } else {
        header.cb_size = 0;
        header.format_chunk_size = 16;
    }
    // Read fact chunk
    if (header.has_fact_chunk) {
        printf("Input the total sample number: ");
        header.fact_chunk_size = 4;
        scanf("%u", &header.fact_chunk_data);
    } else {
        header.fact_chunk_size = 0;
    }
    // Calculate the size of RIFF chunk
    header.riff_chunk_size = 4 + header.format_chunk_size + 8;
    if (header.has_fact_chunk) {
        header.riff_chunk_size += header.fact_chunk_size + 8;
    }
    header.riff_chunk_size += header.data_chunk_size + 8;
    return header;
}

void write_bytes_little(FILE *f, const void *buf, uint32_t size) {
    char *buf_char = (char *) buf;
    for (uint32_t i = 0; i < size; i++) {
        fwrite(buf_char + i, 1, 1, f);
    }
}

void write_bytes_big(FILE *f, const void *buf, uint32_t size) {
    if(size == 0){
        return;
    }
    char *buf_char = (char *) buf;
    for (int32_t i = (int32_t)size - 1; i >= 0; i--) {
        fwrite(buf_char + i, 1, 1, f);
    }
}

void read_bytes_little(FILE *f, const void *buf, uint32_t size) {
    char *buf_char = (char *) buf;
    for (uint32_t i = 0; i < size; i++) {
        fread(buf_char + i, 1, 1, f);
    }
}

void raw_to_wav(struct WAVHeader header) {
    // Write riff header
    write_bytes_little(header.fo, "RIFF", 4);
    write_bytes_little(header.fo, &header.riff_chunk_size, 4);
    write_bytes_little(header.fo, "WAVE", 4);
    // Write format header
    write_bytes_little(header.fo, "fmt ", 4);
    write_bytes_little(header.fo, &header.format_chunk_size, 4);
    write_bytes_little(header.fo, &header.audio_format_code, 2);
    write_bytes_little(header.fo, &header.channels, 2);
    write_bytes_little(header.fo, &header.sample_rate, 4);
    write_bytes_little(header.fo, &header.bytes_per_sec, 4);
    write_bytes_little(header.fo, &header.block_align, 2);
    write_bytes_little(header.fo, &header.bits_per_sample, 2);
    if (header.has_cb_content) {
        write_bytes_little(header.fo, &header.cb_size, 2);
        write_bytes_little(header.fo, &header.valid_bits_per_sample, 2);
        write_bytes_little(header.fo, &header.channel_mask, 4);
        for (int i = 0; i < 16; i++) {
            write_bytes_little(header.fo, &header.sub_format[i], 1);
        }
    }
    // Write fact header
    if (header.has_fact_chunk) {
        write_bytes_little(header.fo, "fact", 4);
        write_bytes_little(header.fo, &header.fact_chunk_size, 4);
        write_bytes_little(header.fo, &header.fact_chunk_data, header.fact_chunk_size);
    }
    // Write data chunk
    write_bytes_little(header.fo, "data", 4);
    write_bytes_little(header.fo, &header.data_chunk_size, 4);
    puts("Please wait for writing file.");
    char c;
    while (fread(&c, 1, 1, header.fi)) {
        fwrite(&c, 1, 1, header.fo);
    }
    fclose(header.fi);
    fclose(header.fo);
    puts("Successfully convert the audio file.");
}

struct WAVHeader read_wav(FILE *f) {
    struct WAVHeader header;
    header.fi = f;
    // Read riff header
    char buf[5];
    buf[4] = '\0';
    read_bytes_little(header.fi, buf, 4);
    read_bytes_little(header.fi, &header.riff_chunk_size, 4);
    read_bytes_little(header.fi, buf, 4);
    // Read format header
    read_bytes_little(header.fi, buf, 4);
    read_bytes_little(header.fi, &header.format_chunk_size, 4);
    read_bytes_little(header.fi, &header.audio_format_code, 2);
    read_bytes_little(header.fi, &header.channels, 2);
    read_bytes_little(header.fi, &header.sample_rate, 4);
    read_bytes_little(header.fi, &header.bytes_per_sec, 4);
    read_bytes_little(header.fi, &header.block_align, 2);
    read_bytes_little(header.fi, &header.bits_per_sample, 2);
    if (header.format_chunk_size == 40) {
        read_bytes_little(header.fi, &header.cb_size, 2);
        read_bytes_little(header.fi, &header.valid_bits_per_sample, 2);
        read_bytes_little(header.fi, &header.channel_mask, 4);
        for (int i = 0; i < 16; i++) {
            read_bytes_little(header.fi, &header.sub_format[i], 1);
        }
    }
    // Read fact header
    read_bytes_little(header.fi, buf, 4);
    if (strcmp(buf, "fact") == 0) {
        read_bytes_little(header.fi, &header.fact_chunk_size, 4);
        read_bytes_little(header.fi, &header.fact_chunk_data, header.fact_chunk_size);
    }
    // Read data chunk
    if (strcmp(buf, "data") != 0) {
        read_bytes_little(header.fi, buf, 4);
    }
    read_bytes_little(header.fi, &header.data_chunk_size, 4);
    printf("Successfully read the audio file with a size %u Bytes!\n", header.riff_chunk_size + 8);
    return header;
}

uint8_t cal_crc8(uint8_t crc8, const uint8_t data) {
    int crc_poly = 0x107;
    crc8 ^= data;
    for (int8_t i = 0; i < 8; ++i) {
        if (crc8 & 0x80)
            crc8 = (crc8 << 1) ^ crc_poly;
        else
            crc8 <<= 1;
    }
    return crc8;
}

uint16_t cal_crc16(uint16_t crc16, const uint16_t data) {
    const int polynomial = 0x18005;
    crc16 ^= data << 8;
    for (uint8_t bit = 0; bit < 8; ++bit) {
        if (crc16 & 0x8000) {
            crc16 = (crc16 << 1) ^ polynomial;
        } else {
            crc16 <<= 1;
        }
    }
    return crc16;
}

void wav_to_flac() {
    FILE *f;
    char filename[100];
    while (1) {
        printf("Enter the input file path: ");
        scanf("%s", filename);
        f = fopen(filename, "rb");
        if (f == NULL) {
            puts("File does not exist, please input again!");
        } else {
            break;
        }
    }
    struct WAVHeader wav_header = read_wav(f);
    printf("Enter the output file path: ");
    scanf("%s", filename);
    wav_header.fo = fopen(filename, "wb");
    if (wav_header.channels != 2) {
        printf("Error: channels is %u which should be 2\n", wav_header.channels);
        return;
    }
    if (wav_header.bits_per_sample != 16 && wav_header.bits_per_sample != 24) {
        printf("Error: bits per sample is %u which should be 16 or 24\n", wav_header.bits_per_sample);
        return;
    }
    puts("Please wait for writing file.");
    uint32_t tmp, rem;
    write_bytes_little(wav_header.fo, "fLaC", 4);
    uint32_t block_samples = 4096;
    // Write STREAMINFO header
    tmp = 0;
    write_bytes_big(wav_header.fo, &tmp, 1);
    tmp = 34;
    write_bytes_big(wav_header.fo, &tmp, 3);
    // Write STREAMINFO body
    write_bytes_big(wav_header.fo, &block_samples, 2);
    write_bytes_big(wav_header.fo, &block_samples, 2);
    tmp = 0;
    write_bytes_big(wav_header.fo, &tmp, 3);
    write_bytes_big(wav_header.fo, &tmp, 3);
    // Write first 16 bits of the sample rate
    tmp = wav_header.sample_rate >> 4;
    rem = wav_header.sample_rate & 0xF;
    write_bytes_big(wav_header.fo, &tmp, 2);
    // Pack rem, number of channels and bits per sample to gather (4 + 3 + 5 = 12 bits)
    tmp = (rem << 8) + ((wav_header.channels - 1) << 5) + (wav_header.bits_per_sample - 1);
    // Write first 8 bits of the tmp
    rem = tmp & 0xF;
    tmp >>= 4;
    write_bytes_big(wav_header.fo, &tmp, 1);
    tmp = rem;
    // Assume that the total number of samples will not exceed 4 bytes
    tmp <<= 4;
    write_bytes_big(wav_header.fo, &tmp, 1);
    // Write the total samples in stream and tmp
    tmp = wav_header.data_chunk_size / wav_header.block_align;
    write_bytes_big(wav_header.fo, &tmp, 4);
    // Don't set MD5
    for (int i = 0; i < 16; i++) {
        tmp = 0;
        write_bytes_big(wav_header.fo, &tmp, 1);
    }
    // Write VORBIS_COMMENT
    tmp = (1 << 7) + 4;
    write_bytes_big(wav_header.fo, &tmp, 1);
    // Set comment vendor
    const char *vendor = "Transformed from wav";
    size_t vendor_length = strlen(vendor);
    // Set comment list
    const char *comment1 = "This is verbatim flac audio";
    size_t comment1_length = strlen(comment1);
    // Block size
    tmp = vendor_length + comment1_length + 12;
    write_bytes_big(wav_header.fo, &tmp, 3);
    // Write comment
    write_bytes_little(wav_header.fo, &vendor_length, 4);
    write_bytes_little(wav_header.fo, vendor, vendor_length);
    // Write comment list
    tmp = 1;
    write_bytes_little(wav_header.fo, &tmp, 4);
    write_bytes_little(wav_header.fo, &comment1_length, 4);
    write_bytes_little(wav_header.fo, comment1, comment1_length);
    // Write frames
    uint32_t frame_num = 0;
    while (1) {
        // Write sub frame
        uint32_t num = fread(chunk_buf, 1, block_samples * wav_header.block_align, wav_header.fi);
        if (num == 0) {
            break;
        }
        uint8_t crc8 = 0;
        uint16_t crc16 = 0;
        // Write frame header
        tmp = 0b11111111111110;
        rem = tmp & 0x3f;
        tmp >>= 6;
        crc8 = cal_crc8(crc8, tmp);
        crc16 = cal_crc16(crc16, tmp);
        write_bytes_big(wav_header.fo, &tmp, 1);
        tmp = rem;
        tmp = tmp << 1; // reserved
        tmp = tmp << 1; // blocking strategy
        crc8 = cal_crc8(crc8, tmp);
        crc16 = cal_crc16(crc16, tmp);
        write_bytes_big(wav_header.fo, &tmp, 1);
        // Write the block size and sample rate
        tmp = 7;
        tmp <<= 4;
        if(wav_header.sample_rate % 10 == 0){
            tmp += 14;
        }else{
            tmp += 13;
        }
        crc8 = cal_crc8(crc8, tmp);
        crc16 = cal_crc16(crc16, tmp);
        write_bytes_big(wav_header.fo, &tmp, 1);
        // Write channel assignment, sample size in bits and reserved bit
        tmp = 0b0001;
        tmp <<= 3;
        if(wav_header.bits_per_sample == 16){
            tmp += 4;
        }else{
            tmp += 6;
        }
        tmp <<= 1;
        crc8 = cal_crc8(crc8, tmp);
        crc16 = cal_crc16(crc16, tmp);
        write_bytes_big(wav_header.fo, &tmp, 1);
        // Write frame number
        tmp = 0xFC | (frame_num >> 30);
        crc8 = cal_crc8(crc8, tmp);
        crc16 = cal_crc16(crc16, tmp);
        write_bytes_big(wav_header.fo, &tmp, 1);
        for(int i=24;i>=0;i-=6){
            tmp = 0x80 | ((frame_num >> i) & 0x3F);
            crc8 = cal_crc8(crc8, tmp);
            crc16 = cal_crc16(crc16, tmp);
            write_bytes_big(wav_header.fo, &tmp, 1);
        }
        // Calculate the number of sample we derive
        uint32_t samples = num / wav_header.block_align;
        uint32_t depth = wav_header.bits_per_sample / 8;
        // Write block size
        tmp = (samples - 1) >> 8;
        crc8 = cal_crc8(crc8, tmp);
        crc16 = cal_crc16(crc16, tmp);
        write_bytes_big(wav_header.fo, &tmp, 1);
        tmp = (samples - 1) & 0xFF;
        crc8 = cal_crc8(crc8, tmp);
        crc16 = cal_crc16(crc16, tmp);
        write_bytes_big(wav_header.fo, &tmp, 1);
        // Write sample rate
        uint32_t val = wav_header.sample_rate;
        if(wav_header.sample_rate % 10 == 0){
            val /= 10;
        }
        tmp = val >> 8;
        crc8 = cal_crc8(crc8, tmp);
        crc16 = cal_crc16(crc16, tmp);
        write_bytes_big(wav_header.fo, &tmp, 1);
        tmp = val & 0xFF;
        crc8 = cal_crc8(crc8, tmp);
        crc16 = cal_crc16(crc16, tmp);
        write_bytes_big(wav_header.fo, &tmp, 1);
        // crc-8
        tmp = crc8;
        crc16 = cal_crc16(crc16, tmp);
        write_bytes_big(wav_header.fo, &tmp, 1);
        int idx = 0;
        for (int i = 0; i < samples; i++) {
            for (int j = 0; j < 2; j++) {
                sample_buf[j][i] = 0;
                for (int k = 0; k < depth; k++) {
                    sample_buf[j][i] = (sample_buf[j][i] << 8) + chunk_buf[idx++];
                }
                sample_buf[j][i] = (sample_buf[j][i] << (32 - wav_header.bits_per_sample)) >> (32 - wav_header.bits_per_sample);
            }
        }
        // Write channels
        for (int channel = 0; channel < 2; channel++) {
            tmp = 1;
            tmp <<= 1;
            crc16 = cal_crc16(crc16, tmp);
            write_bytes_big(wav_header.fo, &tmp, 1);
            for (int i = 0; i < samples; i++) {
                for (int j = 0; j < depth; j++) {
                    tmp = (sample_buf[channel][i] >> (j * 8)) & 0xFF;
                    crc16 = cal_crc16(crc16, tmp);
                    write_bytes_big(wav_header.fo, &tmp, 1);
                }
            }
        }
        // skip crc
        tmp = crc16;
        write_bytes_big(wav_header.fo, &tmp, 2);
        frame_num++;
    }
    fclose(wav_header.fi);
    fclose(wav_header.fo);
}

int main(){
    while(1){
        print_menu();
        int opt;
        scanf("%d", &opt);
        struct WAVHeader header;
        switch(opt){
            case 1:
                // Request header
                header = read_header();
                raw_to_wav(header);
                break;
            case 2:
                wav_to_flac();
                break;
            case 3:
                puts("Thanks");
                return 0;
            default:
                puts("Bad input");
        }
    }
}
