//
// Created by root on 12.12.22.
//

#ifndef LAB2_CRIPT_CPP_CAST_256_H
#define LAB2_CRIPT_CPP_CAST_256_H

#include <stdint-gcc.h>
#include <vector>
#include "stdint.h"



namespace cast {
    typedef struct cast256_instance {
        uint32_t l_key[256];
    } cast256_key;

    class cast_256 {

    private:

        char* keyword;
        int32_t keyword_size = 32;
        char* text;
        uint64_t text_size;
        void *key;

    public:

        cast_256();

        int generate_key();

        int encrypt_data();

        int decrypt_data();

        int encrypt_64bytes(char* text);

        int decrypt_64bytes(char* text);

        char *getKeyword() const;

        const int32_t getKeywordSize() const;

        char *getText() const;

        uint64_t getTextSize() const;

        void *getKey() const;

        void setKeyword(char *keyword, int32_t keyword_size);

        void setText(char *text);

        void setTextSize(uint64_t textSize);

        virtual ~cast_256();

        cast_256(char* keyword, int32_t keyword_size, char* text, uint64_t text_size);

        cast_256(char *keyword, const int32_t &keywordSize, std::vector<char> &buffer);
    };

    //Планирование ключей для раундов по ключевому слову.
    int _mcrypt_set_key(cast256_key *key, const uint32_t *in_key,
                               const int key_len);

    void _mcrypt_encrypt(cast256_key *key, uint32_t *blk);
    void _mcrypt_decrypt(cast256_key *key, uint32_t *blk);


} // cast

#endif //LAB2_CRIPT_CPP_CAST_256_H
