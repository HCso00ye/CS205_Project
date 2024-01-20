#include <iostream>

using namespace std;

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
