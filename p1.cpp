#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <libgen.h>
#include <string.h>
#include "string_set.h"
using namespace std;

const string CPP = "/usr/bin/cpp";
constexpr size_t LINESIZE = 1024;

// function prototypes
void create_str_table_for_file(FILE *pipe, char *filename);


// Chomp the last character from a buffer if it is delim.
void chomp (char* string, char delim) {
   size_t len = strlen (string);
   if (len == 0) return;
   char* nlpos = string + len - 1;
   if (*nlpos == delim) *nlpos = '\0';
}

void create_str_table_for_file(FILE *pipe, char *filename)
{
	int linenr = 1;
    struct string_set ss;

	for (;;)
	{
		char buffer[LINESIZE];
		char *fgets_rc = fgets(buffer, LINESIZE, pipe);
		if (fgets_rc == NULL) break;
		chomp(buffer, '\n');

		int sscanf_rc = sscanf(buffer, "# %d \"%[^\"]\"",
							   &linenr, filename);
		if (sscanf_rc == 2)
		{
			continue;
		}

		char *savepos = NULL;
		char *bufptr = buffer;
		for (int tokenct = 1;; ++tokenct)
		{
			char *token = strtok_r(bufptr, " \t\n", &savepos);
			bufptr = NULL;
			if (token == NULL) break;
            ss.intern(token);
		}
		++linenr;
	}

	string bname = string(basename(filename));
	string ofile = bname.substr(0,
                                bname.find_last_of(".")) + ".str";
	FILE *fp = fopen(ofile.c_str(), "w");
    ss.dump(fp);

	fclose(fp);
}

int main(int argc, char *argv[])
{
	// A bunch of options that we dont actually use at the moment
	int option = 0;
	int yydebug = 0;
	int yy_flex_debug = 0;
	int debug = 0;
	int cppopt = 0;
	string cpparg = "";
    string debugarg = "";

	// Parse command line options with getopt
	while ((option = getopt(argc, argv, "ly@:D:")) != -1)
	{
		switch(option)
		{
			case 'y':
				yydebug = 1;
				break;
			case 'l':
				yy_flex_debug = 1;
				break;
			case '@':
				debug = 1;
                debugarg = string(optarg);
				break;
			case 'D':
				cppopt = 1;
                // Did not know if he was gonna pass it as
                // ./oc -D __OCLIB_OH__ program.oc
                // or as ./oc -D -D__OCLIB_OH__ program.oc
                // so just handle both cases to be safe
                cpparg = ((string(optarg).substr(0, 2) == "-D") ?
                          string(optarg) + " " :
                          "-D" + string(optarg) + " ");
				break;
			default:
				exit(EXIT_FAILURE);
		}
	}

    if (yydebug || yy_flex_debug || debug || cppopt)
    {
        // Flag handling code here, not applicable atm
        // Check them to make compiler happy
    }

	int exit_status = EXIT_SUCCESS;

	// optind contains the index of the first non-argument element
	string filename = string(argv[optind]);

	// strip any leading path using basename
	const char *execname = basename(argv[optind]);
		
	if (filename.size() <= 0)
	{
		fprintf(stderr, "ERROR Please enter a file name!\n");
		exit(EXIT_FAILURE);
	}

	if (filename.substr(filename.length() - 3, 3) != ".oc")
	{
		fprintf(stderr, "ERROR Please enter a valid .oc file!\n");
		exit(EXIT_FAILURE);
	}

    // Run input through C preprocessor then parse the output
	string command = CPP + " " + cpparg + " " + filename;
	FILE *pipe = popen(command.c_str(), "r");
	if (pipe == NULL)
	{
		fprintf(stderr, "%s: %s: %s\n", execname, command.c_str(),
                strerror (errno));
		exit_status = EXIT_FAILURE;
	}

	create_str_table_for_file(pipe, (char *)filename.c_str());
	int pclose_rv = pclose(pipe);
	if (pclose_rv != 0) {exit_status = EXIT_FAILURE;}

    exit(exit_status);
}

