#include <iostream>
#include <cstring>
#include <fstream>
#include <iterator>
#include <algorithm>
#include <cstdio>
#include <cmath>

#include "cast_256.h"


using namespace std;

#define DATA_OFFSET_OFFSET 0x000A
#define WIDTH_OFFSET 0x0012
#define HEIGHT_OFFSET 0x0016
#define BITS_PER_PIXEL_OFFSET 0x001C
#define HEADER_SIZE 14
#define INFO_HEADER_SIZE 40
#define NO_COMPRESION 0
#define MAX_NUMBER_OF_COLORS 0
#define ALL_COLORS_REQUIRED 0

typedef unsigned int int32;
typedef short int16;
typedef unsigned char byte;

//***Inputs*****
//fileName: The name of the file to open
//***Outputs****
//pixels: A pointer to a byte array. This will contain the pixel data
//width: An int pointer to store the width of the image in pixels
//height: An int pointer to store the height of the image in pixels
//bytesPerPixel: An int pointer to store the number of bytes per pixel that are used in the image
void ReadImage(char *fileName, unsigned char **pixels, int32 *width, int32 *height, int32 *bytesPerPixel) {
    //Open the file for reading in binary mode
    FILE *imageFile = fopen(fileName, "rb");
    if (imageFile == NULL)
        exit(55);
    //Read data offset
    int32 dataOffset;
    fseek(imageFile, DATA_OFFSET_OFFSET, SEEK_SET);
    fread(&dataOffset, 4, 1, imageFile);
    //Read width
    fseek(imageFile, WIDTH_OFFSET, SEEK_SET);
    fread(width, 4, 1, imageFile);
    //Read height
    fseek(imageFile, HEIGHT_OFFSET, SEEK_SET);
    fread(height, 4, 1, imageFile);
    //Read bits per pixel
    int16 bitsPerPixel;
    fseek(imageFile, BITS_PER_PIXEL_OFFSET, SEEK_SET);
    fread(&bitsPerPixel, 2, 1, imageFile);
    //Allocate a pixel array
    *bytesPerPixel = ((int32) bitsPerPixel) / 8;

    //Rows are stored bottom-up
    //Each row is padded to be a multiple of 4 bytes.
    //We calculate the padded row size in bytes
    int paddedRowSize = (int) (4 * ceil((float) (*width) / 4.0f)) * (*bytesPerPixel);
    //We are not interested in the padded bytes, so we allocate memory just for
    //the pixel data
    int unpaddedRowSize = (*width) * (*bytesPerPixel);
    //Total size of the pixel data in bytes
    int totalSize = unpaddedRowSize * (*height);
    *pixels = (unsigned char *) malloc(totalSize);
    //Read the pixel data Row by Row.
    //Data is padded and stored bottom-up
    int i = 0;
    //point to the last row of our pixel array (unpadded)
    unsigned char *currentRowPointer = *pixels + ((*height - 1) * unpaddedRowSize);
    for (i = 0; i < *height; i++) {
        //put file cursor in the next row from top to bottom
        fseek(imageFile, dataOffset + (i * paddedRowSize), SEEK_SET);
        //read only unpaddedRowSize bytes (we can ignore the padding bytes)
        fread(currentRowPointer, 1, unpaddedRowSize, imageFile);
        //point to the next row (from bottom to top)
        currentRowPointer -= unpaddedRowSize;
    }

    fclose(imageFile);
}

