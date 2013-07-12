#include "hotpot/hp_error.h"
#include "globals.h"

#include "hotprotocol/hp_json_writer.h"
#include "hotscript/hotobject.h"
#include "hotscript/hot_vm.h"
#include "hotscript/hotobject_reader.h"
#include "hotscript/hotobject_writer.h"
#include "hotscript/script_parser.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "hotdata_parser.h"
#include "hotprotocol/hp_xml_writer.h"
#include "hotprotocol/hp_xml_reader.h"

#include "language/language_types.h"
#include "language/language_reader.h"
#include "language/language.h"

#include "hotscript/hot_vm.h"
#include <io.h>

#define HOTDATA_VERSION "0.0.1"

void version()
{
  printf("HotData version %s\n", HOTDATA_VERSION);
}

void usage()
{
  fprintf(stderr, "Usage: hd [options] file\n\n");
  fprintf(stderr, "Use hd -help for a list of options\n");
}

void help()
{
	fprintf(stderr, "Usage: thrift [options] file\n");
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "  -version					Print the compiler version\n");
	fprintf(stderr, "  -t filename				Set the template file\n");
	fprintf(stderr, "  -i dir					Add a directory to the list of directories\n");
	fprintf(stderr, "  -j dir					set json output path\n");
	fprintf(stderr, "  -o dir					set output path\n");
}


LanguageLib language_lib;
DATA_PARSER dp;
HP_JSON_WRITER jw;
char file_name[HP_MAX_FILE_PATH_LENGTH];
HotObjectReader reader;
HotObjectWriter writer;
SCRIPT_PARSER sp;
FILE* output_file = NULL;
FILE* json_output_file = NULL;


char root_dir[HP_MAX_FILE_PATH_LENGTH];
char real_script_path[HP_MAX_FILE_PATH_LENGTH];
char path_prefix[HP_MAX_FILE_PATH_LENGTH];

void script_putc(HotVM *self, char c)
{
	fputc(c, (FILE*)self->user_data);
}

void get_real_file_path()
{
	hpuint32 i, count;
	hpuint32 len;
	count = 0;
	

	//首先获得根目录
	len = strlen(root_dir);
	for(i = len - 1; i >= 0; --i)
	{
		if(root_dir[i] == HP_FILE_SEPARATOR)
		{
			++count;
			root_dir[i] = 0;
		}
		if(count == 2)
		{
			break;
		}
	}
	//
	snprintf(path_prefix, HP_MAX_FILE_PATH_LENGTH, "%s%cresource%ctemplate%c", root_dir, HP_FILE_SEPARATOR, HP_FILE_SEPARATOR, HP_FILE_SEPARATOR);
	//strncpy(real_script_path, root_dir, HP_MAX_FILE_PATH_LENGTH);

	if(access(file_name, 00) == 0)
	{
		snprintf(real_script_path, HP_MAX_FILE_PATH_LENGTH, "%s", file_name);		
	}
	else
	{
		snprintf(real_script_path, HP_MAX_FILE_PATH_LENGTH, "%s%s", path_prefix, file_name);
	}
}

int main(int argc, char **argv)
{
	int i;
	HotObject *obj = hotobject_new();
	strncpy(root_dir, argv[0], HP_MAX_FILE_PATH_LENGTH);

	data_parser_init(&dp);
	for (i = 1; i < argc - 1; ++i)
	{
		char* arg;

		arg = strtok(argv[i], " ");
		// Treat double dashes as single dashes
		if (arg[0] == '-' && arg[1] == '-')
		{
			++arg;
		}
		if (strcmp(arg, "-help") == 0)
		{
			help();
			goto ERROR_RET;
		}
		else if (strcmp(arg, "-version") == 0)
		{
			version();
			goto ERROR_RET;
		}
		else if (strcmp(arg, "-t") == 0)
		{
			arg = argv[++i];
			if (arg == NULL)
			{
				fprintf(stderr, "Missing template file specification\n");
				usage();
				goto ERROR_RET;
			}
			strncpy(file_name, arg, HP_MAX_FILE_PATH_LENGTH);
		}
		else if (strcmp(arg, "-i") == 0)
		{
			arg = argv[++i];
			if (arg == NULL)
			{
				fprintf(stderr, "Missing template file specification\n");
				usage();
				goto ERROR_RET;
			}
			scanner_stack_add_path(&dp.scanner_stack, arg);
		}
		else if(strcmp(arg, "-j") == 0)
		{
			arg = argv[++i];
			if (arg == NULL)
			{
				fprintf(stderr, "Missing template file specification\n");
				usage();
				goto ERROR_RET;
			}
			json_output_file = fopen(arg, "wb");
		}
		else if(strcmp(arg, "-o") == 0)
		{
			arg = argv[++i];
			if (arg == NULL)
			{
				fprintf(stderr, "Missing template file specification\n");
				usage();
				goto ERROR_RET;
			}
			output_file = fopen(arg, "wb");;
		}
		else
		{
			fprintf(stderr, "Unrecognized option: %s\n", arg);
			usage();
			goto ERROR_RET;
		}
	}
	
	load_language(&language_lib, "D:\\HotPot\\resource\\language\\simplified_chinese.xml");
	

	if(json_output_file != NULL)
	{
		ddekit_json_encoding_writer_init(&jw, json_output_file);
		
		if(data_parser(&dp, argv[i], &jw.super, &language_lib) != E_HP_NOERROR)
		{
			goto ERROR_RET;
		}
		fclose(json_output_file);
		json_output_file = NULL;

		printf("Json output succeed.\n");
	}

	if(output_file == NULL)
	{
		goto ERROR_RET;
	}
	
	
	
	hotobject_writer_init(&writer, obj);
	if(data_parser(&dp, argv[i], &writer.super, &language_lib) != E_HP_NOERROR)
	{
		goto ERROR_RET;
	}

	get_real_file_path();

	hotobject_reader_init(&reader, obj);
	if(script_parser(&sp, real_script_path, &reader.super, output_file, script_putc) != 0)
	{
		goto ERROR_RET;
	}
	printf("template output succeed.\n");
	hotobject_free(obj);

	return 0;
ERROR_RET:
	printf("compile failed\n");
	return 1;
}
