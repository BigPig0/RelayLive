#include "stdafx.h"
#include"md5.h"

#include<iostream>
using namespace std;

const int S[4][4] = {7, 12, 17, 22,
    5, 9, 14, 20,
    4, 11, 16, 23,
    6, 10, 15, 21};
void MD5::init()
{
    A = 0x67452301;
    B = 0xefcdab89;
    C = 0x98badcfe;
    D = 0x10325476;
    remainNum_ = 0;
    remain_[0] = '\0';
    md5Result_hex_[0] = '\0';
    md5Result_[0] = '\0';
    totalInputBits_ = 0;
    isDone_ = false;
}

MD5::MD5()
{
    init();
}

inline MD5::uint32 MD5::RotateLeft(const uint32 x, int n)
{
    return (x << n) | (x >> (32-n));        
    // if x is signed, use: (x << n) | ((x & 0xFFFFFFFF) >> (32-n))
}
inline MD5::uint32 MD5::F(const uint32 x, const uint32 y, const uint32 z)
{
    return (x & y) | ((~x) & z);
}
inline MD5::uint32 MD5::G(const uint32 x, const uint32 y, const uint32 z)
{
    return (x & z) | (y & (~z));
}
inline MD5::uint32 MD5::H(const uint32 x, const uint32 y, const uint32 z)
{
    return x ^ y ^ z;
}
inline MD5::uint32 MD5::I(const uint32 x, const uint32 y, const uint32 z)
{
    return y ^ (x | (~z));
}

inline void MD5::FF(uint32 &a, const uint32 b, const uint32 c, const uint32 d,
                    const uint32 Mj, const int s, const uint32 ti)
{
    a = b + RotateLeft(a + F(b, c, d) + Mj + ti, s);
}      
inline void MD5::GG(uint32 &a, const uint32 b, const uint32 c, const uint32 d,
                    const uint32 Mj, const int s, const uint32 ti)
{
    a = b + RotateLeft(a + G(b, c, d) + Mj + ti, s);
}                   
inline void MD5::HH(uint32 &a, const uint32 b, const uint32 c, const uint32 d,
                    const uint32 Mj, const int s, const uint32 ti)
{
    a = b + RotateLeft(a + H(b, c, d) + Mj + ti, s);
}                    
inline void MD5::II(uint32 &a, const uint32 b, const uint32 c, const uint32 d,
                    const uint32 Mj, const int s, const uint32 ti)
{
    a = b + RotateLeft(a + I(b, c, d) + Mj + ti, s);
}          

// link A B C D to result(bit style result and hexadecimal style result)
void MD5::LinkResult()
{
    //bit style result
    for(int i = 0; i < 4; i++)  //link A: low to high
    {
        md5Result_[i] = (A >> 8*i) & 0xff;
    }
    for(int i = 4; i<8; i++)   //link B: low to high
    {
        md5Result_[i] = (B >> 8*(i - 4)) & 0xff;
    }
    for(int i = 8; i<12; i++)  //link C: low to high
    {
        md5Result_[i] = (C >> 8*(i - 8)) & 0xff;
    }
    for(int i = 12; i<16; i++)  //link D: low to high
    {
        md5Result_[i] = (D >> 8*(i - 12)) & 0xff;
    }

    //change to hexadecimal style result
    // note: it is not the same as simply link hex(A) hex(B) hex(C) hex(D)
    for(int i = 0; i < 16; i++)
        sprintf(& md5Result_hex_[i*2], "%02x", md5Result_[i]);
    md5Result_hex_[32] = '\0';

}

//print the md5 by hex
void MD5::printMd5()
{
    if(!isDone_)
    {
        cout<< "Error: computation is not finished" <<endl;

    }
    else
        cout<< "MD5 Value: " << md5Result_hex_ <<endl;
}

//get the md5 value of hex style 
string MD5::GetMd5()
{
    if(!isDone_)
    {
        cout<< "Error: computation is not finished" <<endl;
        exit(0);
    }
    string a((const char *)md5Result_hex_);
    return a;
}

