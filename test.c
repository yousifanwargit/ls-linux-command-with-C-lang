#include <stdio.h>
#include <unistd.h>
#include <inttypes.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <grp.h>
#include <pwd.h>
#include <string.h>

#define EXIST (1u)
#define NOT_EXIST (0)

typedef struct
{
	uint8_t l : 1;
	uint8_t a : 1;
	uint8_t R : 1;
} optFlags_s;

typedef union
{
	optFlags_s fields;
	uint8_t whole_flags;
} optFlags_u;

typedef struct
{
	uint16_t others_x : 1;
	uint16_t others_w : 1;
	uint16_t others_r : 1;
	uint16_t group_x : 1;
	uint16_t group_w : 1;
	uint16_t group_r : 1;
	uint16_t owner_x : 1;
	uint16_t owner_w : 1;
	uint16_t owner_r : 1;

	uint16_t sticky : 1;
	uint16_t SGID : 1;
	uint16_t SUID : 1;

	uint16_t file_type : 4;
} fileMode_s;

#define FIFO_FT (001)
#define CHAR_FT (002)
#define DIR_FT (004)
#define BLK_FT (006)
#define REG_FT (010)
#define LNK_FT (012)
#define SOCK_FT (014)

char extract_options(int argc,
					 char *argv[],
					 optFlags_u *opt_flags,
					 int *extra_args_index);

int get_entries(DIR *dirp,
				struct dirent ***dir_list,
				int *entries_count,
				optFlags_u opt_flags);

int _ls(char *dir_name, optFlags_u opt_flags);

int main(int argc, char *argv[])
{

	optFlags_u opt_flags = {.whole_flags = NOT_EXIST};
	int extra_args_index = -1;

	char opt_state = extract_options(argc,
									 argv,
									 &opt_flags,
									 &extra_args_index);
	if (opt_state != 0)
	{
		printf("unknown option -%c\n", opt_state);
		return -1;
	}

	if (extra_args_index == argc)
	{

		_ls(".", opt_flags);
	}
	else
	{
		for (; extra_args_index < argc; extra_args_index++)
			_ls(argv[extra_args_index], opt_flags);
	}
	return 0;
}

char extract_options(int argc,
					 char *argv[],
					 optFlags_u *opt_flags,
					 int *extra_args_index)
{
	opt_flags->whole_flags = NOT_EXIST;
	char option;
	char ret_val = 0;

	while ((option = getopt(argc, argv, "laR")) != -1)
	{
		switch (option)
		{
		case 'l':
			opt_flags->fields.l = EXIST;
			break;
		case 'a':
			opt_flags->fields.a = EXIST;
			break;
		case 'R':
			opt_flags->fields.R = EXIST;
			break;
		case '?':
			ret_val = optopt;
			break;
		}
	}
	*extra_args_index = optind;
	return ret_val;
}

int get_entries(DIR *dirp,
				struct dirent ***dir_list,
				int *entries_count,
				optFlags_u opt_flags)
{
	int dynamic_size = 128;
	struct dirent **temp_list = (struct dirent **)malloc(dynamic_size * sizeof(struct dirent *));
	if (dir_list == NULL)
		return -1;

	struct dirent *temp;

	*entries_count = 0;

	while ((temp = readdir(dirp)) != NULL)
	{
		if (temp->d_name[0] != '.')
		{
			temp_list[*entries_count] = temp;
			(*entries_count)++;
		}
		else if (temp->d_name[0] == '.' && opt_flags.fields.a == EXIST)
		{

			temp_list[*entries_count] = temp;
			(*entries_count)++;
		}

		if (*entries_count == dynamic_size)
		{
			dynamic_size += 128;
			temp_list = realloc((void *)dir_list, dynamic_size * sizeof(struct dirent *));
			if (temp_list == NULL)
				return -2;
		}
	}

	temp_list = realloc((void *)temp_list, *entries_count * sizeof(struct dirent *));
	*dir_list = temp_list;
	if (temp_list == NULL)
		return -3;

	return 1;
}

int _ls(char *dir_name,
		optFlags_u opt_flags)

{
	struct dirent **dir_list = NULL;
	DIR *dirp = NULL;
	int entries_count = 0;
	int i;
	struct stat statbuf;
	char file_type;
	struct group *group_data = NULL;
	struct passwd *user_data = NULL;
	char *current_dir = getcwd(NULL, 0);
	fileMode_s *file_mode = NULL;
	chdir(dir_name);
	dirp = opendir(".");
	get_entries(dirp, &dir_list, &entries_count, opt_flags); //-a option is handles here

	if (opt_flags.fields.R == EXIST)
		printf("%s:\n", getcwd(NULL, 0));
	
	for (i = 0; i < entries_count; i++)
	{
		if (opt_flags.fields.l == EXIST)
		{
			stat(dir_list[i]->d_name, &statbuf);
			file_mode =(fileMode_s *) &statbuf.st_mode;
			switch (file_mode->file_type)
			{
			case FIFO_FT:
				file_type = 'f';
				break;
			case CHAR_FT:
				file_type = 'c';
				break;
			case DIR_FT:
				file_type = 'd';
				break;
			case BLK_FT:
				file_type = 'b';
				break;
			case REG_FT:
				file_type = '-';
				break;
			case LNK_FT:
				file_type = 'l';
				break;
			case SOCK_FT:
				file_type = 's';
				break;
			}
			user_data = getpwuid(statbuf.st_uid);
			group_data = getgrgid(statbuf.st_gid);
			printf("%c%c%c%c%c%c%c%c%c%c %ld %s %s %ld ",
				   file_type,
				   (file_mode->owner_r) ? 'r' : '-',
				   (file_mode->owner_w) ? 'w' : '-',
				   (file_mode->owner_x) ? 'x' : '-',
				   (file_mode->group_r) ? 'r' : '-',
				   (file_mode->group_w) ? 'w' : '-',
				   (file_mode->group_x) ? 'x' : '-',
				   (file_mode->others_r) ? 'r' : '-',
				   (file_mode->others_w) ? 'w' : '-',
				   (file_mode->others_x) ? 'x' : '-',

				   statbuf.st_nlink,
				   group_data->gr_name,
				   user_data->pw_name,
				   statbuf.st_size);
		}
		printf("%s\t", dir_list[i]->d_name);
		if (opt_flags.fields.l == EXIST)
			printf("\n");
	}

	printf("\n");

	if (opt_flags.fields.R == EXIST)
{	
for (i = 0; i < entries_count; i++)
	{
		stat(dir_list[i]->d_name, &statbuf);
		file_mode = (fileMode_s *)&statbuf.st_mode;

		if (file_mode->file_type == DIR_FT)
		{
			if (strcmp(dir_list[i]->d_name, ".") != 0 && strcmp(dir_list[i]->d_name, "..") != 0)
			{
				printf("\n");
				_ls(dir_list[i]->d_name, opt_flags);
			}
		}
	}
}
	closedir(dirp);
	chdir(current_dir);
	free(current_dir);
}