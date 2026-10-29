#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "edit.h"

void edit_tag(char *option, char *value, char *filename)
{
    if(strcmp(filename, "sample.mp3") != 0)
    {
        printf("Only sample.mp3 can be edited\n");
        return;
    }
    FILE *src = fopen(filename, "rb");
    if(src == NULL)
    {
        printf("Unable to open file\n");
        return;
    }
    FILE *temp = fopen("temp.mp3", "wb");
    if(temp == NULL)
    {
        printf("Unable to create temporary file\n");
        fclose(src);
        return;
    }
    unsigned char header[10];
    if(fread(header, 1, 10, src) != 10)
    {
        printf("Unable to read MP3 header\n");
        fclose(src);
        fclose(temp);
        remove("temp.mp3");
        return;
    }

    fwrite(header, 1, 10, temp);
    int total_size = ((header[6] & 0x7F) << 21) | ((header[7] & 0x7F) << 14) |
                     ((header[8] & 0x7F) << 7) |(header[9] & 0x7F);

    long tag_end = ftell(src) + total_size;

    char *target_frame = NULL;
    if(strcmp(option, "-t") == 0)
        target_frame = "TIT2";
    else if(strcmp(option, "-a") == 0)
        target_frame = "TPE1";
    else if(strcmp(option, "-A") == 0)
        target_frame = "TALB";
    else if(strcmp(option, "-y") == 0)
        target_frame = "TYER";
    else if(strcmp(option, "-g") == 0)
        target_frame = "TCON";
    else if(strcmp(option, "-c") == 0)
        target_frame = "COMM";
    else
    {
        printf("Invalid tag option\n");
        fclose(src);
        fclose(temp);
        remove("temp.mp3");
        return;
    }
    int tag_found = 0;
    int old_frame_size = 0;
    int new_frame_size_value = 0;

    while(ftell(src) < tag_end)
    {
        char frame_id[5];
        unsigned char frame_size[4];
        unsigned char frame_flags[2];

        if(fread(frame_id, 1, 4, src) != 4)
            break;

        frame_id[4] = '\0';

        if(frame_id[0] == '\0')
            break;
        if(fread(frame_size, 1, 4, src) != 4)
            break;
        if(fread(frame_flags, 1, 2, src) != 2)
            break;

        int size = (frame_size[0] << 24) |
                   (frame_size[1] << 16) |
                   (frame_size[2] << 8) |
                   frame_size[3];

        if(size <= 0 || ftell(src) + size > tag_end)
            break;
        char *frame_data = malloc(size + 1);
        if(frame_data == NULL)
        {
            printf("Memory allocation failed\n");
            fclose(src);
            fclose(temp);
            remove("temp.mp3");
            return;
        }
        if(fread(frame_data, 1, size, src) != size)
        {
            free(frame_data);
            fclose(src);
            fclose(temp);
            remove("temp.mp3");
            return;
        }
        frame_data[size] = '\0';
        if(strcmp(frame_id, target_frame) == 0)
        {
            tag_found = 1;
            old_frame_size = size;
            if(strcmp(frame_id, "COMM") == 0)
            {
                new_frame_size_value = 5 + strlen(value);
                unsigned char new_size[4];
                new_size[0] = (new_frame_size_value >> 24) & 0xFF;
                new_size[1] = (new_frame_size_value >> 16) & 0xFF;
                new_size[2] = (new_frame_size_value >> 8) & 0xFF;
                new_size[3] = new_frame_size_value & 0xFF;
                fwrite(frame_id, 1, 4, temp);
                fwrite(new_size, 1, 4, temp);
                fwrite(frame_flags, 1, 2, temp);

                fwrite(frame_data, 1, 5, temp);
                fwrite(value, 1, strlen(value), temp);
            }
            else
            {
                new_frame_size_value = 1 + strlen(value);
                unsigned char new_size[4];
                new_size[0] = (new_frame_size_value >> 24) & 0xFF;
                new_size[1] = (new_frame_size_value >> 16) & 0xFF;
                new_size[2] = (new_frame_size_value >> 8) & 0xFF;
                new_size[3] = new_frame_size_value & 0xFF;
                unsigned char encoding = 0;
                fwrite(frame_id, 1, 4, temp);
                fwrite(new_size, 1, 4, temp);
                fwrite(frame_flags, 1, 2, temp);
                fwrite(&encoding, 1, 1, temp);
                fwrite(value, 1, strlen(value), temp);
            }
        }
        else
        {
            fwrite(frame_id, 1, 4, temp);
            fwrite(frame_size, 1, 4, temp);
            fwrite(frame_flags, 1, 2, temp);
            fwrite(frame_data, 1, size, temp);
        }
        free(frame_data);
    }
    if(tag_found == 0)
    {
        printf("Tag not found\n");
        fclose(src);
        fclose(temp);
        remove("temp.mp3");
        return;
    }
    int ch;
    while((ch = fgetc(src)) != EOF)
    {
        fputc(ch, temp);
    }
    int new_total_size = total_size - old_frame_size + new_frame_size_value;
    header[6] = (new_total_size >> 21) & 0x7F;
    header[7] = (new_total_size >> 14) & 0x7F;
    header[8] = (new_total_size >> 7) & 0x7F;
    header[9] = new_total_size & 0x7F;

    fseek(temp, 0, SEEK_SET);
    fwrite(header, 1, 10, temp);
    fclose(src);
    fclose(temp);
    if(rename("temp.mp3", "sample.mp3") != 0)
    {
        printf("Unable to replace output file\n");
        remove("temp.mp3");
        return;
    }
    printf("Tag updated successfully\n");
}
