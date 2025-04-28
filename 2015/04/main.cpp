#include <bit>
#include <cassert>
#include <cstdint>
#include <limits>
#include <iostream>
#include <iomanip>

union MD5
{
    std::uint32_t words[4];
    std::uint8_t bytes[16];
};

// https://en.wikipedia.org/wiki/MD5#Pseudocode
static MD5 getMD5(std::uint8_t* const message, const std::uint32_t originalMessageLength, const std::uint32_t maxMessageLength)
{
    // All variables are unsigned 32 bit and wrap modulo 2^32 when calculating

    // s specifies the per-round shift amounts
    constexpr std::uint32_t s[64]
    {
        7, 12, 17, 22,  7, 12, 17, 22,  7, 12, 17, 22,  7, 12, 17, 22,
        5,  9, 14, 20,  5,  9, 14, 20,  5,  9, 14, 20,  5,  9, 14, 20,
        4, 11, 16, 23,  4, 11, 16, 23,  4, 11, 16, 23,  4, 11, 16, 23,
        6, 10, 15, 21,  6, 10, 15, 21,  6, 10, 15, 21,  6, 10, 15, 21,
    };

    // Use binary integer part of the sines of integers (Radians) as constants:
    //for i from 0 to 63 do
    //    K[i] := floor(2^32 × abs(sin(i + 1)))
    //end for
    // (Or just use the following precomputed table):
    constexpr std::uint32_t K[64]
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
    std::uint32_t a0{ 0x67452301 };   // A
    std::uint32_t b0{ 0xefcdab89 };   // B
    std::uint32_t c0{ 0x98badcfe };   // C
    std::uint32_t d0{ 0x10325476 };   // D

    // Pre-processing: adding a single 1 bit
    //append "1" bit to message
     // Notice: the input bytes are considered as bit strings,
     //  where the first bit is the most significant bit of the byte.

    // Pre-processing: padding with zeros
    //append "0" bit until message length in bits ≡ 448 (mod 512)

    // Notice: the two padding steps above are implemented in a simpler way
      //  in implementations that only work with complete bytes: append 0x80
      //  and pad with 0x00 bytes so that the message length in bytes ≡ 56 (mod 64).

    std::uint32_t messageLength{ originalMessageLength };
    message[messageLength++] = 0x80;
    while ((messageLength % 64) != 56)
    {
        message[messageLength++] = 0x00;
    }

    // append original length in bits mod 2^64 to message
    {
        union Uint64
        {
            std::uint64_t u64;
            std::uint8_t u8[8];
        };

        Uint64 len;
        len.u64 = originalMessageLength * 8;
        for (int i{ 0 }; i < std::size(len.u8); ++i)
        {
            message[messageLength++] = len.u8[i];
        }
    }

    assert((messageLength % 64) == 0);
    assert(messageLength < maxMessageLength);

    // Process the message in successive 512-bit chunks:
    {
        struct Chunk
        {
            std::uint32_t words[16];
        };
        int chunksCount{ static_cast<int>(messageLength / sizeof(Chunk)) };
        assert(chunksCount * sizeof(Chunk) == messageLength);
        Chunk* chunks{ reinterpret_cast<Chunk*>(message) };
        //for each 512-bit chunk of padded message do
        for (int chunkIndex{ 0 }; chunkIndex < chunksCount; ++chunkIndex)
        {
            // break chunk into sixteen 32-bit words M[j], 0 ≤ j ≤ 15
            std::uint32_t* M{ &chunks[chunkIndex].words[0] };
            // Initialize hash value for this chunk:
            std::uint32_t A{ a0 };
            std::uint32_t B{ b0 };
            std::uint32_t C{ c0 };
            std::uint32_t D{ d0 };
            // Main loop:
            for (uint32_t i{ 0 }; i < 64; ++i)
            {
                std::uint32_t F;
                std::uint32_t g;
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
                std::uint32_t rotatedF{ std::rotl(F, static_cast<int>(s[i])) };
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

#if 0
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

static void testGetMD5()
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
    for (const char* str : testStrings)
    {
        std::cout << "MD5(\"" << str << "\"):\n";
        messageLength = copyString(str, message, maxMessageLength);
        result = getMD5(reinterpret_cast<std::uint8_t*>(message), static_cast<std::uint32_t>(messageLength), maxMessageLength);
        printMD5(result);
    }
}
#endif

static char* uintToChars(std::uint32_t input, char* output)
{
    if (input == 0)
    {
        *output = '0';
        ++output;
    }
    else
    {
        char reverse[16];
        int digitsCount{ 0 };
        for (; input != 0; input /= 10)
        {
            reverse[digitsCount++] = '0' + static_cast<char>(input % 10);
        }
        for (int i{ digitsCount - 1 }; i >= 0; --i)
        {
            *output = reverse[i];
            ++output;
        }
    }
    *output = 0;
    return output;
}

static std::int32_t findLowestNumberForSantaHash(const char* secretKey, std::uint32_t mask)
{
    constexpr int maxMessageLength{ 256 };
    char message[maxMessageLength];

    int secretKeyLength{ 0 };
    for ( ; secretKey[secretKeyLength] ; ++secretKeyLength)
    {
        message[secretKeyLength] = secretKey[secretKeyLength];
    }

    std::int32_t result = -1;
    for (std::int32_t number{ 0 }; number < std::numeric_limits<std::int32_t>::max(); ++number)
    {
        char* messageEnd{ uintToChars(static_cast<std::uint32_t>(number), &message[secretKeyLength]) };
        std::int64_t messageLength{ messageEnd - message };
        assert(0 < messageLength && messageLength < maxMessageLength);
        MD5 md5{ getMD5(reinterpret_cast<std::uint8_t*>(message), static_cast<std::uint32_t>(messageLength), maxMessageLength) };
        if ((md5.words[0] & mask) == 0)
        {
            result = number;
            break;
        }
    }
    return result;
}

#if 0
static void testUintToChars()
{
    char buf[128]{ "hello" };
    std::cout << buf << '\n';

    uintToChars(0, &buf[5]);
    std::cout << buf << '\n';

    uintToChars(1, &buf[5]);
    std::cout << buf << '\n';

    uintToChars(10, &buf[5]);
    std::cout << buf << '\n';

    uintToChars(123, &buf[5]);
    std::cout << buf << '\n';
}
#endif

int main()
{
#if 0
    testUintToChars();
#endif

#if 0
    testGetMD5();
#endif

    constexpr std::uint32_t mask5zeros{ 0x00f0ffff };
    constexpr std::uint32_t mask6zeros{ 0x00ffffff };

    assert(findLowestNumberForSantaHash("abcdef", mask5zeros) == 609043);
    assert(findLowestNumberForSantaHash("pqrstuv", mask5zeros) == 1048970);

    const char* input{ "ckczppom" };
    std::int32_t answer1{ findLowestNumberForSantaHash(input, mask5zeros) };
    assert(answer1 == 117946);
    std::cout << answer1 << '\n';

    std::int32_t answer2{ findLowestNumberForSantaHash(input, mask6zeros) };
    std::cout << answer2 << '\n';

    return 0;
}
