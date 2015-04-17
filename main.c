#include "shell.h"

#include "alias_functions.c"
#include "cd_functions.c"
#include "env_functions.c"
#include "execute_function.c"

main(int argc, char* argv) {
	shell_init();
	// loops till bye
	while(1) {
		// prepare parser
		wordArray[0] = strdup("");
		// header
		lineHeaderPath();
		switch(getCommand()) {
			case ERRORS:
				printf("You did something wrong.\n");
				break;
			case OK:
				processCommand();
				break;
			default: 
				printf("uhhh...whoops.\n");				
				break;
		}
		resetGlobals();
	}
}

/* used to set up the globals and such */
void shell_init() {
	// black background	
	printf("[01;32;40m" "\n****************************** SHELL STARTS HERE ******************************\n");
	printf("[00;32;40m" "");

	wordArray = (char**) malloc(500 * sizeof(char*));
	wordArray[0] = strdup("");

	// set the variables
	aliasCount = 0;
	wordCount = 0; 
	addedWords = 0;
	builtin_type = 0;
	cdPath = malloc(500 * sizeof(char));
	variable = malloc(500 * sizeof(char));
	word = malloc(500 * sizeof(char));
	
	HOME = malloc(500 * sizeof(char));
	strcpy(HOME, getenv("HOME")); //get home directory so that it stays constant
	
	// prevents this god from crashes
	signal(SIGINT, SIG_IGN); 
	signal(SIGTSTP, SIG_IGN); 
	signal(SIGQUIT, SIG_IGN); 
	int i;
	for (i = 0; i < SIGRTMAX; ++i) { signal(i, SIG_IGN); }

	// start in home
	cd_home_function();
}

/* called after every iteration to reset all variables */
void resetGlobals() {
	wordCount = 0;
	addedWords = 0;
	memset(wordArray, 0, sizeof(wordArray)); 
	free(commands);
	// reset to 0
	builtin_type = 0;
}

/* runs the lexx and yac */
int getCommand() {
	// error
	if(yyparse()) 
		return ERRORS;
	// ok
	else 
		return OK;
}

/* calls appropriate functions */
void processCommand() {
	if(builtin_type > 0)
		do_it();
	// take care of all possible cases (redirections and pipes)
	else
		execute();
}

/* for built-in */
void do_it() {
	switch (builtin_type) {
		case ALIAS_DEF:
			alias_function(variable, word);
			break;
		case ALIASPRINT_DEF:
			alias_print_function();
			break;
		case CDHOME_DEF:
			cd_home_function();
			break;
		case CDPATH_DEF:
			cd_function(cdPath);
			break;
		case UNALIAS_DEF:
			unalias_function(variable);
			break;
		case SETENV_DEF:
			setenv_function(variable, word);	
			break;
		case UNSETENV_DEF:
			unsetenv_function(variable);
			break;
		case PRINTENV_DEF: 
			printenv_function();
			break;
		default:
			break;
	}
}

/* adds new entry to word table */
void insertToWordTable(char *text) {
	char * word = strdup(text);
	char **tempWordArray = (char **) malloc((wordCount+2)*sizeof(char *)); 

	memcpy ((char *) tempWordArray, (char *) wordArray, wordCount*sizeof(char *)); 
	tempWordArray[wordCount]   = word; //word
	tempWordArray[++wordCount] = NULL; //null entry
	wordArray = tempWordArray;
}

/* get matching directories */
char* getDirectories(char* matchString) {
	int i;
	int flags = 0;

	// A type representing a directory stream.
	DIR *dir;
	// array of the files
	struct dirent* ent;

	glob_t *results = malloc(10 * sizeof(glob_t));

	if((dir = opendir(getenv("PWD"))) == NULL) {
		perror ("Cannot open directory");
		printf("Error at line %d\n", __LINE__);
		return "";
	} else {
		// get directory stream
		if((ent = readdir(dir)) != NULL) {
			flags |= (i > 1 ? GLOB_APPEND : 0);
			if (glob(matchString, flags, globerr, results) != 0) {//error
				printf("Error with glob\n");
				printf("Error at line %d\n", __LINE__);
				return "";
			}
		}
		int size = 0;
		// for amount of paths matched
		for(i = 0; i < results->gl_pathc; ++i) 
			size += strlen(results->gl_pathv[i]) + 1;
		
		char* result = malloc(size * sizeof(char));
	
		strcpy(result, "");
		for(i = 0; i < results->gl_pathc; i++) {
			strcat(result, results->gl_pathv[i]);
			strcat(result, "[^");
		}
		result[strlen(result) - 1] = '\0'; //null terminate
		globfree(results); //free glob expression
		closedir(dir); //close directory 
		return result;
	}
	return "";
}
int globerr(const char *path, int eerrno) {
	perror ("Error with glob\n");
	printf ("Error with path %s at line %d\n", path, __LINE__);
	return 0;	
}


