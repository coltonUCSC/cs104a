#include "symstack.h"
#include "lyutils.h"
#include <iostream>
#include <stack>
using namespace std;

//vector of unordered_maps<pair((single)string*, symbol*)>
vector<symbol_table*> symbol_stack;
symbol_table struct_table;
vector<int> blockStack; 

int g_block_nr = 0;
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
	blockStack.push_back(0);
	symbol_stack.push_back(new symbol_table());// TODO change to NULL then fix segfaults 
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
		blockStack.push_back(g_block_nr);
	}
	else if (node->symbol == TOK_BLOCK)
	{
		++g_block_nr;
		blockStack.push_back(g_block_nr);
		symbol_stack.push_back(new symbol_table());// TODO change to NULL then fix segfaults 
		node->block_nr = g_block_nr;
	}
	else if ((node->symbol == TOK_FUNCTION) || (node->symbol == TOK_PROTOTYPE))
	{
		node->block_nr = 0;
		symbol_stack.push_back(new symbol_table());// TODO change to NULL then fix segfaults
	}
	else
	{
		node->block_nr = blockStack.back();
	}
}

void check_leave_block(astree *node)
{
	if (node->symbol == TOK_PARAM)
	{
		// special case, restore block number
		blockStack.pop_back();
		--g_block_nr;
	}
	else if(node->symbol == TOK_BLOCK)
	{
		blockStack.pop_back();
		symbol_stack.pop_back();
	}
	else if ((node->symbol == TOK_FUNCTION) || (node->symbol == TOK_PROTOTYPE))
	{
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
}

// bool lookup_symbol(astree *node)
// {
// 	for (int i=g_block_nr; i>=0; --i)
// 	{
// 		auto found = symbol_stack[i]->find(node->lexinfo);
// 		if (found != symbol_stack[i]->end()) return true;
// 	}
// 	return false;
// }

void processNode(astree* node) 
{
	//node->block_nr = g_block_nr;
	switch(node->symbol)
	{
		case TOK_DECLID:
		{
			break;
		}
		case TOK_INT:
		{
			astree *left = node->children[0];
			if (!left) break;
			node->attr[ATTR_int] = 1; 
			left->attr[ATTR_int] = 1;
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
			symbol_stack.back()->insert(symbol_entry(const_cast<string*>(left->children[0]->lexinfo), sym));
			break;	
		}
		case TOK_TYPEID:
		{
			node->attr[ATTR_typeid] = 1;
			break;
		}
		case TOK_IDENT:
		{
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
					//insert each field into the struct symbols field hash_table
					sym->fields->insert(symbol_entry
						(const_cast<string*>(node->children[i]->children[0]->lexinfo), 
							new symbol(node->children[i])));
				}
			}
		}
		case TOK_BLOCK:
		{
			break;
		}
		case TOK_PROTOTYPE:
		case TOK_FUNCTION:
		{
			node->attr[ATTR_function] = 1;
			astree *left = node->children[0];
			symbol *sym = new symbol(node);

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

			//function is defined
			if (node->children.size() > 2)
			{
				//???
				//do a look up local->global 
			}
			break;
		}
	}
}