void MD5::UcharToUint(uint32 output[], const uchar8 input[], const unsigned int transLength)         
{
    for(int i = 0, j = 0; j < transLength; i++, j += 4)
    {
        output[i] = ((uint32)input[j]) | (((uint32)input[j+1]) << 8) |
            (((uint32)input[j+2]) << 16) | (((uint32)input[j+3]) << 24);
    }
}

// four round on a block of 512 bits;
void MD5::FourRound(const uchar8 block[])
{
    uint32 a = A, b = B, c = C, d = D;
    uint32 M[16];
    UcharToUint(M, block, blockLen_); //blockLen_ is a const int =64;

    //round 1
    FF (a, b, c, d, M[ 0], S[0][0], 0xd76aa478); 
    FF (d, a, b, c, M[ 1], S[0][1], 0xe8c7b756); 
    FF (c, d, a, b, M[ 2], S[0][2], 0x242070db); 
    FF (b, c, d, a, M[ 3], S[0][3], 0xc1bdceee); 
    FF (a, b, c, d, M[ 4], S[0][0], 0xf57c0faf); 
    FF (d, a, b, c, M[ 5], S[0][1], 0x4787c62a); 
    FF (c, d, a, b, M[ 6], S[0][2], 0xa8304613);
    FF (b, c, d, a, M[ 7], S[0][3], 0xfd469501);
    FF (a, b, c, d, M[ 8], S[0][0], 0x698098d8); 
    FF (d, a, b, c, M[ 9], S[0][1], 0x8b44f7af);
    FF (c, d, a, b, M[10], S[0][2], 0xffff5bb1); 
    FF (b, c, d, a, M[11], S[0][3], 0x895cd7be); 
    FF (a, b, c, d, M[12], S[0][0], 0x6b901122);
    FF (d, a, b, c, M[13], S[0][1], 0xfd987193);
    FF (c, d, a, b, M[14], S[0][2], 0xa679438e);
    FF (b, c, d, a, M[15], S[0][3], 0x49b40821); 

    // round 2 
    GG (a, b, c, d, M[ 1], S[1][0], 0xf61e2562); 
    GG (d, a, b, c, M[ 6], S[1][1], 0xc040b340); 
    GG (c, d, a, b, M[11], S[1][2], 0x265e5a51); 
    GG (b, c, d, a, M[ 0], S[1][3], 0xe9b6c7aa);
    GG (a, b, c, d, M[ 5], S[1][0], 0xd62f105d); 
    GG (d, a, b, c, M[10], S[1][1],  0x2441453); 
    GG (c, d, a, b, M[15], S[1][2], 0xd8a1e681);
    GG (b, c, d, a, M[ 4], S[1][3], 0xe7d3fbc8); 
    GG (a, b, c, d, M[ 9], S[1][0], 0x21e1cde6);
    GG (d, a, b, c, M[14], S[1][1], 0xc33707d6);
    GG (c, d, a, b, M[ 3], S[1][2], 0xf4d50d87);
    GG (b, c, d, a, M[ 8], S[1][3], 0x455a14ed); 
    GG (a, b, c, d, M[13], S[1][0], 0xa9e3e905); 
    GG (d, a, b, c, M[ 2], S[1][1], 0xfcefa3f8); 
    GG (c, d, a, b, M[ 7], S[1][2], 0x676f02d9);
    GG (b, c, d, a, M[12], S[1][3], 0x8d2a4c8a);

    //round 3 
    HH (a, b, c, d, M[ 5], S[2][0], 0xfffa3942); 
    HH (d, a, b, c, M[ 8], S[2][1], 0x8771f681); 
    HH (c, d, a, b, M[11], S[2][2], 0x6d9d6122); 
    HH (b, c, d, a, M[14], S[2][3], 0xfde5380c);
    HH (a, b, c, d, M[ 1], S[2][0], 0xa4beea44); 
    HH (d, a, b, c, M[ 4], S[2][1], 0x4bdecfa9);
    HH (c, d, a, b, M[ 7], S[2][2], 0xf6bb4b60); 
    HH (b, c, d, a, M[10], S[2][3], 0xbebfbc70); 
    HH (a, b, c, d, M[13], S[2][0], 0x289b7ec6); 
    HH (d, a, b, c, M[ 0], S[2][1], 0xeaa127fa); 
    HH (c, d, a, b, M[ 3], S[2][2], 0xd4ef3085);
    HH (b, c, d, a, M[ 6], S[2][3],  0x4881d05); 
    HH (a, b, c, d, M[ 9], S[2][0], 0xd9d4d039); 
    HH (d, a, b, c, M[12], S[2][1], 0xe6db99e5); 
    HH (c, d, a, b, M[15], S[2][2], 0x1fa27cf8); 
    HH (b, c, d, a, M[ 2], S[2][3], 0xc4ac5665); 

    //round 4 
    II (a, b, c, d, M[ 0], S[3][0], 0xf4292244); 
    II (d, a, b, c, M[ 7], S[3][1], 0x432aff97); 
    II (c, d, a, b, M[14], S[3][2], 0xab9423a7); 
    II (b, c, d, a, M[ 5], S[3][3], 0xfc93a039);
    II (a, b, c, d, M[12], S[3][0], 0x655b59c3); 
    II (d, a, b, c, M[ 3], S[3][1], 0x8f0ccc92); 
    II (c, d, a, b, M[10], S[3][2], 0xffeff47d); 
    II (b, c, d, a, M[ 1], S[3][3], 0x85845dd1); 
    II (a, b, c, d, M[ 8], S[3][0], 0x6fa87e4f); 
    II (d, a, b, c, M[15], S[3][1], 0xfe2ce6e0); 
    II (c, d, a, b, M[ 6], S[3][2], 0xa3014314); 
    II (b, c, d, a, M[13], S[3][3], 0x4e0811a1); 
    II (a, b, c, d, M[ 4], S[3][0], 0xf7537e82); 
    II (d, a, b, c, M[11], S[3][1], 0xbd3af235); 
    II (c, d, a, b, M[ 2], S[3][2], 0x2ad7d2bb);
    II (b, c, d, a, M[ 9], S[3][3], 0xeb86d391); 

    A += a;
    B += b;
    C += c;
    D += d;
}