/* get word between quotes */
void quoteFunction(char* text) {
	condense(text, ' ');
	char* betweenQuotes = malloc(300 * sizeof(char));

	strncpy(betweenQuotes, &text[1], strlen(text) - 2); 
	char* result = malloc(300 * sizeof(char));

	int index = 0;
	int i;
	int* results = malloc(300 * sizeof(int));
	int resultCount = 0;
	int openBrace = 0;
	int closeBrace = 0;
	
	// Checks for environment variable expansion
	for(i = 0; i < strlen(betweenQuotes); ++i) {
		if(betweenQuotes[i] == '$' && betweenQuotes[i + 1] == '{') { 
			index = i;
			results[resultCount++] = index;			
			++openBrace;
		}
		if(betweenQuotes[i] == '}') {//closer
			index = i;
			results[resultCount++] = index;
			++closeBrace;
		}
		if(closeBrace > openBrace) { 
			perror ("Input error..");
			printf ("Error at line %d\n", __LINE__);
			return;
		}
	}
	if(openBrace != closeBrace) {
		perror ("Syntax error");
		printf ("Error at line %d\n", __LINE__);
		return;
	}
	char *result2 = malloc(300 * sizeof(char));

	// no openBrace or closeBrace
	if(resultCount == 0) {
		insertToWordTable(betweenQuotes);
	}
	else {
		strcpy(result2, "");
		char* result3 = malloc(300 * sizeof(char));
	
		for(i = 0; i < resultCount; ++i) {
			if(i == 0) {//first open
				strncat(result2, &betweenQuotes[0], results[0]); //add the beginning
				memcpy(result3, &betweenQuotes[results[0] + 2], (results[1] - results[0] - 2) * sizeof(char));
				if(getenv(result3) == NULL) {//invalid environment variable
					perror ("Invalid environment variable.");
					printf ("Error at line %d\n", __LINE__);
					return;
				}
				strcpy(result3, getenv(result3));
				strcat(result2, result3);
				memset(result3, 0, sizeof(result3)); //clear buffer
			}
			else if(i % 2 == 0 && i != 0) {//other openBrace
				strncat(result2, &betweenQuotes[results[i - 1] + 1], results[i] - results[i - 1] - 1);
				strncpy(result3, &betweenQuotes[results[i] + 2], (results[i + 1] - results[i] - 2) * sizeof(char));
				if(getenv(result3) == NULL) {
					perror ("Invalid environment variable.");
					printf ("Error at line %d\n", __LINE__);
					return;
				}
				strcpy(result3,getenv(result3));
				strcat(result2, result3);
				memset(result3, 0, sizeof(result3)); //clear buffer
			}
			else {
				//do nothing
			}
		}
		// more
		if(results[resultCount - 1] != strlen(betweenQuotes) - 1) {
			strcat(result2, &betweenQuotes[results[resultCount - 1] + 1]); //add all the leftovers
			insertToWordTable(result2);
		}
		// done
		else {
			insertToWordTable(result2);
		}
	}
}

/* processes the text passed and calls insert */
void yytextProcessor(char* text) {
	char* result = malloc(300 * sizeof(char));
	
	char* string = strtok(text, ":"); 
	strcpy(result, "");
	while(string != NULL) {
		char* copy = strdup(tildeExpansion(string));
		strcat(result, copy);
		strcat(result, ":"); 
		string = strtok(NULL, ":");
	}
	
	result[strlen(result) - 1] = '\0'; //remove last colon
	insertToWordTable(result);
}

