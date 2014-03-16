//CIOTEC MARIAN_STEFAN 333CA

%{
#include <iostream>
#include <stack>
#include <vector>
#include <string.h>
#include "utils.h"
#include "nfa.h"
using namespace std;

extern "C" int yylex();
extern "C" int yyparse();
extern "C" FILE *yyin;
extern int yy_scan_string(const char *);
void yyerror(const char *s);

vector<Node*> postfix;
vector<char> my_alphabet;
vector<char> my_word;
vector<char*> states_name;
vector<char*> from, to;
vector<vector<char> > input;
char* start;
vector<char*> final;
Node *root;
%}

%union {
	char symbol_val;
	char* state_name;
}

%token OR STAR PLUS OPTIONAL
%token <symbol_val> SYMBOL
%token OA CA OP CP CA2 OT CT
%token COMA DESP
%token ENDL
%token NAME
%token LIMBAJ_VID SIR_VID

%%
line:	OA alphabet CA exp { 
			root = postfix.back();
			postfix.pop_back();
	}
	| OP states COMA OA alphabet CA2 COMA OA transitions CA2 COMA start COMA final CP
	;


alphabet: alphabet SYMBOL { my_alphabet.push_back(yylval.symbol_val); }
	  | alphabet COMA
	  | SYMBOL	{ my_alphabet.push_back(yylval.symbol_val); }
	  ;
	  
exp:	term OR exp { 
			Node *left = postfix.back();
			postfix.pop_back();
			Node *right = postfix.back();
			postfix.pop_back(); 
			postfix.push_back(new Node('|', left, right)); 
	}
	
	| term
	;
term:	factor term { 
			Node *left = postfix.back();
			postfix.pop_back();
			Node *right = postfix.back();
			postfix.pop_back(); 
			postfix.push_back(new Node('P', left, right)); 
	}
	
	| factor
	;
factor:	primary STAR {
			Node *node = postfix.back();
			postfix.pop_back();
			postfix.push_back(new Node('*', node, NULL)); 
	}
	
	| primary PLUS { 
			Node *node = postfix.back();
			postfix.pop_back();
			postfix.push_back(new Node('+', node, NULL)); 
	}
	
	| primary OPTIONAL {
			Node *node = postfix.back();
			postfix.pop_back();
			postfix.push_back(new Node('?', node, NULL)); 
	}
	
	| primary
	;
primary: OP exp CP
	 | SYMBOL {   
	 	postfix.push_back(new Node(yylval.symbol_val, NULL, NULL)); 
	 }
	 | LIMBAJ_VID {
	 	postfix.push_back(new Node('O', NULL, NULL));
	 }
	 | SIR_VID {
	 	postfix.push_back(new Node('e', NULL, NULL));
	 }
	 ;
word:	SIR_VID {
		my_word.push_back('e');
	}
	| word SYMBOL {
		my_word.push_back(yylval.symbol_val);
	}
	| SYMBOL {
		my_word.push_back(yylval.symbol_val);
	}
	;
	
states: OA state CA2
	;

state:	state NAME { states_name.push_back(strdup(yylval.state_name)); }
	| state COMA
	| NAME { states_name.push_back(strdup(yylval.state_name)); }
	;
transitions: LIMBAJ_VID
	     | relations
	     | functions
	     ;
relations: relations relation
	   | relations COMA
	   | relation
	   ;
functions: functions function
	   | functions COMA
	   | function
	   ;
relation: OP from COMA input COMA to CP
	  ;
from:	NAME { from.push_back(strdup(yylval.state_name)); }
	;
input:	word { input.push_back(my_word); my_word.clear(); }
	;
to:	NAME { to.push_back(strdup(yylval.state_name)); }
	;
start:  NAME { start = strdup(yylval.state_name); }
	;
final:  OA f_state CA2
	| LIMBAJ_VID
	;
f_state: f_state NAME { final.push_back(strdup(yylval.state_name)); }
	 | f_state COMA
	 | NAME { final.push_back(strdup(yylval.state_name)); }
	 ;
function: OT from COMA input CT to
	  ;



%%

