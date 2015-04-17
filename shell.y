%{
#include "shell.h"
extern char *yytext;

void yyerror(const char *str) {
	fprintf(stderr,"error: %s\n", str);
}

int yywrap() {
	return 1;
}

%}
%token CD PRINTENV UNSETENV SETENV NEWLINE ALIAS UNALIAS BYE WORD QUOTES ENVIRONMENTVARIABLE READFROM WRITETO PIPE AMPERSAND APPEND STANDARDERRORFILE STANDARDERROROUTPUT 
%%

// some recursion
begin: 
		| builtin_commands NEWLINE {
			return 0;
		}
		| execute_commands begin {
			return 0;
		}
		| NEWLINE {
			return 0;
		};

// these cases make builtin_type > 0
builtin_commands:
		cd_case
		|env_case
		|alias_case
		|bye_case;
		
cd_case:
		CD {
			builtin_type = CDHOME_DEF;
		}
	    |
		CD word_case {
			builtin_type = CDPATH_DEF;	
			strcpy(cdPath, wordArray[wordCount - 1]);
		};
env_case:
	    PRINTENV {
			builtin_type = PRINTENV_DEF;
		}
		| 
		UNSETENV word_case {
			builtin_type = UNSETENV_DEF;
			variable = strdup(wordArray[wordCount - 1]);
		}   
		|
		SETENV word_case word_case {
			builtin_type = SETENV_DEF;
			variable = strdup(wordArray[wordCount - 2]);
			word = strdup(wordArray[wordCount - 1]);
		};
alias_case:
		ALIAS {
			builtin_type = ALIASPRINT_DEF;
		}
		|
		ALIAS  word_case  word_case {
			builtin_type = ALIAS_DEF;
			variable = strdup(wordArray[wordCount - 2]);
			word = strdup(wordArray[wordCount - 1]);
		}
		|
		UNALIAS word_case {
			builtin_type = UNALIAS_DEF;
			variable = strdup(wordArray[wordCount - 1]);
		};		
bye_case:
		BYE { 
			printf("[01;31;40m" "\n******************************  SHELL ENDS HERE  *******************************\n\n");
			printf("[00;00;00m");
			exit(0); //exit shell
		};



execute_commands:
		ampersand_case
		|standard_error_redirect_case
		|standard_error_redirect_case2
		|error_case        
		|word_case
		|pipe_case
		|read_from_case
		|write_to_case
		|append_case;


read_from_case:
		READFROM {
			insertToWordTable("<");
		};
write_to_case:
		WRITETO{
			insertToWordTable(">");			
		};
pipe_case:
		PIPE {
			insertToWordTable("|");
		};
ampersand_case:
		AMPERSAND {
			insertToWordTable("&");			
		};
standard_error_redirect_case:
		STANDARDERRORFILE {
			insertToWordTable("2>&1");			
		};

standard_error_redirect_case2:
		STANDARDERROROUTPUT {
			insertToWordTable(yytext);			
		};
error_case:
		error {
			//printf ("Syntax error.\n");			
		};
append_case:
		APPEND {
			insertToWordTable(">>");			
		};
		
word_case:
		WORD {
			yytextProcessor(yytext);
		}
		|		
		ENVIRONMENTVARIABLE {
			processEnvironmentVariable(yytext);
		}
		|	
		QUOTES {
			quoteFunction(yytext);
		};

