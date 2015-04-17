%{
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "y.tab.h"
%}
%%
cd 			return CD;
printenv 	return PRINTENV;
unsetenv 	return UNSETENV;
setenv 		return SETENV;
alias 		return ALIAS;
unalias 	return UNALIAS;
bye 		return BYE;
\n 			return NEWLINE;


[-*?@!#%',=~_.:/:A-Za-z0-9]+ 						return WORD;
"\""[-*? \\<>&\|${},_\"@!#%'=~.:/:A-Za-z0-9]+"\"" 	return QUOTES;
"${"[-*?@!#%,'=~._:/:AAN-Za-z0-9]+"}" 				return ENVIRONMENTVARIABLE; 

[ \t]+ 

"<" 	return READFROM;
">" 	return WRITETO;
"|" 	return PIPE;
"&" 	return AMPERSAND;
">>"	return APPEND;

"2>"[*?@!#%',=~.:/:A-Za-z0-9]+ 	return STANDARDERRORFILE;
"2>&1" 							return STANDARDERROROUTPUT;
. {yyerror("Unrecognized character");}
%%
