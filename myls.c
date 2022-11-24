#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <inttypes.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>


/*struct of 16 bits of the permession of any file*/
typedef struct
{
    uint16_t other_x : 1;
    uint16_t other_w : 1;
    uint16_t other_r : 1;
    uint16_t group_x : 1;
    uint16_t group_w : 1;
    uint16_t group_r : 1;
    uint16_t user_x : 1;
    uint16_t user_w : 1;
    uint16_t user_r : 1;
    uint16_t sticky : 1;
    uint16_t sgid : 1;
    uint16_t suid : 1;
    uint16_t file_type : 4;
} mode_s;

/*union of 16 bits of the permession of any file*/
typedef union
{
    mode_s mode;
    uint16_t file_per;
} mode_u;

char file_type;
char* month;
int main(int argc, char **argv)
{
    char first = 0;
    char num_of_loops = 0;
    /*for loop for list the files in any dir of in the current path*/
    for (int i = 1; i <= argc; i++)
    {
        DIR *dirptr;
        /*to list the files in the curr path*/
        if(argc == 1){
            char cwd_buf[100];
            char *cwd = getcwd(cwd_buf, 100);
            printf("CWD: %s\n", cwd);
            dirptr = opendir(cwd);
        }
        /*to list all files in the dir which passed to the myls command*/
        else{
            num_of_loops++;
            if(first != 0){
                chdir("../");
            }
            first = 1;
            printf("%s:\n", argv[i]);
            dirptr = opendir(argv[i]);
            chdir(argv[i]);
            }
        struct dirent *entry;
        struct stat mystat;
        while ((entry = readdir(dirptr)) != NULL)
        {
            if ((stat(entry->d_name, &mystat)) < 0)
            {
                printf("stat failed to %s\n", entry->d_name);
            }
            mode_u mode_1;
            mode_1.file_per = mystat.st_mode;
            struct tm *time = gmtime(&(mystat.st_ctim.tv_sec)); //to get time stamps
            struct passwd *user_s = getpwuid(mystat.st_uid); //to get user name
            struct group *group_s = getgrgid(mystat.st_gid); //to get group name
            /*these conditions to find the type of the file (directory, character device, block device, FIFO, symbolic link, socket)*/
            if ((mystat.st_mode & S_IFMT) == S_IFREG)
            {
                file_type = '-';
            }
            else if ((mystat.st_mode & S_IFMT) == S_IFLNK)
            {
                file_type = 'l';
            }
            else if ((mystat.st_mode & S_IFMT) == S_IFDIR)
            {
                file_type = 'd';
            }
            else if ((mystat.st_mode & S_IFMT) == S_IFSOCK)
            {
                file_type = 's';
            }
            else if ((mystat.st_mode & S_IFMT) == S_IFBLK)
            {
                file_type = 'b';
            }
            else if ((mystat.st_mode & S_IFMT) == S_IFCHR)
            {
                file_type = 'c';
            }
            else if ((mystat.st_mode & S_IFMT) == S_IFIFO)
            {
                file_type = 'p';
            }
            /*these conditions to handls the month in the time struct of the file*/
            switch (time->tm_mon){
                case 0:
                    month = "Jan";
                    break;
                case 1:
                    month = "Feb";
                    break;
                case 2:
                    month = "Mar";
                    break;
                case 3:
                    month = "Apr";
                    break;
                case 4:
                    month = "May";
                    break;
                case 5:
                    month = "Jun";
                    break;
                case 6:
                    month = "Jul";
                    break;
                case 7:
                    month = "Aug";
                    break;
                case 8:
                    month = "Sep";
                    break;
                case 9:
                    month = "Oct";
                    break;
                case 10:
                    month = "Nov";
                    break;
                case 11:
                    month = "Dec";
                    break;
                default:
                    break;
            }
            /*the output of the ls -la format*/
            printf("%c%c%c%c%c%c%c%c%c%c ",
                   file_type,
                   (mode_1.mode.user_r == 1) ? 'r' : '-',
                   (mode_1.mode.user_w == 1) ? 'w' : '-',
                   (mode_1.mode.user_x == 1) ? 'x' : '-',
                   (mode_1.mode.group_r == 1) ? 'r' : '-',
                   (mode_1.mode.group_w == 1) ? 'w' : '-',
                   (mode_1.mode.group_x == 1) ? 'x' : '-',
                   (mode_1.mode.other_r == 1) ? 'r' : '-',
                   (mode_1.mode.other_w == 1) ? 'w' : '-',
                   (mode_1.mode.other_x == 1) ? 'x' : '-');
            printf("%*lu", 4, mystat.st_nlink);
            printf("%*s", 10, user_s->pw_name);
            printf("%*s", 10, group_s->gr_name);
            printf("%*lu  ", 10, (mystat.st_blocks * 512));
            printf("%s %2d %2d:",month, time->tm_mday, time->tm_hour + 2);
            if((time->tm_min) < 10){
                printf("0%d ", time->tm_min);
            }
            else if((time->tm_min) >= 10){
                printf("%2d ",time->tm_min);
            }
            printf("%s\n", entry->d_name );
        }
        puts(""); // separation between the content of each dir
        closedir(dirptr);
        if(num_of_loops == (argc - 1)){
            break;
        }
    }

    return 0;
}
