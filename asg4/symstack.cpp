#include "symstack.h"
#include "lyutils.h"
#include <iostream>
#include <stack>
using namespace std;

//vector of unordered_maps<pair((single)string*, symbol*)>
vector<symbol_table*> symbol_stack;
symbol_table struct_table;
vector<int> block_stack; 
vector<string> function_stack;

int g_block_nr = 0;

void dump_symbol_stack();
void dump_symbol(symbol *sym);

symbol::symbol(astree* node)
{
	filenr = node->lloc.filenr;
	linenr = node->lloc.linenr;
	offset = node->lloc.offset;
	fields = NULL;
	parameters = NULL;
	block_nr = g_block_nr;
}

void init_symtables(astree* node)
{
	// this push back is crucial, a 0 sits on the bottom of the 
	// block stack, inheriting everything not in a more deeply nested block
	block_stack.push_back(0);
	symbol_stack.push_back(NULL);// TODO change to NULL then fix segfaults 
	// dump_symbol_stack(); //%%: DUMP_SYMBOL
	build_symtables(node);
}

// TODO match prototype to function?
void check_enter_block(astree* node)
{
	if (node->symbol == TOK_PARAM)
	{
		// special case, drop the param into
		// the next block number
		g_block_nr++;
		node->block_nr = g_block_nr;
		block_stack.push_back(g_block_nr);
	}
	else if (node->symbol == TOK_BLOCK)
	{
		++g_block_nr;
		block_stack.push_back(g_block_nr);
		symbol_stack.push_back(NULL);// TODO change to NULL then fix segfaults 
		node->block_nr = g_block_nr;
	}
	else if ((node->symbol == TOK_FUNCTION) || (node->symbol == TOK_PROTOTYPE))
	{
		node->block_nr = 0;
		symbol_stack.push_back(NULL);// TODO change to NULL then fix segfaults
	}
	else
	{
		node->block_nr = block_stack.back();
	}
}

void dump_function_stack()
{
	for(size_t i=0; i<function_stack.size(); ++i)
	{
		printf("%s\n", function_stack[i].c_str());
	}
	function_stack.clear();
	printf("\n");
}

void check_leave_block(astree *node)
{
	if (node->symbol == TOK_PARAM)
	{
		// special case, restore block number
		block_stack.pop_back();
		--g_block_nr;
	}
	else if(node->symbol == TOK_BLOCK)
	{
		block_stack.pop_back();
		symbol_stack.pop_back();
	}
	else if ((node->symbol == TOK_FUNCTION) || (node->symbol == TOK_PROTOTYPE))
	{
		dump_function_stack();
		symbol_stack.pop_back();
	}
}

void build_symtables(astree* node) 
{
	check_enter_block(node);
	for (auto child : node->children)
		build_symtables(child);
	processNode(node);
	check_leave_block(node);
	// dump_symbol_stack();
}
	// for (auto it = symbol_stack.end(); it != symbol_stack.begin(); it--)
	// {
		// printf(" %s\n", (*it)->first);
		// auto find = (*it)->find(const_cast<string*>(node->lexinfo));
		// if (find != (*it)->end())
	// }
		// for (auto it = p->begin(); it != p->end(); ++it)
		// {		
		// 	printf("found");
		// }

symbol* lookup_struct(astree *node)
{
	auto find = struct_table.find(const_cast<string*>(node->lexinfo));
	if (find != struct_table.end()) return find->second;
	return NULL;
}

symbol* lookup_symbol(astree *node)
{
	symbol_table *p;
	// printf("search: %s ...", node->lexinfo->c_str());
	for (size_t i = symbol_stack.size()-1; i > 0; i--) 
	{
		p = symbol_stack[i];
		if (p == NULL) continue;
		auto find = p->find(const_cast<string*>(node->lexinfo));
		if (find != p->end())
			return find->second;
			// printf("found");
	}
	// printf("\n");
	return NULL;
}

// Parse lexinfo for type, if found assign appropriately
bool set_type_attr(astree *node)
{
	if ((*(node->lexinfo)) == string("int"))
	{
		node->attr[ATTR_int]= 1;
		return true;
	}
	else if ((*(node->lexinfo)) == string("void"))
	{
		node->attr[ATTR_void]= 1;
		return true;
	}
	else if ((*(node->lexinfo)) == string("null"))
	{
		node->attr[ATTR_null]= 1;
		return true;
	}
	else if ((*(node->lexinfo)) == string("string"))
	{
		node->attr[ATTR_string]= 1;
		return true;
	}

	symbol *sym = lookup_struct(node);
	if (sym != NULL)
	{
		node->attr[ATTR_struct] = 1;
		node->struct_name = (*(node->lexinfo));
		return true;
	}

	return false;
}

