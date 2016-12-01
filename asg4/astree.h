// Colton Willey
// cwwilley@ucsc.edu
//
// Jacob Janowski
// jnjanows@ucsc.edu

#ifndef __ASTREE_H__
#define __ASTREE_H__

#include <string>
#include <vector>
#include <bitset>
using namespace std;

#include "auxlib.h"

enum { ATTR_void, ATTR_int, ATTR_null, ATTR_string,
ATTR_struct, ATTR_array, ATTR_function, ATTR_variable,
ATTR_field, ATTR_typeid, ATTR_param, ATTR_lval, ATTR_const,
ATTR_vreg, ATTR_vaddr, ATTR_bitset_size,
};

using attr_bitset = bitset<ATTR_bitset_size>;

struct location {
   size_t filenr;
   size_t linenr;
   size_t offset;
};

struct astree {
   // Fields.
   int symbol;               // token code
   int block_nr;
   location lloc;            // source location
   location lloc_decl;
   string struct_name;
   const string* lexinfo;    // pointer to lexical information
   vector<astree*> children; // children of this n-way node
   attr_bitset attr;
   // Functions.
   astree (int symbol, const location&, const char* lexinfo);
   astree ();
   ~astree();
   // astree* adopt (astree* one, astree* two, astree* three = nullptr);
   astree* adopt (astree* child1, astree* child2 = nullptr, astree* child3 = nullptr);
   astree* adopt_sym (astree* child, int symbol);
   // astree* new_func(astree*, astree*, astree*);
   astree* sym (int);
   void dump_node (FILE*);
   void print_node();
   void dump_tree (FILE*, int depth = 0);
   static void dump (FILE* outfile, astree* tree);
   static void print (FILE* outfile, astree* tree, int depth = 0);
};
string write_attr(astree* node);
void destroy (astree* tree1, astree* tree2 = nullptr, astree* tree3 = nullptr);
astree* new_func(astree*,astree*,astree*);
void errllocprintf (const location&, const char* format, const char*);

#endif

