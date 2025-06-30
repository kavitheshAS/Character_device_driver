#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int8_t write_buffer[1024];
int8_t read_buffer[1024];

int main()
{
    int fd;
    char input[256]; // Change the size if necessary
    char options;

    printf("Demo of Character Device Driver ......\n");

    fd = open("/dev/chr_device", O_RDWR);

    if (fd < 0)
    {
        printf("Cannot open the device file ....!\n");
        return 0;
    }

    while (1)
    {
        printf("Please enter your options from the below Menu ......\n");
        printf("1. Write\n");
        printf("2. Read\n");
        printf("3. Exit\n");

        fgets(input, sizeof(input), stdin);

        if (sscanf(input, " %c", &options) != 1)
        {
            printf("Invalid input. Please enter a valid option.\n");
            continue;
        }

        printf("Your option is = %c \n", options);

        switch (options)
        {
            case '1':
                printf("Enter the string:\n");
                fgets(write_buffer, sizeof(write_buffer), stdin);
                write(fd, write_buffer, strlen(write_buffer) + 1);
                printf("The Data is written ......! Done ...!\n");
                break;

            case '2':
                printf("Data is reading ....\n");
                read(fd, read_buffer, sizeof(read_buffer));
                printf("Done ...!\n");
                printf("Data = %s\n", read_buffer);
                break;

            case '3':
                close(fd);
                exit(0);

            default:
                printf("Invalid Option, Enter a valid option = %c \n", options);
        }
    }

    close(fd);
    return 0;
}