void write_symbol(astree *node)
{
	string depth="";
	for (size_t i=0; i<block_stack.size()-1; i++)
		depth += "   ";

	if (node->symbol == TOK_STRUCT)
	{
		string temp = write_attr(node);
		printf("%s (%zd.%zd.%zd) {%d} %s", (node->children[0]->lexinfo)->c_str(), 
					 node->children[0]->lloc.filenr, node->children[0]->lloc.linenr,
					 node->children[0]->lloc.offset, node->block_nr, temp.c_str());
		printf("\n");
		for (auto child : node->children)
		{
			if (child->symbol == TOK_FIELD)
			{
				string temp = write_attr(child);
				printf("   %s (%zd.%zd.%zd) field {%s} %s", (child->children[0]->lexinfo)->c_str(), 
							 child->children[0]->lloc.filenr, child->children[0]->lloc.linenr,
							 child->children[0]->lloc.offset, (node->children[0]->lexinfo)->c_str(), 
							 temp.c_str());
				printf("\n");
			}
		}
		printf("\n");
	}
	else if (node->symbol == TOK_FUNCTION)
	{
		// paranoia line
		string temp = write_attr(node);
		printf("%s (%zd.%zd.%zd) {%d} %s", (node->children[0]->children[0]->lexinfo)->c_str(), 
					 node->children[0]->children[0]->lloc.filenr, node->children[0]->children[0]->lloc.linenr,
					 node->children[0]->children[0]->lloc.offset, node->block_nr, temp.c_str());
		printf("\n");
		for (auto child : node->children[1]->children)
		{
			string temp = write_attr(child->children[0]) + write_attr(child);
			printf("   %s (%zd.%zd.%zd) {%d} %s", (child->children[0]->lexinfo)->c_str(), 
						 child->children[0]->lloc.filenr, child->children[0]->lloc.linenr,
						 child->children[0]->lloc.offset, child->block_nr, temp.c_str());
			printf("\n");
		}
		printf("\n");
	}
	else if (node->symbol == TOK_VARDECL)
	{
		char buff[1024];
		string temp = write_attr(node->children[0]) + write_attr(node->children[0]->children[0]);
		sprintf(buff, "%s%s (%zd.%zd.%zd) {%d} %s", depth.c_str(), (node->children[0]->children[0]->lexinfo)->c_str(), 
					 node->children[0]->children[0]->lloc.filenr, node->children[0]->children[0]->lloc.linenr,
					 node->children[0]->children[0]->lloc.offset, node->block_nr, temp.c_str());
		if (node->block_nr != 0)
			function_stack.push_back(string(buff));
		else
			printf("%s\n", buff);
	}
}