//***Inputs*****
//fileName: The name of the file to save
//pixels: Pointer to the pixel data array
//width: The width of the image in pixels
//height: The height of the image in pixels
//bytesPerPixel: The number of bytes per pixel that are used in the image
void WriteImage(const char *fileName, unsigned char *pixels, int32 width, int32 height, int32 bytesPerPixel) {
    //Open file in binary mode
    FILE *outputFile = fopen(fileName, "wb");
    if (outputFile == NULL)
        exit(55);
    //*****HEADER************//
    //write signature
    const char *BM = "BM";
    fwrite(&BM[0], 1, 1, outputFile);
    fwrite(&BM[1], 1, 1, outputFile);
    //Write file size considering padded bytes
    int paddedRowSize = (int) (4 * ceil((float) width / 4.0f)) * bytesPerPixel;
    int32 fileSize = paddedRowSize * height + HEADER_SIZE + INFO_HEADER_SIZE;
    fwrite(&fileSize, 4, 1, outputFile);
    //Write reserved
    int32 reserved = 0x0000;
    fwrite(&reserved, 4, 1, outputFile);
    //Write data offset
    int32 dataOffset = HEADER_SIZE + INFO_HEADER_SIZE;
    fwrite(&dataOffset, 4, 1, outputFile);

    //*******INFO*HEADER******//
    //Write size
    int32 infoHeaderSize = INFO_HEADER_SIZE;
    fwrite(&infoHeaderSize, 4, 1, outputFile);
    //Write width and height
    fwrite(&width, 4, 1, outputFile);
    fwrite(&height, 4, 1, outputFile);
    //Write planes
    int16 planes = 1; //always 1
    fwrite(&planes, 2, 1, outputFile);
    //write bits per pixel
    int16 bitsPerPixel = bytesPerPixel * 8;
    fwrite(&bitsPerPixel, 2, 1, outputFile);
    //write compression
    int32 compression = NO_COMPRESION;
    fwrite(&compression, 4, 1, outputFile);
    //write image size (in bytes)
    int32 imageSize = width * height * bytesPerPixel;
    fwrite(&imageSize, 4, 1, outputFile);
    //write resolution (in pixels per meter)
    int32 resolutionX = 11811; //300 dpi
    int32 resolutionY = 11811; //300 dpi
    fwrite(&resolutionX, 4, 1, outputFile);
    fwrite(&resolutionY, 4, 1, outputFile);
    //write colors used
    int32 colorsUsed = MAX_NUMBER_OF_COLORS;
    fwrite(&colorsUsed, 4, 1, outputFile);
    //Write important colors
    int32 importantColors = ALL_COLORS_REQUIRED;
    fwrite(&importantColors, 4, 1, outputFile);
    //write data
    int i = 0;
    int unpaddedRowSize = width * bytesPerPixel;
    for (i = 0; i < height; i++) {
        //start writing from the beginning of last row in the pixel array
        int pixelOffset = ((height - i) - 1) * unpaddedRowSize;
        fwrite(&pixels[pixelOffset], 1, paddedRowSize, outputFile);
    }
    fclose(outputFile);
}


