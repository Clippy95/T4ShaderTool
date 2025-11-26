#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#ifdef _DEBUG
#   define _CRTDBG_MAP_ALLOC
#   include <stdlib.h>
#   include <crtdbg.h>
#endif

int isDefined(void const* ptr)
{
    return (*(uint32_t*)ptr != 0);
}

#include <windows.h>
int colorPrint(uint8_t color, char const* fmt, ...)
{
    int result;
    va_list arg;

    HANDLE handle = 0;
    handle = GetStdHandle(STD_OUTPUT_HANDLE);

    va_start(arg, fmt);
    if (isDefined(&handle))
    {
        SetConsoleTextAttribute(handle, color);
        result = vprintf(fmt, arg);
        SetConsoleTextAttribute(handle, 0x07);
    }
    else
        result = vprintf(fmt, arg);

    va_end(arg);
    return result;
}

enum COLORS {
    BLACK, DARK_BLUE, DARK_GREEN, DARK_CYAN, DARK_RED, DARK_MAGENTA, DARK_YELLOW,
    GRAY, DARK_GRAY, BLUE, GREEN, CYAN, RED, MAGENTA, YELLOW, WHITE
};

uint32_t generateHashValue(char const* str)
{
    unsigned int result = 0;
    while (*str)
    {
        result = (result * 33) ^ (*str);
        ++str;
    }
    return result;
}

int updateShaderSize(char const* filename)
{
    FILE* file = 0;

    file = fopen(filename, "rb");
    if (isDefined(&file))
    {
        uint8_t* buffer;
        uint32_t fsize = 0;

        fseek(file, 0, SEEK_END);
        fsize = ftell(file);
        fseek(file, 0, SEEK_SET);

        buffer = (uint8_t*)malloc(fsize * sizeof(uint8_t));
        if (isDefined(&buffer))
        {
            fread(buffer, sizeof(uint8_t), fsize, file);
            fclose(file);

            file = fopen(filename, "wb");
            if (isDefined(&file))
            {
                fwrite(&fsize, sizeof(uint32_t), 1, file);
                fwrite(buffer, sizeof(uint8_t), fsize, file);
                printf("%s, %u bytes\n", filename, fsize);
                fclose(file);
            }
            else {
                colorPrint(RED, "ERROR: Failed to open file '%s' for writing!\n", filename);
                colorPrint(RED, "Make sure you are running shader_tool.exe as administrator!\n");
            }
            free(buffer);
        }
        else {
            fclose(file);
            colorPrint(RED, "ERROR: Failed to allocate %u bytes of memory!\n", fsize);
        }
    }
    return 1;
}

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        printf("USAGE: shader_tool.exe <shader_name1> <shader_name2> <shader_name3> ...\n");
        printf("Example: shader_tool.exe z_scrolling_hud my_custom_shader another_shader\n");
        return 0;
    }

    printf("#----------------------------------------------------------------------------#\n");
    printf("|                  -- Call of Duty 5 WaW - Shader Tool v0.3 --                |\n");
    printf("|                                Made by Clippy95                             |\n");
    printf("|            Thanks to Otso O for original COD4 shader_tool                   |\n");
    printf("#----------#----------------------------#------------------------------------#\n");
    printf("| Hash     | Shader name for techniques | File name in raw\\shader_bin        |\n");
    printf("#----------#----------------------------#------------------------------------#\n");


    for (int i = 1; i < argc; i++)
    {
        char const* shader_name = argv[i];
        char vs_full[256];
        char ps_full[256];


        sprintf(vs_full, "vs_3_0_%s.hlsl", shader_name);
        sprintf(ps_full, "ps_3_0_%s.hlsl", shader_name);


        uint32_t vs_hash = generateHashValue(vs_full);
        uint32_t ps_hash = generateHashValue(ps_full);


        printf("| %.8X | vs_3_0_%-19s | vs_3_0_%.8x                 |\n",
            vs_hash, shader_name, vs_hash);


        printf("| %.8X | ps_3_0_%-19s | ps_3_0_%.8x                 |\n",
            ps_hash, shader_name, ps_hash);
    }

    printf("#----------#----------------------------#------------------------------------#\n\n");


    printf("Compiling shaders with fxc.exe ...\n");
    for (int i = 1; i < argc; i++)
    {
        char const* shader_name = argv[i];
        char vs_full[256];
        char ps_full[256];
        char cmd[512];
        char shader_output[64];

        sprintf(vs_full, "vs_3_0_%s.hlsl", shader_name);
        sprintf(ps_full, "ps_3_0_%s.hlsl", shader_name);

        uint32_t vs_hash = generateHashValue(vs_full);
        uint32_t ps_hash = generateHashValue(ps_full);


        sprintf(shader_output, "vs_3_0_%.8x", vs_hash);
        sprintf(cmd, "fxc.exe %s /T vs_3_0 /E vs_main /Fo %s > nul", vs_full, shader_output);
        system(cmd);
        updateShaderSize(shader_output);


        sprintf(shader_output, "ps_3_0_%.8x", ps_hash);
        sprintf(cmd, "fxc.exe %s /T ps_3_0 /E ps_main /Fo %s > nul", ps_full, shader_output);
        system(cmd);
        updateShaderSize(shader_output);
    }

    printf("\nCompilation complete!\n");

#ifdef _DEBUG
    system("pause");
    _CrtDumpMemoryLeaks();
#endif

    return 0;
}