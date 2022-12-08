#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <inttypes.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

#define EXIST (1u)
#define NOT_EXIST (0)

typedef struct
{
	uint8_t l : 1;
	uint8_t a : 1;
	uint8_t R : 1;
    uint8_t i : 1;
    uint8_t _1 : 1;
} optFlags_s;

typedef union
{
	optFlags_s fields;
	uint8_t whole_flags;
} optFlags_u;

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

char extract_options(int argc, char *argv[], optFlags_u *opt_flags, int *extra_args_index);
void long_format_list_with_hidden_or_not(int argc, char **argv, char with_hidden, char with_inode);
void normal_list(int argc, char **argv, char with_inode, char with_hidden, char separate);


int main(int argc, char **argv)
{
    optFlags_u opt_flags = {.whole_flags = NOT_EXIST};
	int extra_args_index = -1;
char opt_state = extract_options(argc, argv, &opt_flags, &extra_args_index);

	if (opt_state != 0)
	{
		printf("unknown option -%c\n", opt_state);
		return -1;
	}

	if (opt_flags.whole_flags == 0)
	{
		normal_list(argc,argv,'0','0','0');
	}
    else if(opt_flags.whole_flags == 1){
        long_format_list_with_hidden_or_not(argc, argv,'0','0');
    }
    else if(opt_flags.whole_flags == 3){
        long_format_list_with_hidden_or_not(argc, argv,'1','0');
    }
    else if(opt_flags.whole_flags == 9){
        long_format_list_with_hidden_or_not(argc, argv,'0','1');
    }
    else if(opt_flags.whole_flags == 11){
        long_format_list_with_hidden_or_not(argc, argv,'1','1');
    }
    else if(opt_flags.whole_flags == 2){
        normal_list(argc, argv,'0','1','0');
    }
    else if(opt_flags.whole_flags == 8){
        normal_list(argc, argv,'1','0','0');
    }
    else if(opt_flags.whole_flags == 10){
        normal_list(argc, argv,'1','1','0');
    }
    else if(opt_flags.whole_flags == 16){
        normal_list(argc, argv,'0','0','1');
    }
    else if(opt_flags.whole_flags == 18){
        normal_list(argc, argv,'0','1','1');
    }
    else if(opt_flags.whole_flags == 24){
        normal_list(argc, argv,'1','0','1');
    }
    else if(opt_flags.whole_flags == 26){
        normal_list(argc, argv,'1','1','1');
    }

    return 0;
}


char extract_options(int argc, char *argv[], optFlags_u *opt_flags, int *extra_args_index)
{
	opt_flags->whole_flags = NOT_EXIST;
	char option;
	char ret_val = 0;

	while ((option = getopt(argc, argv, "lai1R")) != -1)
	{
		switch (option)
		{
		case 'l':
			opt_flags->fields.l = EXIST;
			break;
		case 'a':
			opt_flags->fields.a = EXIST;
			break;
		case 'R': //not used yet
			opt_flags->fields.R = EXIST;
			break;
        case 'i':
			opt_flags->fields.i = EXIST;
			break;
		case '1':
			opt_flags->fields._1 = EXIST;
			break;
		case '?':
			ret_val = optopt;
			break;
		}
	}
	*extra_args_index = optind;
	return ret_val;
}


void long_format_list_with_hidden_or_not(int argc, char **argv, char with_hidden, char with_inode){
    char first = 0;
    char num_of_loops = 0;
    /*for loop for list the files in any dir of in the current path*/
    for (int i = optind; i <= argc; i++)
    {
        DIR *dirptr;
        /*to list the files in the curr path*/
        if(optind == argc){
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
            if((with_hidden == '0') && (entry->d_name[0] == '.')){continue;}
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
            if(with_inode == '1'){
                printf("%7lu ", entry->d_ino);
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
        if(i == (argc - 1)){
            break;
        }
    }
}

void normal_list(int argc, char **argv, char with_inode, char with_hidden, char separate){
    char first = 0;
    char num_of_loops = 0;
    /*for loop for list the files in any dir of in the current path*/
    for (int i = optind; i <= argc; i++)
    {
        DIR *dirptr;
        /*to list the files in the curr path*/
        if(optind == argc){
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
            if((with_hidden == '0') && (entry->d_name[0] == '.')){continue;}
            if(with_inode == '0' && separate == '0'){
                printf("%s  ", entry->d_name);
            }
            if(with_inode == '1' && separate == '0'){
                printf("%lu %s  ", entry->d_ino, entry->d_name);
            }
            if(with_inode == '1' && separate == '1'){
                printf("%lu %s\n", entry->d_ino, entry->d_name);
            }
            if(separate == '1' && with_inode == '0'){
                printf("%s  \n", entry->d_name);
            }


        
        }
        puts(""); // separation between the content of each dir
        puts(""); // separation between the content of each dir
        closedir(dirptr);
        if(i == (argc - 1)){
            break;
        }
}
}
