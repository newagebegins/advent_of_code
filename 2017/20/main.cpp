#include <stdint.h>
#include <stdio.h>

#define internal static
#define local_persist static

typedef int32_t s32;
typedef uint32_t u32;
typedef int32_t b32;

#define Assert(Condition) if(!(Condition)) { *(int *)0 = 0; }
#define ArrayCount(Array) (sizeof(Array)/sizeof((Array)[0]))

inline u32
AbsoluteValue(s32 X)
{
    u32 Result = (X < 0) ? -X : X;
    return(Result);
}

internal void
ReadEntireFileAndNullTerminate(char *FileName, u32 BufferSize, char *Buffer)
{
    FILE *File = fopen(FileName, "rb");
    Assert(File);
    size_t ReadCount = fread(Buffer, 1, BufferSize - 1, File);
    Assert(ReadCount < BufferSize);
    Buffer[ReadCount] = 0;
    fclose(File);
}

struct v3
{
    s32 x, y, z;
};

struct particle
{
    b32 Collided;
    v3 P, V, A;
};

struct parser
{
    char *At;
};

inline b32
IsWhitespace(char C)
{
    b32 Result = ((C == ' ') || (C == '\t') || (C == '\r') || (C == '\n'));
    return(Result);
}

inline void
SkipWhitespace(parser *Parser)
{
    while(IsWhitespace(Parser->At[0]))
    {
        ++Parser->At;
    }
}

inline void
SkipChar(parser *Parser, char C)
{
    Assert(Parser->At[0] == C);
    ++Parser->At;
}

inline void
SkipString(parser *Parser, char *String)
{
    for(char *At = String;
        At[0];
        ++At)
    {
        SkipChar(Parser, At[0]);
    }
}

inline b32
IsDigit(char C)
{
    b32 Result = ((C >= '0') && (C <= '9'));
    return(Result);
}

internal s32
ParseS32(parser *Parser)
{
    SkipWhitespace(Parser);

    s32 Result = 0;
    s32 Sign = 1;

    if(Parser->At[0] == '-')
    {
        ++Parser->At;
        Sign = -1;
    }

    Assert(IsDigit(Parser->At[0]));

    while(IsDigit(Parser->At[0]))
    {
        Result = 10*Result + (Parser->At[0] - '0');
        ++Parser->At;
    }

    Result *= Sign;
    return(Result);
}

inline v3
ParseV3(parser *Parser)
{
    v3 Result;
    SkipChar(Parser, '<');
    Result.x = ParseS32(Parser);
    SkipChar(Parser, ',');
    Result.y = ParseS32(Parser);
    SkipChar(Parser, ',');
    Result.z = ParseS32(Parser);
    SkipChar(Parser, '>');
    return(Result);
}

internal u32
ParseParticles(char *Input, u32 MaxParticleCount, particle *Particles)
{
    u32 ParticleCount = 0;

    parser Parser;
    Parser.At = Input;

    SkipWhitespace(&Parser);

    while(Parser.At[0])
    {
        Assert(ParticleCount < MaxParticleCount);
        particle *Particle = Particles + ParticleCount++;
        Particle->Collided = false;

        SkipString(&Parser, "p=");
        Particle->P = ParseV3(&Parser);
        SkipString(&Parser, ", v=");
        Particle->V = ParseV3(&Parser);
        SkipString(&Parser, ", a=");
        Particle->A = ParseV3(&Parser);

        SkipWhitespace(&Parser);
    }

    return(ParticleCount);
}

inline b32
ParticleWillBeCloserToOrigin(particle *P)
{
    b32 Result = (((P->P.x > 0) && ((P->V.x < 0) || (P->A.x < 0))) ||
                  ((P->P.x < 0) && ((P->V.x > 0) || (P->A.x > 0))) ||
                  ((P->P.y > 0) && ((P->V.y < 0) || (P->A.y < 0))) ||
                  ((P->P.y < 0) && ((P->V.y > 0) || (P->A.y > 0))) ||
                  ((P->P.z > 0) && ((P->V.z < 0) || (P->A.z < 0))) ||
                  ((P->P.z < 0) && ((P->V.z > 0) || (P->A.z > 0))));
    return(Result);
}

