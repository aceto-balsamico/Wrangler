#include <stdio.h>
#include <string.h>

int custom_strlen(char* str)
{
    int char_index = 0;
    while(str[char_index] != '\0')
    {
        char_index++;
    }
    return char_index;
}

int custom_strcpy(char* dst, char* src)
{
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
    int dst_length = custom_strlen(dst);
    custom_strcpy(&dst[dst_length], src);
}

int custom_strcmp(char* str1, char* str2)
{
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

int custom_strstr(char* origin_str, char* search_str)
{
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

    char* string1 = "Hello1";
    char* string2 = "Hello";
    int result = custom_strcmp(string1, string2);
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