int main(int argc, char *argv[]) {



    char keyword[256];
    int32_t keyword_size = 0;


//    for (int i = 0; i < argc; i++) {
//        // Выводим список аргументов в цикле
//        std::cout << "Argument " << i << " : " << argv[i] << std::endl;
//    }

    if (argc < 4) {
        cout << "Invalid arguments!" << endl;
        cout << "example: -bmp -key keyword -en/-de filename.txt/.bmp" << endl;
        return 1;
    }
    else if (argc > 4) {
        if (!strcmp(argv[1], "-bmp")) {
            if (!strcmp(argv[2], "-key")) {

                keyword_size = strlen(argv[3]);
                for (int i = 0; i < keyword_size; ++i) {
                    keyword[i] = argv[3][i];
                }

                if (!strcmp(argv[4], "-en")) {
                    uint8_t *pixels;
                    int32 width;
                    int32 height;
                    int32 bytesPerPixel;
                    ReadImage(&argv[5][0], &pixels, &width, &height, &bytesPerPixel);

                    std::vector<char> buffer;
                    for (int i = 0; i < width * height * bytesPerPixel; ++i) {
                        buffer.push_back(pixels[i]);
                    }


                    cast::cast_256 cast256(keyword, keyword_size, buffer.data(), buffer.size());
                    cast256.setTextSize(buffer.size());
                    int a = cast256.encrypt_data();
                    a = 10;




                    WriteImage("out-encryption.BMP", (uint8_t*)buffer.data(), width, height, bytesPerPixel);


                } else if (!strcmp(argv[4], "-de")) {
                    uint8_t *pixels;
                    int32 width;
                    int32 height;
                    int32 bytesPerPixel;
                    ReadImage(&argv[5][0], &pixels, &width, &height, &bytesPerPixel);

                    std::vector<char> buffer;
                    for (int i = 0; i < width * height * bytesPerPixel; ++i) {
                        buffer.push_back(pixels[i]);
                    }


                    cast::cast_256 cast256(keyword, keyword_size, buffer.data(), buffer.size());
                    cast256.setTextSize(buffer.size());
                    int a = cast256.decrypt_data();
                    a = 10;

                    WriteImage("out-decryption.BMP", (uint8_t*)buffer.data(), width, height, bytesPerPixel);

                } else {
                    cout << "Invalid arguments!" << endl;
                    return 3;
                }

            }
            return 0;
        }
        //FOR TEXT
        if (!strcmp(argv[1], "-key")) {
            keyword_size = strlen(argv[2]);
            for (int i = 0; i < keyword_size; ++i) {
                keyword[i] = argv[2][i];
            }


            if (!strcmp(argv[3], "-en")) {
                std::ifstream input(argv[4], std::ios::binary);
                if (!input.is_open()) {
                    cout << "ERROR! File not found!" << endl;
                    exit(4);
                }
                std::vector<char> buffer(std::istreambuf_iterator<char>(input), {});
                input.close();
                std::ofstream ofs;
                ofs.open(argv[4], std::ofstream::out | std::ofstream::trunc);
                if (!ofs.is_open()) {
                    cout << "ERROR! File not found!" << endl;
                    exit(5);
                }
                ofs.close();

                int remainder = buffer.size() % 64;
                if (buffer.size() < 64) {
                    remainder = 64 - buffer.size();
                    for (int i = 0; i < remainder; ++i) {
                        buffer.push_back('\0');
                    }
                    remainder = 0;
                }
                if (remainder != 0) {
                    for (int i = 0; i < 64 - remainder; ++i) {
                        buffer.push_back('\0');
                    }
                }


                for (char a: buffer) {
                    cout << a << " ";
                }
                std::cout << endl;

                cast::cast_256 cast256(keyword, keyword_size, buffer.data(), buffer.size());
                cast256.setTextSize(buffer.size());
                int a = cast256.encrypt_data();

                for (char a: buffer) {
                    cout << a << " ";
                }
                std::cout << endl;

                std::ofstream output(argv[4], std::ios::binary);
                std::ostream_iterator<char> output_iterator(output);
                std::copy(buffer.begin(), buffer.end(), output_iterator);
                output.close();

            } else if (!strcmp(argv[3], "-de")) {
                std::ifstream input(argv[4], std::ios::binary);
                if (!input.is_open()) {
                    cout << "ERROR! File not found!" << endl;
                    exit(4);
                }
                std::vector<char> buffer(std::istreambuf_iterator<char>(input), {});
                input.close();
                std::ofstream ofs;
                ofs.open(argv[4], std::ofstream::out | std::ofstream::trunc);
                if (!ofs.is_open()) {
                    cout << "ERROR! File not found!" << endl;
                    exit(5);
                }
                ofs.close();

                int remainder = buffer.size() % 64;
                if (buffer.size() < 64) {
                    remainder = 64 - buffer.size();
                    for (int i = 0; i < remainder; ++i) {
                        buffer.push_back('\0');
                    }
                    remainder = 0;
                }
                if (remainder != 0) {
                    for (int i = 0; i < 64 - remainder; ++i) {
                        buffer.push_back('\0');
                    }
                }

                cast::cast_256 cast256(keyword, keyword_size, buffer.data(), buffer.size());
                cast256.setTextSize(buffer.size());
                int a = cast256.decrypt_data();
                a = 10;


                while (buffer.back() == '\0') {
                    buffer.pop_back();
                }

                std::ofstream output(argv[4], std::ios::binary);
                std::ostream_iterator<char> output_iterator(output);
                std::copy(buffer.begin(), buffer.end(), output_iterator);
                output.close();


            } else {
                cout << "Invalid arguments!" << endl;
                return 3;
            }

        } else {
            cout << "Invalid arguments!" << endl;
            return 2;
        }

        return 0;
    }

    if (!strcmp(argv[1], "-key")) {
        keyword_size = strlen(argv[2]);
        for (int i = 0; i < keyword_size; ++i) {
            keyword[i] = argv[2][i];
        }


        if (!strcmp(argv[3], "-en")) {
            std::ifstream input(argv[4], std::ios::binary);
            if (!input.is_open()) {
                cout << "ERROR! File not found!" << endl;
                exit(4);
            }
            std::vector<char> buffer(std::istreambuf_iterator<char>(input), {});
            input.close();
            std::ofstream ofs;
            ofs.open(argv[4], std::ofstream::out | std::ofstream::trunc);
            if (!ofs.is_open()) {
                cout << "ERROR! File not found!" << endl;
                exit(5);
            }
            ofs.close();

            int remainder = buffer.size() % 64;
            if (buffer.size() < 64) {
                remainder = 64 - buffer.size();
                for (int i = 0; i < remainder; ++i) {
                    buffer.push_back('\0');
                }
                remainder = 0;
            }
            if (remainder != 0) {
                for (int i = 0; i < 64 - remainder; ++i) {
                    buffer.push_back('\0');
                }
            }


            for (char a: buffer) {
                cout << a << " ";
            }
            std::cout << endl;

            cast::cast_256 cast256(keyword, keyword_size, buffer.data(), buffer.size());
            cast256.setTextSize(buffer.size());
            int a = cast256.encrypt_data();

            for (char a: buffer) {
                cout << a << " ";
            }
            std::cout << endl;

            std::ofstream output(argv[4], std::ios::binary);
            std::ostream_iterator<char> output_iterator(output);
            std::copy(buffer.begin(), buffer.end(), output_iterator);
            output.close();

        } else if (!strcmp(argv[3], "-de")) {
            std::ifstream input(argv[4], std::ios::binary);
            if (!input.is_open()) {
                cout << "ERROR! File not found!" << endl;
                exit(4);
            }
            std::vector<char> buffer(std::istreambuf_iterator<char>(input), {});
            input.close();
            std::ofstream ofs;
            ofs.open(argv[4], std::ofstream::out | std::ofstream::trunc);
            if (!ofs.is_open()) {
                cout << "ERROR! File not found!" << endl;
                exit(5);
            }
            ofs.close();

            int remainder = buffer.size() % 64;
            if (buffer.size() < 64) {
                remainder = 64 - buffer.size();
                for (int i = 0; i < remainder; ++i) {
                    buffer.push_back('\0');
                }
                remainder = 0;
            }
            if (remainder != 0) {
                for (int i = 0; i < 64 - remainder; ++i) {
                    buffer.push_back('\0');
                }
            }

            cast::cast_256 cast256(keyword, keyword_size, buffer.data(), buffer.size());
            cast256.setTextSize(buffer.size());
            int a = cast256.decrypt_data();
            a = 10;


            while (buffer.back() == '\0') {
                buffer.pop_back();
            }

            std::ofstream output(argv[4], std::ios::binary);
            std::ostream_iterator<char> output_iterator(output);
            std::copy(buffer.begin(), buffer.end(), output_iterator);
            output.close();


        } else {
            cout << "Invalid arguments!" << endl;
            return 3;
        }

    } else {
        cout << "Invalid arguments!" << endl;
        return 2;
    }



//    cast::cast_256 cast{};
//    char keyword[11] = "1231231233";
//    char text[64+64] = "aeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeedddddddddddddiiiiikkkkkkkkbaeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeedddddddddddddiiiiikkkkkkkkba";
//    cast.setKeyword(keyword, sizeof keyword);
//    cast.setText(text);
//    cast.setTextSize(64+64);
//
//    int a = cast.encrypt_data();
//
//    int b = cast.decrypt_data();


//    void *key = nullptr;
//    char *keyword;
//    keyword = new char[32];
//    key = new void*[sizeof( cast::cast256_key)];
//    for (int j = 0; j < sizeof keyword; j++) {
//        keyword[j] = 1 % 256;
//    }
//
//    cast::_mcrypt_set_key((cast::cast256_key *)(key), (uint32_t*)keyword, 32);
//    int a = static_cast<cast::cast256_key *>(key)->l_key[0];
//    a = 10;

//    void *key;
//    char *keyword;
//
//    keyword = new char[32];
//    for (int j = 0; j < sizeof keyword; j++) {
//        keyword[j] = ((j * 2 + 10) % 256);
//    }
//
//    key = new void*[sizeof( cast::cast256_key)];
//
//    cast::_mcrypt_set_key(static_cast<cast::cast256_key *>(key), (uint32_t*)keyword, 32);
//
//
//    char plaintext[64] = "edikedikedikedikedikedikedikedikedikedikedikedikedikedikedikedi";
//
//
//    cast::_mcrypt_encrypt(static_cast<cast::cast256_key *>(key), (uint32_t*)plaintext);
//
//    cast::_mcrypt_decrypt(static_cast<cast::cast256_key *>(key), (uint32_t*)plaintext);
//
//    cast::cast_256 cast(keyword, plaintext, sizeof(plaintext));
//
//    cast.encrypt_64bytes();
//
//    cast.decrypt_64bytes();

    return 0;
}
