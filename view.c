#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "view.h"

void view_file(char *filename)
{
    FILE *fp = fopen(filename, "rb");
    if(fp == NULL)
    {
        printf("Unable to open file\n");
        return;
    }
    char header[4];
    fread(header, 1, 3, fp);
    header[3] = '\0';

    if(strcmp(header, "ID3") != 0)
    {
        printf("Invalid MP3 file\n");
        fclose(fp);
        return;
    }

    unsigned char version[2];
    unsigned char flags;
    unsigned char tag_size[4];

    fread(version, 1, 2, fp);
    fread(&flags, 1, 1, fp);
    fread(tag_size, 1, 4, fp);

    int total_size = ((tag_size[0] & 0x7F) << 21) |  ((tag_size[1] & 0x7F) << 14) | ((tag_size[2] & 0x7F) << 7) | (tag_size[3] & 0x7F);

    long tag_end = ftell(fp) + total_size;
    char title[100] = "";
    char artist[100] = "";
    char album[100] = "";
    char year[100] = "";
    char genre[100] = "";
    char comment[100] = "";

    while(ftell(fp) < tag_end)
    {
        char frame_id[5];
        unsigned char frame_size[4];
        unsigned char frame_flags[2];
        if(fread(frame_id, 1, 4, fp) != 4)
            break;

        frame_id[4] = '\0';

        if(frame_id[0] == '\0')
            break;
        if(fread(frame_size, 1, 4, fp) != 4)
            break;
        if(fread(frame_flags, 1, 2, fp) != 2)
            break;
        int size = (frame_size[0] << 24) |(frame_size[1] << 16) | (frame_size[2] << 8) |frame_size[3];

        if(size <= 0 || ftell(fp) + size > tag_end)
            break;

        char *frame_data = malloc(size + 1);
        if(frame_data == NULL)
        {
            printf("Memory allocation failed\n");
            fclose(fp);
            return;
        }
        if(fread(frame_data, 1, size, fp) != size)
        {
            free(frame_data);
            break;
        }
        frame_data[size] = '\0';
        if(strcmp(frame_id, "TIT2") == 0)
        {
            snprintf(title, sizeof(title), "%s", frame_data + 1);
        }
        else if(strcmp(frame_id, "TPE1") == 0)
        {
            snprintf(artist, sizeof(artist), "%s", frame_data + 1);
        }
        else if(strcmp(frame_id, "TALB") == 0)
        {
            snprintf(album, sizeof(album), "%s", frame_data + 1);
        }
        else if(strcmp(frame_id, "TYER") == 0)
        {
            snprintf(year, sizeof(year), "%s", frame_data + 1);
        }
        else if(strcmp(frame_id, "TCON") == 0)
        {
            snprintf(genre, sizeof(genre), "%s", frame_data + 1);
        }
        else if(strcmp(frame_id, "COMM") == 0)
        {
            if(size >= 5)
            {
                snprintf(comment, sizeof(comment), "%s", frame_data + 5);
            }
        }
        free(frame_data);
    }
    printf("\n----- MP3 TAG INFORMATION -----\n");
    printf("Title    : %s\n", title);
    printf("Artist   : %s\n", artist);
    printf("Album    : %s\n", album);
    printf("Year     : %s\n", year);
    printf("Genre    : %s\n", genre);
    printf("Comment  : %s\n", comment);
    fclose(fp);
}