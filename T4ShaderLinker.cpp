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
                free(buffer);
                return 1;
            }
            else {
                colorPrint(RED, "ERROR: Failed to open file '%s' for writing!\n", filename);
                colorPrint(RED, "Make sure you are running the tool as administrator!\n");
                free(buffer);
                return 0;
            }
        }
        else {
            fclose(file);
            colorPrint(RED, "ERROR: Failed to allocate %u bytes of memory!\n", fsize);
            return 0;
        }
    }
    else {
        colorPrint(RED, "ERROR: Shader compilation failed or output file '%s' not found!\n", filename);
        return 0;
    }
}

typedef struct {
    char** names;
    int count;
    int capacity;
} ShaderList;

void initShaderList(ShaderList* list)
{
    list->names = NULL;
    list->count = 0;
    list->capacity = 0;
}

void addShader(ShaderList* list, char const* name)
{
    if (list->count >= list->capacity)
    {
        list->capacity = list->capacity == 0 ? 16 : list->capacity * 2;
        list->names = (char**)realloc(list->names, list->capacity * sizeof(char*));
    }
    list->names[list->count] = _strdup(name);
    list->count++;
}

void freeShaderList(ShaderList* list)
{
    for (int i = 0; i < list->count; i++)
    {
        free(list->names[i]);
    }
    free(list->names);
}

int scanShadersInDirectory(char const* directory, ShaderList* vs_list, ShaderList* ps_list)
{
    WIN32_FIND_DATAA findData;
    HANDLE hFind;
    char search_path[512];
    int found_count = 0;

    sprintf(search_path, "%s\\*.hlsl", directory);

    hFind = FindFirstFileA(search_path, &findData);
    if (hFind == INVALID_HANDLE_VALUE)
    {
        colorPrint(YELLOW, "WARNING: Could not find shader_src directory or no .hlsl files found.\n");
        return 0;
    }

    do
    {
        if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        {
            char* filename = findData.cFileName;
            int len = strlen(filename);

            if (len > 5 && strcmp(filename + len - 5, ".hlsl") == 0)
            {
                char shader_name[256];
                strncpy(shader_name, filename, len - 5);
                shader_name[len - 5] = '\0';

                if (strncmp(shader_name, "vs_3_0_", 7) == 0)
                {
                    addShader(vs_list, shader_name + 7);
                    found_count++;
                }
                else if (strncmp(shader_name, "ps_3_0_", 7) == 0)
                {
                    addShader(ps_list, shader_name + 7);
                    found_count++;
                }
            }
        }
    } while (FindNextFileA(hFind, &findData));

    FindClose(hFind);
    return found_count;
}

