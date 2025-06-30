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
    int src_length = custom_strlen(src);
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

int custom_strstr(char* str, char search_char)
{
    int char_index = 0;
    while(str[char_index] != '\0')
    {
        if(str[char_index] == search_char)
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

    int index = custom_strstr(str3, 'W');
    if (index != -1) 
    {
        printf("Found 'W' at index: %d\n", index);
    } 
    else 
    {
        printf("'W' not found\n");
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
