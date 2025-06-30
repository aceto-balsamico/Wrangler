#include <stdio.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
double get_time_sec()
{
    static LARGE_INTEGER freq;
    static int initialized = 0;
    LARGE_INTEGER now;
    if (!initialized) {
        QueryPerformanceFrequency(&freq);
        initialized = 1;
    }
    QueryPerformanceCounter(&now);
    return (double)now.QuadPart / (double)freq.QuadPart;
}
#else
#include <time.h>
double get_time_sec()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}
#endif


int custom_strlen(char* str)
{
    if(str == NULL)
    {
        return -1;
    }
    int char_index = 0;

    while(str[char_index] != '\0')
    {
        char_index++;
    }
    return char_index;
}

int custom_strcpy(char* dst, char* src)
{
    if(dst == NULL || src == NULL)
    {
        return -1;
    }
    int char_index = 0;

    while(src[char_index] != '\0')
    {
        dst[char_index] = src[char_index];
        char_index++;
    }
    dst[char_index] = '\0';
    return char_index;
}

void custom_strcat(char* dst, char* src)
{
    if(dst == NULL || src == NULL)
    {
        return;
    }
    int dst_length = 0;
    dst_length = custom_strlen(dst);
    custom_strcpy(&dst[dst_length], src);
}

int custom_strcmp(char* str1, char* str2)
{
    if(str1 == NULL || str2 == NULL)
    {
        return -1;
    }
    
    int char_index = 0;
    while(str1[char_index] != '\0' && str2[char_index] != '\0')
    {
        if(str1[char_index] != str2[char_index])
        {
            return 1;
        }
        char_index++;
    }
    if(str1[char_index] != str2[char_index]) 
    {
        return 1;
    }
    return 0; 
}

int custom_strcmp_advanced(char* array, char* reference)
{
    if(array == NULL || reference == NULL)
    {
        return -1;
    }

    if((*(long long*)array ^ *(long long*)reference) == 0)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

int custom_strstr(char* origin_str, char* search_str)
{
    if(origin_str == NULL || search_str == NULL)
    {
        return -1;
    }
    int char_index = 0;
    char check_str[128] = {};
    int search_length = custom_strlen(search_str);
    if(search_length > sizeof(check_str) - 1)
    {
        return -1; // Not Supported search_str length
    }

    while (origin_str[char_index] != '\0')
    {
        custom_strcpy(check_str, &origin_str[char_index]);
        check_str[search_length] = '\0';
        if (custom_strcmp(check_str, search_str) == 0)
        {
            return char_index;
        }
        char_index++;
    }
    return -1;
}


int  main()
{
    char str1[128] = "Hello, World!";
    char str2[128] = "Nice to meet you!\n";
    char str3[128] = {};
    
    int length = custom_strlen(str1);
    printf("Length of String: %d, real: %d\n", length, (int)strlen(str1));
    custom_strcpy(str3, str1);
    printf("Copied String: %s\n", str3);
    custom_strcat(str3, str2);
    printf("Concatenated String: %s\n", str3);

    int index = custom_strstr(str3, "World");
    if (index != -1)
    {
        printf("Found 'World' at index: %d\n", index);
    }
    else
    {
        printf("'World' not found in the string.\n");
    }

    double current_time = get_time_sec();
    int check_equal = 0;
    for(int i = 0; i < 1000000; i++)
    {
        custom_strcmp(str1, str2);
    }
    // int check_equal = custom_strcmp_advanced("hello", "NULL");
    double elapsed_time = get_time_sec() - current_time;
    printf("Time taken for custom_strcmp_advanced: %f seconds\n", elapsed_time);
    if (check_equal == 0)
    {
        printf("Strings are equal.\n");
    }
    else
    {
        printf("Strings are not equal.\n");
    }
    


    current_time = get_time_sec();
    char* string1 = "Hello1";
    char* string2 = "Hello";
    int result = 0;
    for(int i = 0; i < 1000000; i++)
    {
        custom_strcmp(string1, string2);
    }
    // int result = custom_strcmp(string1, string2);
    elapsed_time = get_time_sec() - current_time;
    printf("Time taken for custom_strcmp: %f seconds\n", elapsed_time);


    if (result == 0) 
    {
        return 0;
    }
    else 
    {
        return 1;
    }
    return 0;
}