int main(int argc, char** argv)
{
    char* exe_name = argv[0];
    int use_current_dir = 0;
    int arg_start = 1;
    char const* source_dir = "shader_src";

    char* last_slash = strrchr(exe_name, '\\');
    if (!last_slash) last_slash = strrchr(exe_name, '/');
    if (last_slash) exe_name = last_slash + 1;


    if (argc >= 2 && (strcmp(argv[1], "-cd") == 0 || strcmp(argv[1], "/cd") == 0))
    {
        use_current_dir = 1;
        arg_start = 2;
        source_dir = ".";
    }

    printf("#----------------------------------------------------------------------------#\n");
    printf("|                  -- Call of Duty 5 WaW - Shader Tool v0.4 --                |\n");
    printf("|                                Made by Clippy95                             |\n");
    printf("|            Thanks to Otso O for original COD4 shader_tool                   |\n");
    printf("#----------#----------------------------#------------------------------------#\n");

    ShaderList vs_list, ps_list;
    initShaderList(&vs_list);
    initShaderList(&ps_list);

    char** shader_names = NULL;
    int shader_count = 0;


    if (argc <= arg_start)
    {
        printf("Scanning for shaders in '%s' directory...\n\n", source_dir);

        int found = scanShadersInDirectory(source_dir, &vs_list, &ps_list);

        if (found == 0)
        {
            printf("USAGE: %s [-cd] <shader_name1> <shader_name2> <shader_name3> ...\n", exe_name);
            printf("       %s           - Compile all shaders in shader_src\\ folder\n", exe_name);
            printf("       %s -cd       - Compile all shaders in current directory\n", exe_name);
            printf("Example: %s z_scrolling_hud my_custom_shader another_shader\n", exe_name);
            freeShaderList(&vs_list);
            freeShaderList(&ps_list);
            return 0;
        }


        ShaderList combined;
        initShaderList(&combined);

        for (int i = 0; i < vs_list.count; i++)
        {
            int found_dup = 0;
            for (int j = 0; j < combined.count; j++)
            {
                if (strcmp(vs_list.names[i], combined.names[j]) == 0)
                {
                    found_dup = 1;
                    break;
                }
            }
            if (!found_dup)
                addShader(&combined, vs_list.names[i]);
        }

        for (int i = 0; i < ps_list.count; i++)
        {
            int found_dup = 0;
            for (int j = 0; j < combined.count; j++)
            {
                if (strcmp(ps_list.names[i], combined.names[j]) == 0)
                {
                    found_dup = 1;
                    break;
                }
            }
            if (!found_dup)
                addShader(&combined, ps_list.names[i]);
        }

        shader_names = combined.names;
        shader_count = combined.count;

        printf("Found %d shader(s) to compile.\n\n", shader_count);
    }
    else
    {

        shader_count = argc - arg_start;
        shader_names = argv + arg_start;
    }

    printf("| Hash     | Shader name for techniques | File name in raw\\shader_bin        |\n");
    printf("#----------#----------------------------#------------------------------------#\n");

    for (int i = 0; i < shader_count; i++)
    {
        char const* shader_name = shader_names[i];
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
    int success_count = 0;
    int total_shaders = shader_count * 2;  // VS and PS for each shader

    for (int i = 0; i < shader_count; i++)
    {
        char const* shader_name = shader_names[i];
        char vs_full[512];
        char ps_full[512];
        char cmd[1024];
        char shader_output[64];

        if (use_current_dir)
        {
            sprintf(vs_full, "vs_3_0_%s.hlsl", shader_name);
            sprintf(ps_full, "ps_3_0_%s.hlsl", shader_name);
        }
        else
        {
            sprintf(vs_full, "%s\\vs_3_0_%s.hlsl", source_dir, shader_name);
            sprintf(ps_full, "%s\\ps_3_0_%s.hlsl", source_dir, shader_name);
        }

        uint32_t vs_hash = generateHashValue(use_current_dir ? vs_full : vs_full + strlen(source_dir) + 1);
        uint32_t ps_hash = generateHashValue(use_current_dir ? ps_full : ps_full + strlen(source_dir) + 1);

        // Check if VS file exists
        FILE* test_file = fopen(vs_full, "r");
        if (test_file)
        {
            fclose(test_file);
            sprintf(shader_output, "vs_3_0_%.8x", vs_hash);
            sprintf(cmd, "fxc.exe \"%s\" /T vs_3_0 /E vs_main /Fo %s", vs_full, shader_output);
            int vs_result = system(cmd);
            if (vs_result == 0 && updateShaderSize(shader_output))
                success_count++;
        }
        else
        {
            colorPrint(YELLOW, "WARNING: File not found: %s\n", vs_full);
        }


        test_file = fopen(ps_full, "r");
        if (test_file)
        {
            fclose(test_file);
            sprintf(shader_output, "ps_3_0_%.8x", ps_hash);
            sprintf(cmd, "fxc.exe \"%s\" /T ps_3_0 /E ps_main /Fo %s", ps_full, shader_output);
            int ps_result = system(cmd);
            if (ps_result == 0 && updateShaderSize(shader_output))
                success_count++;
        }
        else
        {
            colorPrint(YELLOW, "WARNING: File not found: %s\n", ps_full);
        }
    }

    printf("\n");
    if (success_count == total_shaders)
    {
        colorPrint(GREEN, "Compilation complete! All %d shaders compiled successfully.\n", success_count);
    }
    else if (success_count > 0)
    {
        colorPrint(YELLOW, "Compilation finished with warnings. %d/%d shaders compiled successfully.\n", success_count, total_shaders);
    }
    else
    {
        colorPrint(RED, "Compilation failed! No shaders were compiled successfully.\n");
        colorPrint(RED, "Make sure fxc.exe is in your PATH and shader files exist.\n");
    }

    // Cleanup if we allocated shader list
    if (argc <= arg_start)
    {
        freeShaderList(&vs_list);
        freeShaderList(&ps_list);
        // combined list was already freed by setting shader_names to it
    }

#ifdef _DEBUG
    system("pause");
    _CrtDumpMemoryLeaks();
#endif

    return 0;
}