/* expands words with ~ */
char* tildeExpansion(char* text) {
	if(strncmp(text, "~", 1) == 0) {//tilde expansion
		int length = strlen(&text[1]); 
		if(length == 0) {		// single tilde
			int result = chdir(HOME); //get home directory and move to it
			if(result == -1) { //error
				perror("Directory not changed");
				printf("Error at line %d\n", __LINE__);
				return;
			}
			return HOME;
		}
		else { //actual expansion
			char *result = strchr(&text[1], '/');
			if (result == NULL) {//end of string, so must be username
				pwd = getpwnam(&text[1]); //gets user info
				if (pwd == NULL) {//error
					perror("Error retrieving struct.");
					printf("Error at line %d\n", __LINE__);
					return;
				}
				int result = chdir(pwd->pw_dir); //get home directory and move to it
				if(result == -1) {//error
					perror("Directory not changed");
					printf("Error at line %d\n", __LINE__);
					return;
				}
				return pwd->pw_dir; //username
			}
			else {//string continues
				char *directory = malloc(300 * sizeof(char));
				strcpy(directory, HOME); //start with home directory
				int index = length - 1;
				int i;
				for(i = 0; i < length; ++i)	{
					if(text[i] == '/') {//find slash
						index = i;
						break;
					}
				}
				char* appender = strdup(&text[index + 1]);
				
				strcat(directory, "/"); //add slash
				strcat(directory, appender); //append
				return directory;
			}
		}
	}
	else {//no tilde
		return text;
	}
}

/* inserts the text into the word array */
void cardsGoneWild(char* text, int position) {
	char* saveptr;
	char* result = strdup(text);
	char* result2 = strdup(text);
	char* filesList = strtok_r(result, "[^", &saveptr); //parse to get each indiviual file
	
	// how many positions we add
	int tokens = 0; 
	while(filesList != NULL) {
		++tokens;
		filesList = strtok_r(NULL, "[^", &saveptr);
	}

	//  create temp table
	char **tempWordArray = (char **) malloc((wordCount+tokens)*sizeof(char *)); 

	// copy all entries from 0 to position of wordArray into tempWordArray
	memcpy ((char *) tempWordArray, (char *) wordArray, position*sizeof(char *)); 

	char** textForLater = malloc((wordCount - position) * sizeof(char *)); // text we add at the end of the wordArray

	int i;
	int index = 0;
	for(i = position + 1; i < wordCount; ++i) {
		textForLater[index] = malloc((strlen(wordArray[i]) + 1) * sizeof(char)); // allocate enough space for entry

		strcpy(textForLater[index], wordArray[i]); //copy entry into array
		++index;
	}

	char* saveptr2;
	char* filesList2 = strtok_r(result2, "[^", &saveptr2);
	int j = 0;
	--wordCount; //since we are overwriting an entry, need to decrement wordCount beforehand
	while(filesList2 != NULL) {
		char* es = malloc(strlen(filesList2) + 1); //allocate space for word and terminating character
		strcpy(es, filesList2); //copy text into pointer
		tempWordArray[position + j] = es; //word
		++j; //move forward
		++wordCount; //added another word
		filesList2 = strtok_r(NULL, "[^", &saveptr2);
	}
	int k;
	index = 0;
	for(k = position + j; k < wordCount; ++k) {
		tempWordArray[k] = malloc((strlen(textForLater[index]) + 1)*sizeof(char)); //allocate space

		strcpy(tempWordArray[k], textForLater[index++]); //copy over
	}
	tempWordArray[wordCount + 1] = NULL; //null entry
	wordArray = tempWordArray;
	addedWords += j - 1; //how many wordCount we added
}

/* take everything between ${ and } */
void processEnvironmentVariable(char* yyText) {
	char* insideText = malloc(300 * sizeof(char));
	strncpy(insideText, &yyText[2], strlen(yyText) - 3); 
	char* result = malloc(300 * sizeof(char));
	if(getenv(insideText) == NULL) //invalid environment variable
	{
		perror ("Invalid environment variable.");
		printf ("Error at line %d\n", __LINE__);
	}
	else
	{
		insertToWordTable( getenv(insideText));
	}	
}
/* prints the prompt */
void lineHeaderPath() {
	//printf("\n");
	// get user
	struct passwd *passwd;
	passwd = getpwuid ( getuid()); 
	printf("[1;34;40m" "%s@", passwd->pw_name);	

	// get computer name
	char* hostName = malloc(300 * sizeof(char));
	gethostname(hostName, 20);
	printf("[1;34;40m" "%s:", hostName);

	// get path
	printf("[1;35;40m" "%s$ ", getenv("PWD"));

	// back to normal
	printf("[00;00;40m");
}

/* condenses particular character */
void condense(char* string, char toCondense) { 
	int i = 0;
	int size = strlen(string);
	while ( i < size ){
		if(string[i] == toCondense && string[i+1] == toCondense){
			int j = i;
			while ( j <=size ){
				string[j] = string[++j];
			}
			--size;
		}
		else
			++i;
	}
}

int getWords() {
	return wordCount;
}

