#include "Sha256Hasher.hpp"
#include <stdexcept>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>

using namespace std;

Sha256Hasher::Sha256Hasher() :
    _hAlg(nullptr),
    _hHash(nullptr),
    _pbHashObject(nullptr)
{
    DWORD cbData = 0, cbHashObject = 0;
    NTSTATUS status = BCryptOpenAlgorithmProvider(&_hAlg, BCRYPT_SHA256_ALGORITHM, nullptr, BCRYPT_HASH_REUSABLE_FLAG);
    if (status < 0) {
        throw runtime_error("fail to BCryptOpenAgorithmProvider (" + to_string(status) + ")");
    }

    status = BCryptGetProperty(_hAlg, BCRYPT_OBJECT_LENGTH, (PBYTE)&cbHashObject, sizeof(DWORD), &cbData, 0);
    if (status < 0) {
        BCryptCloseAlgorithmProvider(_hAlg, 0);
        throw runtime_error("fail to BCryptGetProperty (" + to_string(status) + ")");
    }

    _pbHashObject = (PBYTE)HeapAlloc(GetProcessHeap(), 0, cbHashObject);
    if (_pbHashObject == nullptr) {
        BCryptCloseAlgorithmProvider(_hAlg, 0);
        throw runtime_error("fail to HeapAlloc");
    }

    status = BCryptCreateHash(_hAlg, &_hHash, _pbHashObject, cbHashObject, nullptr, 0, BCRYPT_HASH_REUSABLE_FLAG);
    if (status < 0) {
        BCryptCloseAlgorithmProvider(_hAlg, 0);
        HeapFree(GetProcessHeap(), 0, _pbHashObject);
        throw runtime_error("fail to BCryptCreateHash (" + to_string(status) + ")");
    }
}

Sha256Hasher::~Sha256Hasher()
{
    if (_hAlg) {
        BCryptCloseAlgorithmProvider(_hAlg, 0);
    }
    if (_hHash) {
        BCryptDestroyHash(_hHash);
    }
    if (_pbHashObject) {
        HeapFree(GetProcessHeap(), 0, _pbHashObject);
    }
}

std::string Sha256Hasher::getFileHash(const std::filesystem::path& path)
{
    ifstream fp(path, ios::in | ios::binary);
    if (!fp.good()) {
        ostringstream os;
        os << "cannot open \"" << path << "\", " << errno << ".";
        throw runtime_error(os.str());
    }

    constexpr size_t bufSize = 1 << 12;
    char buf[bufSize];

    while (fp.good()) {
        fp.read(buf, bufSize);
        NTSTATUS status = BCryptHashData(_hHash, (PBYTE)buf, fp.gcount(), 0);
        if (status < 0) {
            throw runtime_error("fail to BCryptHashData (" + to_string(status) + ")");
        }
    }

    constexpr size_t digestSize = 32;
    BYTE digest[digestSize];
    NTSTATUS status = BCryptFinishHash(_hHash, digest, digestSize, 0);
    if (status < 0) {
        throw runtime_error("fail to BCryptFinishHash (" + to_string(status) + ")");
    }

    ostringstream os;
    os << hex << uppercase;
    for (int i = 0; i < digestSize; i++) {
        os << setw(2) << setfill('0') << (int)digest[i];
    }
    return os.str();
}
