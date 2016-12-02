// Colton Willey
// cwwilley@ucsc.edu
//
// Jacob Janowski
// jnjanows@ucsc.edu

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <libgen.h>
#include <string.h>
#include "string_set.h"
#include "lyutils.h"
#include "symstack.h"

using namespace std;

const string CPP = "/usr/bin/cpp";
constexpr size_t LINESIZE = 1024;

// function prototypes
void do_scan(char *filename);


// Chomp the last character from a buffer if it is delim.
void chomp (char* string, char delim) {
   size_t len = strlen (string);
   if (len == 0) return;
   char* nlpos = string + len - 1;
   if (*nlpos == delim) *nlpos = '\0';
}

void cpp_popen (string filename) {
   string cpp_command = CPP + " " + filename;
   yyin = popen (cpp_command.c_str(), "r");
   if (yyin == NULL) {
      syserrprintf (cpp_command.c_str());
      exit (exec::exit_status);
   }else {
      if (yy_flex_debug) {
         fprintf (stderr, "-- popen (%s), fileno(yyin) = %d\n",
                  cpp_command.c_str(), fileno (yyin));
      }
      // lexer::newfilename (cpp_command);
   }
}

void dump_parse(char *filename)
{
    string bname = string(basename(filename));
    string ofile = bname.substr(0, 
                        bname.find_last_of(".")) + ".ast";
    FILE *fp = fopen(ofile.c_str(), "w");
    parser::root->print(fp, parser::root, 0);
    fclose(fp);
}

void do_scan(char *filename)
{
    struct string_set ss;
    for (;;)
    {
        fflush(NULL);
        int token = yylex();
        if (token == YYEOF) break;
        ss.intern(yytext);
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
    yy_flex_debug = 0;
    int option = 0;
    yydebug = 0;
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
    exec::execname = string(execname);

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
    // FILE *pipe = popen(command.c_str(), "r");
    string bname = string(basename(&filename[0]));
    string ofile = bname.substr(0,
                                bname.find_last_of(".")) + ".tok";
    string symfile = bname.substr(0,
                                bname.find_last_of(".")) + ".sym";
    set_tokout(ofile);
    // yyin = popen(command.c_str(), "r");
    cpp_popen(filename);
    if (yyin == NULL)
    {
        fprintf(stderr, "%s: %s: %s\n", execname, command.c_str(),
                strerror (errno));
        exit_status = EXIT_FAILURE;
    }

    yyparse();
    do_scan((char *)filename.c_str());
    if (parser::root != NULL)
        dump_symtables(parser::root, symfile);

    dump_parse((char *)filename.c_str());
    
    int pclose_rv = pclose(yyin);
    if (pclose_rv != 0) {exit_status = EXIT_FAILURE;}
    return exit_status;
}

