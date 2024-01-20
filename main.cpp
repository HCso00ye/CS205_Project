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
