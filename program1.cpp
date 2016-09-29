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

// TODO use shittier comments

// Cant say it's not descriptive
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
	for (;;) {
      char buffer[LINESIZE];
      char* fgets_rc = fgets (buffer, LINESIZE, pipe);
      if (fgets_rc == NULL) break;
      chomp (buffer, '\n');
      printf ("%s:line %d: [%s]\n", filename, linenr, buffer);

      // http://gcc.gnu.org/onlinedocs/cpp/Preprocessor-Output.html
      int sscanf_rc = sscanf (buffer, "# %d \"%[^\"]\"",
                              &linenr, filename);
      if (sscanf_rc == 2) {
         printf ("DIRECTIVE: line %d file \"%s\"\n", linenr, filename);
         continue;
      }

      char* savepos = NULL;
      char* bufptr = buffer;
      for (int tokenct = 1;; ++tokenct) {
         char* token = strtok_r (bufptr, " \t\n", &savepos);
         bufptr = NULL;
         if (token == NULL) break;
         const string* str = string_set::intern (token);
         printf ("intern (\"%s\") returned %p->\"%s\"\n",
              filename, str, str->c_str());
         //printf ("token %d.%d: [%s]\n",
         //        linenr, tokenct, token);
         // Dont know if we will need this?
      }
      ++linenr;
   }

   // TODO dump to open FILE* on filename
   string_set::dump (stdout);
}

int main(int argc, char *argv[])
{
	// A bunch of options that we dont actually use rn
	int option = 0;
	int yydebug = 0;
	int yy_flex_debug = 0;
	int debug = 0;
	int mycppopt = 0;
	string cpparg = "";

	// Spec said use getopt, which turns out to be sick AF
	while ((option = getopt(argc, argv, "ly@D:")) != -1)
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
				break;
			case 'D':
				mycppopt = 1;
				cpparg = string(optarg);
				break;
			default:
				exit(EXIT_FAILURE);
		}
	}

	int exit_status = EXIT_SUCCESS;

	// optind is global extern brought in by the header containing getopt.
	// it is incremented by the getopt internals for each successful argument
	// parsed.  After getopt returns -1 you are guaranteed that the first
	// non-option index in argv will be located at optind
	string filename = string(argv[optind]);

	// basename, also sick AF. Returns the name of the file without
	// an absolute path prepended or extensions. 
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

	string command = CPP + " " + filename;
	printf("cmd: %s\n", command.c_str());
	FILE *pipe = popen(command.c_str(), "r");
	if (pipe == NULL)
	{
		fprintf(stderr, "%s: %s: %s\n", execname, command.c_str(), strerror (errno));
		exit_status = EXIT_FAILURE;
	}

	create_str_table_for_file(pipe, (char *)filename.c_str());
	int pclose_rv = pclose(pipe);
	if (pclose_rv != 0) exit_status = EXIT_FAILURE;

	return exit_status;
}

