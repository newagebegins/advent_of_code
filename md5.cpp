#include <bit>
#include <cassert>
#include <cstdint>
#include <limits>
#include <iostream>
#include <iomanip>

// https://en.wikipedia.org/wiki/MD5#Pseudocode
static MD5 getMD5(u8* message, u32 originalMessageLength, u32 maxMessageLength)
{
    // All variables are unsigned 32 bit and wrap modulo 2^32 when calculating

    // s specifies the per-round shift amounts
    u32 s[64] =
    {
        7, 12, 17, 22,  7, 12, 17, 22,  7, 12, 17, 22,  7, 12, 17, 22,
        5,  9, 14, 20,  5,  9, 14, 20,  5,  9, 14, 20,  5,  9, 14, 20,
        4, 11, 16, 23,  4, 11, 16, 23,  4, 11, 16, 23,  4, 11, 16, 23,
        6, 10, 15, 21,  6, 10, 15, 21,  6, 10, 15, 21,  6, 10, 15, 21,
    };

    u32 K[64] =
    {
        0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
        0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
        0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
        0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
        0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
        0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
        0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
        0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
        0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
        0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
        0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
        0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
        0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
        0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
        0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
        0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391,
    };

    // Initialize variables:
    u32 a0 = 0x67452301;   // A
    u32 b0 = 0xefcdab89;   // B
    u32 c0 = 0x98badcfe;   // C
    u32 d0 = 0x10325476;   // D

    // Pre-processing: adding a single 1 bit
    //append "1" bit to message
     // Notice: the input bytes are considered as bit strings,
     //  where the first bit is the most significant bit of the byte.

    // Pre-processing: padding with zeros
    //append "0" bit until message length in bits ≡ 448 (mod 512)

    // Notice: the two padding steps above are implemented in a simpler way
      //  in implementations that only work with complete bytes: append 0x80
      //  and pad with 0x00 bytes so that the message length in bytes ≡ 56 (mod 64).

    u32 messageLength = originalMessageLength;
    message[messageLength++] = 0x80;
    while ((messageLength % 64) != 56)
    {
        message[messageLength++] = 0x00;
    }

    // append original length in bits mod 2^64 to message
    {
        union Uint64
        {
            u64 u64;
            u8 u8[8];
        };

        Uint64 len;
        len.u64 = originalMessageLength * 8;
        for (int i{ 0 }; i < ArrayCount(len.u8); ++i)
        {
            message[messageLength++] = len.u8[i];
        }
    }

    Assert((messageLength % 64) == 0);
    Assert(messageLength < maxMessageLength);

    // Process the message in successive 512-bit chunks:
    {
        struct Chunk
        {
            u32 words[16];
        };
        u32 chunksCount = (messageLength / sizeof(Chunk));
        Assert(chunksCount * sizeof(Chunk) == messageLength);
        Chunk* chunks = (Chunk *)message;
        //for each 512-bit chunk of padded message do
        for (u32 chunkIndex = 0; chunkIndex < chunksCount; ++chunkIndex)
        {
            // break chunk into sixteen 32-bit words M[j], 0 ≤ j ≤ 15
            u32* M = &chunks[chunkIndex].words[0];
            // Initialize hash value for this chunk:
            u32 A = a0;
            u32 B = b0;
            u32 C = c0;
            u32 D = d0;
            // Main loop:
            for (u32 i = 0; i < 64; ++i)
            {
                u32 F;
                u32 g;
                if (i < 16)
                {
                    F = (B & C) | ((~B) & D);
                    g = i;
                }
                else if (i < 32)
                {
                    F = (D & B) | ((~D) & C);
                    g = (5*i + 1) % 16;
                }
                else if (i < 48)
                {
                    F = B ^ C ^ D;
                    g = (3*i + 5) % 16;
                }
                else
                {
                    F = C ^ (B | (~D));
                    g = (7*i) % 16;
                }
                // Be wary of the below definitions of a,b,c,d
                F += A + K[i] + M[g];  // M[g] must be a 32-bit block
                A = D;
                D = C;
                C = B;
                u32 rotatedF = std::rotl(F, static_cast<int>(s[i]));
                B += rotatedF;
            }
            // Add this chunk's hash to result so far:
            a0 += A;
            b0 += B;
            c0 += C;
            d0 += D;
        }
    }

    //var char digest[16] := a0 append b0 append c0 append d0 // (Output is in little-endian)
    return { a0, b0, c0, d0 };
}

static int copyString(const char* input, char* output, int maxLength)
{
    int length{ 0 };
    for (; input[length]; ++length)
    {
        output[length] = input[length];
    }
    assert(length < maxLength);
    return length;
}

static void printMD5(MD5 md5)
{
    for (int i{ 0 }; i < std::size(md5.bytes); ++i)
    {
        std::cout << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(md5.bytes[i]);
    }
    std::cout << '\n';
}

static b32
Equal(MD5 A, MD5 B)
{
    b32 Result = true;
    for(u32 WordIndex = 0;
        WordIndex < ArrayCount(A.words);
        ++WordIndex)
    {
        if(A.words[WordIndex] != B.words[WordIndex])
        {
            Result = false;
            break;
        }
    }
    return(Result);
}

inline u8
CharToDigit(char C)
{
    u8 Result = (C > '9') ? ((C - 'a') + 0xA) : (C - '0');
    return(Result);
}

static MD5
ToMD5(const char *Str)
{
    MD5 Result;
    for(u32 ByteIndex = 0;
        ByteIndex < ArrayCount(Result.bytes);
        ++ByteIndex)
    {
        char C1 = Str[2*ByteIndex];
        char C2 = Str[2*ByteIndex + 1];
        u8 B1 = CharToDigit(C1);
        u8 B2 = CharToDigit(C2);
        u8 Byte = (B1 << 4) | B2;
        Result.bytes[ByteIndex] = Byte;
    }
    return(Result);
}

static void
TestMD5()
{
    constexpr int maxMessageLength{ 1024 };
    char message[maxMessageLength];

    MD5 result;
    int messageLength;

    const char* testStrings[]
    {
        "",
        "a",
        "abc",
        "The quick brown fox jumps over the lazy dog",
        "The quick brown fox jumps over the lazy dog.",
        "abcdef609043",
        "pqrstuv1048970",
    };
    const char* encodedStrings[] =
    {
        "d41d8cd98f00b204e9800998ecf8427e",
        "0cc175b9c0f1b6a831c399e269772661",
        "900150983cd24fb0d6963f7d28e17f72",
        "9e107d9d372bb6826bd81d3542a419d6",
        "e4d909c290d0fb1ca068ffaddf22cbd0",
        "000001dbbfa3a5c83a2d506429c7b00e",
        "000006136ef2ff3b291c85725f17325c",
    };
    for (u32 StringIndex = 0;
         StringIndex < ArrayCount(testStrings);
         ++StringIndex)
    {
        const char *str = testStrings[StringIndex];
        std::cout << "MD5(\"" << str << "\"):\n";
        messageLength = copyString(str, message, maxMessageLength);
        result = getMD5(reinterpret_cast<std::uint8_t*>(message), static_cast<u32>(messageLength), maxMessageLength);
        printMD5(result);
        Assert(Equal(result, ToMD5(encodedStrings[StringIndex])));
    }
}

