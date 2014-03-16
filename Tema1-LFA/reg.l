%{
#include <iostream>
#include "reg.tab.h"
using namespace std;
#define YY_DECL extern "C" int yylex()
%}

%%
([0-9])|([a-df-z])|("!"|"#"|"$"|"%"|"&"|"-"|"."|"/"|":"|";"|"<"|">"|"="|"@"|"["|"]"|"ˆ"|"‘"|" ̃") {
	yylval.symbol_val = *yytext;
	return SYMBOL;
}

([A-NP-Z]|"_")([a-z]|[0-9]|"_")* {
	yylval.state_name = yytext;
	return NAME;
}

"O" return LIMBAJ_VID;

"e" return SIR_VID;

"{" return OA;
"}:" return CA;
"}" return CA2;

"d(" return OT;
")=" return CT;

"(" return OP;
")" return CP;

"|" return OR;
"*" return STAR;
"+" return PLUS;
"?" return OPTIONAL;

"," return COMA;

"\n" return ENDL;
[ \t] {/* ignore whitespace */} 
