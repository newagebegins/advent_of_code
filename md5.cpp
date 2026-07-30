union md5
{
    u32 Words[4];
    u8 Bytes[16];
};

inline u32
RotateLeft(u32 Val, u32 Shift)
{
    u32 Result = (Val << Shift) | (Val >> (32 - Shift));
    return(Result);
}

// https://en.wikipedia.org/wiki/MD5#Pseudocode
internal md5
MD5(memory_arena *Arena, u8 *OriginalMessage, u32 OriginalMessageLength)
{
    temporary_memory TempMem = BeginTemporaryMemory(Arena);

    u32 Shifts[64] =
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

    u32 A0 = 0x67452301;
    u32 B0 = 0xefcdab89;
    u32 C0 = 0x98badcfe;
    u32 D0 = 0x10325476;

    // NOTE(slava): Pre-processing of the message:
    // 1) Add a single 1 bit.
    // 2) Pad with zeroes until the length is congruent to 56 mod 64.
    // 2) Append original length in bits mod 2^64.

    u32 MessageLength = OriginalMessageLength;

    ++MessageLength;

    s32 PadZeroCount = 56 - (MessageLength % 64);
    if(PadZeroCount < 0)
    {
        PadZeroCount += 64;
    }
    MessageLength += PadZeroCount;
    Assert(((MessageLength - 56) % 64) == 0);

    MessageLength += 8;

    Assert((MessageLength % 64) == 0);

    u8 *Message = PushArray(Arena, MessageLength, u8);
    Copy(OriginalMessageLength, OriginalMessage, Message);
    u32 At = OriginalMessageLength;
    Message[At++] = 0x80;
    for(s32 PadZeroIndex = 0;
        PadZeroIndex < PadZeroCount;
        ++PadZeroIndex)
    {
        Message[At++] = 0;
    }
    Assert((At % 8) == 0);
    *((u64 *)Message + (At / 8)) = 8*OriginalMessageLength;

    // NOTE(slava): Process the message in successive 512-bit chunks
    u32 ChunkCount = (MessageLength / 64);
    u32 *End = (u32 *)Message + ChunkCount*16;
    for(u32 *M = (u32 *)Message; M != End; M += 16)
    {
        u32 A = A0;
        u32 B = B0;
        u32 C = C0;
        u32 D = D0;
        for(u32 I = 0; I < 64; ++I)
        {
            u32 F;
            u32 G;
            if(I < 16)
            {
                F = (B & C) | ((~B) & D);
                G = I;
            }
            else if(I < 32)
            {
                F = (D & B) | ((~D) & C);
                G = (5*I + 1) % 16;
            }
            else if(I < 48)
            {
                F = B ^ C ^ D;
                G = (3*I + 5) % 16;
            }
            else
            {
                F = C ^ (B | (~D));
                G = (7*I) % 16;
            }
            F += A + K[I] + M[G];
            A = D;
            D = C;
            C = B;
            B += RotateLeft(F, Shifts[I]);
        }

        A0 += A;
        B0 += B;
        C0 += C;
        D0 += D;
    }

    EndTemporaryMemory(TempMem);

    md5 Result = { A0, B0, C0, D0 };

    return(Result);
}

internal b32
Equal(md5 A, md5 B)
{
    b32 Result = true;
    for(u32 WordIndex = 0;
        WordIndex < ArrayCount(A.Words);
        ++WordIndex)
    {
        if(A.Words[WordIndex] != B.Words[WordIndex])
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

internal md5
ToMD5(const char *Str)
{
    md5 Result = {};
    for(u32 ByteIndex = 0;
        ByteIndex < ArrayCount(Result.Bytes);
        ++ByteIndex)
    {
        char C1 = Str[2*ByteIndex];
        char C2 = Str[2*ByteIndex + 1];
        u8 B1 = CharToDigit(C1);
        u8 B2 = CharToDigit(C2);
        u8 Byte = (B1 << 4) | B2;
        Result.Bytes[ByteIndex] = Byte;
    }
    return(Result);
}

internal void
TestMD5(memory_arena *Arena)
{
    char *TestStrings[] =
    {
        "",
        "a",
        "abc",
        "The quick brown fox jumps over the lazy dog",
        "The quick brown fox jumps over the lazy dog.",
        "abcdef609043",
        "pqrstuv1048970",
        "012345678901234567890123456789012345678901234567890123456789",
    };
    char *EncodedStrings[] =
    {
        "d41d8cd98f00b204e9800998ecf8427e",
        "0cc175b9c0f1b6a831c399e269772661",
        "900150983cd24fb0d6963f7d28e17f72",
        "9e107d9d372bb6826bd81d3542a419d6",
        "e4d909c290d0fb1ca068ffaddf22cbd0",
        "000001dbbfa3a5c83a2d506429c7b00e",
        "000006136ef2ff3b291c85725f17325c",
        "1ced811af47ead374872fcca9d73dd71",
    };
    Assert(ArrayCount(TestStrings) == ArrayCount(EncodedStrings));
    for(u32 StringIndex = 0;
         StringIndex < ArrayCount(TestStrings);
         ++StringIndex)
    {
        char *Str = TestStrings[StringIndex];
        md5 Hash = MD5(Arena, (u8 *)Str, StringLength(Str));
        Assert(Equal(Hash, ToMD5(EncodedStrings[StringIndex])));
    }
}