int main(int argc, char* argv[]) {
	int i,j;
	char from_re[] = "--from-RE";
    	char from_dfa[] = "--from-DFA";
    	char from_nfa[] = "--from-NFA";
    	char to_re[] = "--to-RE";
    	char to_dfa[] = "--to-DFA";
    	char to_nfa[] = "--to-NFA";
    	char to_contains[] = "--contains";
	char input_string[1000];
	cin >> input_string;
	yy_scan_string(input_string);
    	yyparse();
    	if(equals(argv[1], from_re)) {
    		if(equals(argv[2], to_re)) {
    			NFA* nfa_aux = new NFA();
    			NFA* nfa = nfa_aux->createNew(root);
    			nfa->alphabet = my_alphabet;
    			DFA* dfa = transform_NFA_to_DFA(nfa);
    			vector<char> regex = transform_DFA_to_REGEX(dfa);
    			cout << "{";
    			cout << my_alphabet[0];
    			for(i = 1; i < my_alphabet.size(); i++) {
    				cout << "," << my_alphabet[i];
    			}
    			cout << "}:";
    			for(i = 0; i < regex.size(); i++) {
    				cout << regex[i];
    			}
    			cout << endl;
    		} 
    		else if(equals(argv[2], to_dfa)) {
    			NFA* nfa_aux = new NFA();
    			NFA* nfa = nfa_aux->createNew(root);
    			nfa->alphabet = my_alphabet;
    			DFA* dfa = transform_NFA_to_DFA(nfa);
    			dfa->show_dfa();
    		}
    		else if(equals(argv[2], to_nfa)) {
    			NFA* nfa_aux = new NFA();
    			NFA* nfa = nfa_aux->createNew(root);
    			nfa->alphabet = my_alphabet;
    			nfa->show_nfa();
    		}
    		else if(equals(argv[2], to_contains)) {
    			NFA* nfa_aux = new NFA();
    			NFA* nfa = nfa_aux->createNew(root);
    			nfa->alphabet = my_alphabet;
    			DFA* dfa = transform_NFA_to_DFA(nfa);
    			for(i = 3; i < argc; i++) {
    				string s(argv[i]);
    				if(dfa->containsWord(s)) {
    					cout << "True" << endl;
    				}
    				else {
    					cout << "False" << endl;
    				}
    			}
    		}  	
    	}
    	else if(equals(argv[1], from_dfa)) {
    		DFA* dfa = create_DFA(states_name, my_alphabet, from, input, to, start, final);
    		NFA* nfa = transformToMyNFA(states_name, my_alphabet, from, input, to, start, final);
    		if(equals(argv[2], to_re)) {
    			cout << "{";
    			cout << my_alphabet[0];
    			for(i = 1; i < my_alphabet.size(); i++) {
    				cout << "," << my_alphabet[i];
    			}
    			cout << "}:";
    			vector<char> regex = transform_DFA_to_REGEX(dfa);
    			for(i = 0; i < regex.size(); i++) {
    				cout << regex[i];
    			}
    			cout << endl;
    		} 
    		else if(equals(argv[2], to_dfa)) {
    			dfa->show_dfa();
    		}
    		else if(equals(argv[2], to_nfa)) {
    			nfa->show_nfa();
    		}
    		else if(equals(argv[2], to_contains)) {
    			for(i = 3; i < argc; i++) {
    				string s(argv[i]);
    				if(dfa->containsWord(s)) {
    					cout << "True" << endl;
    				}
    				else {
    					cout << "False" << endl;
    				}
    			}
    		}
    	}
    	else if(equals(argv[1], from_nfa)) {
    		NFA* nfa = transformToMyNFA(states_name, my_alphabet, from, input, to, start, final);
    		if(equals(argv[2], to_re)) {
    			DFA* dfa = transform_NFA_to_DFA(nfa);
    			cout << "{";
    			cout << my_alphabet[0];
    			for(i = 1; i < my_alphabet.size(); i++) {
    				cout << "," << my_alphabet[i];
    			}
    			cout << "}:";
    			vector<char> regex = transform_DFA_to_REGEX(dfa);
    			for(i = 0; i < regex.size(); i++) {
    				cout << regex[i];
    			}
    			cout << endl;
    		} 
    		else if(equals(argv[2], to_dfa)) {
    			DFA* dfa = transform_NFA_to_DFA(nfa);
    			dfa->show_dfa();
    		}
    		else if(equals(argv[2], to_nfa)) {
    			nfa->show_nfa();
    		}
    		else if(equals(argv[2], to_contains)) {
    			DFA* dfa = transform_NFA_to_DFA(nfa);
    			for(i = 3; i < argc; i++) {
    				string s(argv[i]);
    				if(dfa->containsWord(s)) {
    					cout << "True" << endl;
    				}
    				else {
    					cout << "False" << endl;
    				}
    			}
    		}
    	}	
	return 0;
}

void yyerror(const char *s) {
	cout << "Parse error!  Message: " << s << endl;
	exit(-1);
}

	