// update md5,must consider the remain_.
void MD5::UpdateMd5(const uchar8 input[], const int length)
{    
    isDone_ = false;
    totalInputBits_ += (length << 3);

    int start = blockLen_ - remainNum_; //blockLen_ = 64
    //copy a part of input to remain_ so it can form a block(size=64)

    if(start <= length)
    {
        // can form a block,then do FourRound to this block
        memcpy(&remain_[remainNum_], input, start) ;
        FourRound(remain_);

        int i;
        for(i = start; i <= length - blockLen_; i += blockLen_)
        {
            FourRound(&input[i]);
        }
        remainNum_ = length - i;  
        memcpy(remain_, &input[i], remainNum_);  
    }     
    else
    {
        // can not form a block, function return;
        memcpy(&remain_[remainNum_], input, length);
        remainNum_ += length;
    }    

}

void MD5::UpdateMd5(const char8 input[], const int length)
{
    UpdateMd5((const uchar8 *)input, length);
}

// padding with 100000... to remain_ and add the 64bit original size of input 
void MD5::Finalize()
{
    if(isDone_ == true)
        return;

    uchar8 padding[64] = {
        0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };

    int temp = 56 - remainNum_;  //56 = 448/8
    if(temp > 0)
    {
        UpdateMd5(padding, temp);
        totalInputBits_ -= (temp << 3);
    }
    else if(temp < 0)
    {
        UpdateMd5(padding, 64 + temp);
        totalInputBits_ -= ((64 + temp) << 3);
    }

    // trans totalInputBits_ to uchar8 (64bits)
    uchar8 Bits[8];
    for(int i = 0; i < 8; i++)
    {
        Bits[i] = (totalInputBits_ >> 8*i) & 0xff;
    }

    UpdateMd5(Bits, 8); // add the number of  original input (the last 64bits)

    LinkResult();
    isDone_ = true;
}

// comput the md5 based on input, (just this one input)
void MD5::ComputMd5(const uchar8 input[], const int length)
{
    init();
    UpdateMd5(input, length);
    Finalize();
}

void MD5::ComputMd5(const char8 input[], const int length)
{
    ComputMd5((const uchar8 *)input, length);
}