void processNode(astree *node) 
{
	//printf("processNode, symbol_stack.size: %lu node: %s \n", symbol_stack.size(), parser::get_yytname(node->symbol));
	//node->block_nr = g_block_nr;
	switch(node->symbol)
	{
		case TOK_DECLID:
		{
			node->attr[ATTR_variable] = 1;
			break;
		}
		case TOK_INT:
		{
			node->attr[ATTR_int] = 1;
			// INT not guaranteed to have child, could be array type
			if (node->children.size() > 0)
			{
				astree *left = node->children[0];
				if (!left) break;			 
				// left->attr[ATTR_int] = 1;
			}
			break;
		}
		case TOK_STRING:
		{
			node->attr[ATTR_string] = 1;
			break;
		}
		case TOK_INTCON:
		{
			node->attr[ATTR_int] = 1; 
			node->attr[ATTR_const] = 1;
			break;
		}
		case TOK_VOID:
		{
			// TODO??
			node->attr[ATTR_void] = 1;
			break;
		}
		case TOK_VARDECL:
		{
			astree *left = node->children[0];
			left->children[0]->attr[ATTR_lval] = 1; 
			left->children[0]->attr[ATTR_variable] = 1;
			symbol *sym = new symbol(left->children[0]);
			if (symbol_stack.back() == NULL)
			{
				symbol_stack[symbol_stack.size()-1] = new symbol_table();
				//symbol_stack.push_back(new symbol_table());
			}
			// printf("TOK_VARDECL val: %s\n", (left->children[0]->lexinfo)->c_str()); //%%: DUMP_SYMBOL
			symbol_stack.back()->insert(symbol_entry(const_cast<string*>(left->children[0]->lexinfo), sym));

			// if (node->childre[0]->symbol == TOK_TYPEID)
			// dump_symbol_stack(); //%%: DUMP_SYMBOL
			write_symbol(node);
			break;	
		}
		case TOK_TYPEID:
		{
			node->attr[ATTR_typeid] = 1;
			if (node->children.size() == 0) break;
			symbol *sym = lookup_struct(node);
			if (sym != NULL) 
			{
				node->children[0]->struct_name = *(node->lexinfo);
				node->children[0]->attr[ATTR_struct] = 1;
			}
			// else nothing, could be a struct
			break;
		}
		case TOK_IDENT:
		{
			symbol *sym = lookup_symbol(node);
			// symbol *str = lookup_struct(node);
			if (sym != NULL) // NOT ERROR
				node->lloc_decl = location{sym->filenr, sym->linenr, sym->offset};
			break;
		}
		case TOK_PARAM:
		{
			node->attr[ATTR_param] = 1;
			node->attr[ATTR_lval] = 1;
			break;
		}
		case TOK_FIELD:
		{
			node->attr[ATTR_field] = 1;
			break;
		}
		case TOK_STRUCT:
		{
			node->attr[ATTR_struct] = 1;
			node->children[0]->attr[ATTR_struct] = 1;
			node->children[0]->struct_name = *(node->children[0]->lexinfo);
			node->struct_name = *(node->children[0]->lexinfo);
			symbol *sym = new symbol(node);
			//check struct_table

			//look for struct in table
			auto found = struct_table.find(const_cast<string*>(node->children[0]->lexinfo));
			if (found == struct_table.end())
				struct_table.insert(symbol_entry(const_cast<string*>(node->children[0]->lexinfo), sym));

			//check for fields declarations
			if (node->children.size()+1 > 1)
			{
				sym->fields = new symbol_table;
				for (size_t i=1; i<node->children.size(); ++i)
				{
					// set type attribute
					if (!set_type_attr(node->children[i]))
						printf("ERROR field names no type!\n");

					//insert each field into the struct symbols field hash_table
					sym->fields->insert(symbol_entry
						(const_cast<string*>(node->children[i]->children[0]->lexinfo), 
							new symbol(node->children[i])));
				}
			}
			write_symbol(node);
			break;
		}
		case TOK_BLOCK:
		{
			break;
		}
		case TOK_PROTOTYPE:
		case TOK_FUNCTION:
		{
			// printf("TOK_FUNCTION start, symbol_stack.size: %lu\n", symbol_stack.size()); //%%: DUMP_SYMBOL
			symbol *look_up = lookup_symbol(node);
			if ((node->symbol == TOK_FUNCTION) &&
				(look_up != NULL)) 
				break; //ERROR
			node->attr[ATTR_function] = 1;
			astree *left = node->children[0];

			//  process return type
			if (left->symbol == TOK_TYPEID)
			{
				node->attr[ATTR_struct] = 1;
				node->struct_name = (*(left->lexinfo));
			}
			else
			{
				node->attr |= left->attr;
			}

			symbol *sym = new symbol(node);
			if (symbol_stack.back() == NULL)
			{
				symbol_stack[symbol_stack.size()-1] = new symbol_table();
				//symbol_stack.push_back(new symbol_table());
			}
			auto found = symbol_stack.back()->find(const_cast<string*>(left->children[0]->lexinfo));
			if (found == symbol_stack.back()->end()) {
				symbol_stack.back()->insert(symbol_entry(const_cast<string*>(left->children[0]->lexinfo), sym));
				sym->parameters = new vector<symbol*>();
			}

			// if (node->children.size() == 3) g_block_nr++;
			for (auto child: node->children[1]->children) 
			{
				child->attr[ATTR_param] = 1;
				child->attr[ATTR_lval] = 1;
				sym->parameters->push_back(new symbol(child));
			}
			// do we need to do anything with children[3] (block)
			// dump_symbol_stack(); //%%: DUMP_SYMBOL
			write_symbol(node);
			break;
		}
		case TOK_CALL:
		{
			symbol *func = lookup_symbol(node->children[0]);
			if (func == NULL) break; //ERROR
			if (func->parameters->size() != node->children.size()-1)
				break; //ERROR
			break;
		}
	}
}

void dump_symbol_stack()
{
	printf("BEGIN DUMPING symbol stack, size: %lu\n", symbol_stack.size());
	for (size_t i = 0; i < symbol_stack.size(); i++)
	{
		printf("entry #%d\n", (int)i);
		
		symbol_table *p = symbol_stack[i];
		if (p != NULL)
		{
			int j = 0;
			for (auto it = p->begin(); it != p->end(); ++it)
			{
				//printf("start new symbol_table entry:\n");
				string *str = it->first;
				symbol *s = it->second;
				//cout << "str: " << *str << endl;
				printf("    symbol_table[%d].str: %s ", j, str->c_str());
				dump_symbol(s);
				j++;
			}
		}
		else
		{
			printf("    NULL!\n");
		}
	}
	printf("END DUMP\n");
}

void dump_symbol(symbol *sym)
{
	printf ("(%zd.%zd.%zd) {%d}\n",
            sym->filenr, sym->linenr, sym->offset,
            (int)sym->block_nr);
}