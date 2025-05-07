#include <windows.h>

#include <stdio.h>

#define Assert(expression) if (!(expression)) { *(int*)0 = 1; }

char* readEntireFile(const char* path)
{
    char* result = nullptr;
    HANDLE inputFileHandle = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (inputFileHandle != INVALID_HANDLE_VALUE)
    {
        LARGE_INTEGER fileSize;
        if (GetFileSizeEx(inputFileHandle, &fileSize))
        {
            void* buffer = VirtualAlloc(NULL, fileSize.LowPart, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
            if (buffer)
            {
                DWORD numberOfBytesRead;
                if (ReadFile(inputFileHandle, buffer, fileSize.LowPart, &numberOfBytesRead, NULL) && (numberOfBytesRead == fileSize.LowPart))
                {
                    result = (char*)buffer;
                }
                else
                {
                    VirtualFree(buffer, 0, MEM_RELEASE);
                }
            }
            else
            {
                // VirtualAlloc() failed
            }
        }
        else
        {
            // GetFileSizeEx() failed
        }
    }
    else
    {
        // CreateFileA() failed
    }
    return result;
}

int main()
{
    const char* const inputText = readEntireFile("input.txt");
    if (inputText)
    {
        int literalCharsCount = 0;
        int inMemoryCharsCount = 0;
        const char* p = inputText;
        while (*p)
        {
            switch (*p)
            {
            case '"':
                ++p;
                ++literalCharsCount;
                break;
            case '\\':
                ++p;
                switch (*p)
                {
                case 'x':
                    p += 3;
                    literalCharsCount += 4;
                    break;
                case '\\':
                case '"':
                    ++p;
                    literalCharsCount += 2;
                    break;
                default:
                    Assert(0);
                    break;
                }
                ++inMemoryCharsCount;
                break;
            default:
                ++p;
                ++literalCharsCount;
                ++inMemoryCharsCount;
                break;
            }
        }
        printf("1: %d\n", literalCharsCount - inMemoryCharsCount);

        int encodedCharsCount = 0;
        p = inputText;
        while (*p)
        {
            switch (*p)
            {
            case '"':
                ++p;
                encodedCharsCount += 1 + 2;
                break;
            case '\\':
                ++p;
                switch (*p)
                {
                case 'x':
                    p += 3;
                    encodedCharsCount += 4 + 1;
                    break;
                case '\\':
                case '"':
                    ++p;
                    encodedCharsCount += 2 + 2;
                    break;
                default:
                    Assert(0);
                    break;
                }
                break;
            default:
                ++p;
                ++encodedCharsCount;
                break;
            }
        }

        printf("2: %d\n", encodedCharsCount - literalCharsCount);
    }
    else
    {
        // readEntireFile() failed
    }
    return 0;
}
