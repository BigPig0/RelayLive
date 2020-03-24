#pragma once

#include "ssl_export.h"
#include <stdio.h>
#include <stdint.h>
 
class _SSL_API SHA1
{
public:
    SHA1();
    ~SHA1();

    void Init();

    void Update(const uint8_t* in, const uint32_t len);

    void Finalize();

    uint8_t* Result(uint8_t* out = NULL);

    uint8_t* Comput(const uint8_t* in, const uint32_t len, uint8_t* out = NULL);

    uint8_t* File(const uint8_t* path, uint8_t* out = NULL);

private:
    void* handle;
};