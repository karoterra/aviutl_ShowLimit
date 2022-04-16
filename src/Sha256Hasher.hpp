#pragma once

#include <Windows.h>
#include <string>

class Sha256Hasher
{
public:
    Sha256Hasher();
    ~Sha256Hasher();

    std::string getFileHash(const char* path);

private:
    BCRYPT_ALG_HANDLE _hAlg;
    BCRYPT_HASH_HANDLE _hHash;
    PBYTE _pbHashObject;
};
