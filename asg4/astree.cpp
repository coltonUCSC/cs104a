// Colton Willey
// cwwilley@ucsc.edu
//
// Jacob Janowski
// jnjanows@ucsc.edu

#include <assert.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>

#include "astree.h"
#include "string_set.h"
#include "lyutils.h"

astree::astree() {
   symbol = 0;
   lloc = location{0,0,0};
   lexinfo = nullptr;
   block_nr = 0;
   struct_name = "";
}

astree::astree (int symbol_, const location& lloc_, const char* info) {
   symbol = symbol_;
   lloc = lloc_;
   lexinfo = string_set::intern (info);
   block_nr = 0;
   struct_name = "";
   // vector defaults to empty -- no children
}

astree::~astree() {
   while (not children.empty()) {
      astree* child = children.back();
      children.pop_back();
      delete child;
   }
   if (yydebug) {
      fprintf (stderr, "Deleting astree (");
      astree::dump (stderr, this);
      fprintf (stderr, ")\n");
   }
}

astree* new_func(astree* identdecl, astree* params, astree *block) {
   int type = TOK_FUNCTION;
   if (";" == (*block->lexinfo))
      type = TOK_PROTOTYPE;
   astree *node = new astree(type, identdecl->lloc, "");
   node->adopt(identdecl, params, block);
   return node;
}

astree* astree::sym (int sym) {
   symbol = sym;
   return this;
}

// astree* astree::adopt (astree* one, astree* two, astree* three) {
//    this->adopt(one);
//    this->adopt(two, three);
// }

// astree* astree::adopt (int n, ...) {
//    va_list adopt_list;
//    va_start (adopt_list, n);
//    for (int i=0; i<n; ++i)
//       this->adopt(va_arg(adopt_list, i));
//    return this;
// }

// astree* astree::adopt (astree* child) {
//    if (child != nullptr) children.push_back (child1);
//    return this;
// }

astree* astree::adopt (astree* child1, astree* child2, astree* child3) {
   if (child1 != nullptr) children.push_back (child1);
   if (child2 != nullptr) children.push_back (child2);
   if (child3 != nullptr) children.push_back (child3);
   // printf("%s  ->  %s\n", parser::get_yytname(symbol), lexinfo->c_str());
   // if (child1) printf("\t[1]:%s  ->  %s\n", 
   //    parser::get_yytname(child1->symbol), child1->lexinfo->c_str());
   // if (child2) printf("\t[2]:%s  ->  %s\n", 
   //    parser::get_yytname(child2->symbol), child2->lexinfo->c_str());
   // if (child3) printf("\t[3]:%s  ->  %s\n", 
   //    parser::get_yytname(child3->symbol), child3->lexinfo->c_str());
   return this;
}

astree* astree::adopt_sym (astree* child, int symbol_) {
   symbol = symbol_;
   return adopt (child);
}

void astree::print_node()
{
   string attrs = write_attr(this);
   const char *tname = parser::get_yytname(this->symbol);
   if(strstr (tname, "TOK_") == tname) tname +=4;
   printf("%s\n", tname);
   printf("%s \"%s\" (%zd.%zd.%zd) {%d} %s\n",
            tname, this->lexinfo->c_str(),
            this->lloc.filenr, this->lloc.linenr, this->lloc.offset,
            this->block_nr, attrs.c_str());
}

void astree::dump_node (FILE* outfile) {
   fprintf (outfile, "%p->{%s %zd.%zd.%zd \"%s\":",
            this, parser::get_yytname (symbol),
            lloc.filenr, lloc.linenr, lloc.offset,
            lexinfo->c_str());
   for (size_t child = 0; child < children.size(); ++child) {
      fprintf (outfile, " %p", children.at(child));
   }
}

void astree::dump_tree (FILE* outfile, int depth) {
   fprintf (outfile, "%*s", depth * 3, "");
   dump_node (outfile);
   fprintf (outfile, "\n");
   for (astree* child: children) child->dump_tree (outfile, depth + 1);
   fflush (NULL);
}

void astree::dump (FILE* outfile, astree* tree) {
   if (tree == nullptr) fprintf (outfile, "nullptr");
                   else tree->dump_node (outfile);
}

string write_attr(astree* node)
{
   string buffer = "";
   for (int i=0; i<ATTR_bitset_size; ++i)
   {
      if (node->attr[i] == 1) 
      {
         switch(i) 
         {
            case ATTR_void: buffer += "void "; break; 
            case ATTR_int: buffer += "int "; break; 
            case ATTR_null: buffer += "null "; break; 
            case ATTR_string: buffer += "string "; break;
            case ATTR_struct: buffer += "struct ";
                              buffer += "\"" + node->struct_name + "\" ";
                              break; 
            case ATTR_array: buffer += "array "; break; 
            case ATTR_function: buffer += "function "; break; 
            case ATTR_variable: buffer += "variable "; break;
            //case ATTR_field: buffer += "field "; break; 
           // case ATTR_typeid: buffer += "typeid "; break;
            case ATTR_param: buffer += "param "; break;
            case ATTR_lval: buffer += "lval "; break; 
            case ATTR_const: buffer += "const "; break;
            case ATTR_vreg: buffer += "vreg "; break; 
            case ATTR_vaddr: buffer += "vaddr "; break;
         }
      }
   }
   return buffer;
}

void astree::print (FILE* outfile, astree* tree, int depth) {
   for (int i = 0; i < depth; ++i)
   {
      fprintf (outfile, "|  ");
   }
   const char *tname = parser::get_yytname(tree->symbol);
   if(strcmp(tname, "TOK_") == 0) 
      tname +=4;

   string attrs = write_attr(tree);
   fprintf (outfile, "%s \"%s\" (%zd.%zd.%zd) {%d} %s",
            tname, tree->lexinfo->c_str(),
            tree->lloc.filenr, tree->lloc.linenr, tree->lloc.offset,
            tree->block_nr, attrs.c_str());

   if (tree->symbol == TOK_IDENT)
   {
      fprintf (outfile, "(%zd.%zd.%zd)", 
         tree->lloc_decl.filenr, tree->lloc_decl.linenr, tree->lloc_decl.offset);
   }
   fprintf(outfile, "\n");
   // TODO print node definition location for TOK_IDENT
   for (astree* child: tree->children) {
      astree::print (outfile, child, depth + 1);
   }
}

void destroy (astree* tree){destroy(tree, nullptr);}
void destroy (astree* tree1, astree* tree2, astree* tree3) {
   if (tree1 != nullptr) delete tree1;
   if (tree2 != nullptr) delete tree2;
   if (tree3 != nullptr) delete tree3;
}

void errllocprintf (const location& lloc, const char* format,
                    const char* arg) {
   static char buffer[0x1000];
   assert (sizeof buffer > strlen (format) + strlen (arg));
   snprintf (buffer, sizeof buffer, format, arg);
   errprintf ("%s:%zd.%zd: %s", 
              lexer::filename (lloc.filenr), lloc.linenr, lloc.offset,
              buffer);
}