internal u32
FindParticleThatWillStayClosestToOriginInTheLongRun(u32 ParticleCount, particle *Particles)
{
    b32 Finished = false;

    while(!Finished)
    {
        Finished = true;

        for(u32 ParticleIndex = 0;
            ParticleIndex < ParticleCount;
            ++ParticleIndex)
        {
            particle *Particle = Particles + ParticleIndex;

            Particle->V.x += Particle->A.x;
            Particle->V.y += Particle->A.y;
            Particle->V.z += Particle->A.z;

            Particle->P.x += Particle->V.x;
            Particle->P.y += Particle->V.y;
            Particle->P.z += Particle->V.z;

            if(ParticleWillBeCloserToOrigin(Particle))
            {
                Finished = false;
            }
        }
    }

    u32 Result = ParticleCount;
    u32 MinDistanceFromOrigin = 0xFFFFFFFF;

    for(u32 ParticleIndex = 0;
        ParticleIndex < ParticleCount;
        ++ParticleIndex)
    {
        particle *Particle = Particles + ParticleIndex;
        u32 DistanceFromOrigin = (AbsoluteValue(Particle->P.x) +
                                  AbsoluteValue(Particle->P.y) +
                                  AbsoluteValue(Particle->P.z));
        if(DistanceFromOrigin < MinDistanceFromOrigin)
        {
            MinDistanceFromOrigin = DistanceFromOrigin;
            Result = ParticleIndex;
        }
    }

    return(Result);
}

inline b32
CanCollideInFuture(particle *A, particle *B)
{
    particle ARel;

    ARel.P.x = A->P.x - B->P.x;
    ARel.P.y = A->P.y - B->P.y;
    ARel.P.z = A->P.z - B->P.z;

    ARel.V.x = A->V.x - B->V.x;
    ARel.V.y = A->V.y - B->V.y;
    ARel.V.z = A->V.z - B->V.z;

    ARel.A.x = A->A.x - B->A.x;
    ARel.A.y = A->A.y - B->A.y;
    ARel.A.z = A->A.z - B->A.z;

    b32 Result = ParticleWillBeCloserToOrigin(&ARel);

    return(Result);
}

internal u32
CountParticlesAfterAllCollisionsAreResolved(u32 ParticleCount, particle *Particles)
{
    b32 Finished = false;

    while(!Finished)
    {
        Finished = true;

        for(u32 ParticleIndex1 = 0;
            ParticleIndex1 < (ParticleCount - 1);
            ++ParticleIndex1)
        {
            particle *Particle1 = Particles + ParticleIndex1;
            if(!Particle1->Collided)
            {
                for(u32 ParticleIndex2 = ParticleIndex1 + 1;
                    ParticleIndex2 < ParticleCount;
                    ++ParticleIndex2)
                {
                    particle *Particle2 = Particles + ParticleIndex2;
                    if(!Particle2->Collided)
                    {
                        if((Particle1->P.x == Particle2->P.x) &&
                           (Particle1->P.y == Particle2->P.y) &&
                           (Particle1->P.z == Particle2->P.z))
                        {
                            Particle1->Collided = true;
                            Particle2->Collided = true;
                        }
                        else if(CanCollideInFuture(Particle1, Particle2))
                        {
                            Finished = false;
                        }
                    }
                }
            }
        }

        if(!Finished)
        {
            for(u32 ParticleIndex = 0;
                ParticleIndex < ParticleCount;
                ++ParticleIndex)
            {
                particle *Particle = Particles + ParticleIndex;

                if(!Particle->Collided)
                {
                    Particle->V.x += Particle->A.x;
                    Particle->V.y += Particle->A.y;
                    Particle->V.z += Particle->A.z;

                    Particle->P.x += Particle->V.x;
                    Particle->P.y += Particle->V.y;
                    Particle->P.z += Particle->V.z;
                }
            }
        }
    }

    u32 Result = 0;

    for(u32 ParticleIndex = 0;
        ParticleIndex < ParticleCount;
        ++ParticleIndex)
    {
        particle *Particle = Particles + ParticleIndex;
        if(!Particle->Collided)
        {
            ++Result;
        }
    }

    return(Result);
}

int
main(int ArgCount, char **Args)
{
    char *InputFileName = ((ArgCount > 1) ? Args[1] : "input.txt");

    local_persist char Input[64*1024];
    ReadEntireFileAndNullTerminate(InputFileName, sizeof(Input), Input);

    local_persist particle Particles[1024];

    {
        u32 ParticleCount = ParseParticles(Input, ArrayCount(Particles), Particles);
        u32 Particle = FindParticleThatWillStayClosestToOriginInTheLongRun(ParticleCount, Particles);
        printf("%u\n", Particle);
    }

    {
        u32 ParticleCount = ParseParticles(Input, ArrayCount(Particles), Particles);
        u32 Count = CountParticlesAfterAllCollisionsAreResolved(ParticleCount, Particles);
        printf("%u\n", Count);
    }

    return(0);
}
