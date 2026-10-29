#include <stdio.h>
#include <string.h>

#include "view.h"
#include "edit.h"

void help()
{
    printf("\n----- HELP -----\n");
    printf("-v : View MP3 tags\n");
    printf("-e : Edit MP3 tags\n");
    printf("-t : Edit Title\n");
    printf("-a : Edit Artist\n");
    printf("-A : Edit Album\n");
    printf("-y : Edit Year\n");
    printf("-g : Edit Genre\n");
    printf("-c : Edit Comment\n");
    printf("-h : Help\n");
}

int main(int argc, char *argv[])
{
    // Command line mode 
    if(argc == 2 && strcmp(argv[1], "-h") == 0)
    {
        help();
        return 0;
    }
    if(argc == 3 && strcmp(argv[1], "-v") == 0)
    {
        view_file(argv[2]);
        return 0;
    }
    if(argc == 5 && strcmp(argv[1], "-e") == 0)
    {
        edit_tag(argv[2], argv[3], argv[4]);
        return 0;
    }
    //menu
    int choice;
    do
    {
        printf("\n----- MP3 TAG READER -----\n");
        printf("1. Help\n");
        printf("2. View\n");
        printf("3. Edit\n");
        printf("4. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);
        switch(choice)
        {
            case 1:
                help();
                break;

            case 2:
                printf("Error: No such file or directory\n");
                break;

            case 3:
                printf("Use command: ./a.out -e <tag> <value> <filename>\n");
                break;

            case 4:
                printf("Exit\n");
                break;

            default:
                printf("Invalid choice\n");
        }

    } while(choice != 4);
    return 0;
}