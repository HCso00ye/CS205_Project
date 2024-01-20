#include <iostream>

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

void print_menu() {
    printf("\n");
    printf("Menu:\n");
    printf("\t1. Raw to WAV\n");
    printf("\t2. WAV to FLAC\n");
    printf("\t3. Exit\n");
    printf("\n");
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



int main(){
    while(1){
        print_menu();
        int opt;
        scanf("%d", &opt);
        struct WAVHeader header;
        switch(opt){
            case 1:
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
