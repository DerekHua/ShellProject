
/* This alias command adds a new alias to the shell. An alias is
essentially a shorthand form of a long command. */
void alias_function(char *name, char *word) {

	// remove current assignment for the name
	unalias_function(name);          

	struct aliasStruct pair;
	pair.name = strdup(name);
	pair.word = strdup(word);

	struct aliasStruct* aliasesTemp = malloc((aliasCount+2) * sizeof(struct aliasStruct)); 

	int i;
	for(i = 0; i < aliasCount; ++i) {
		aliasesTemp[i] = aliases[i];
	}
	aliasesTemp[aliasCount] = pair; 
	// aliasesTemp[aliasCount + 1] = NULL; 

	free(aliases);
	aliases = aliasesTemp;
	++aliasCount; 	
}

/*  The unalias command is used to remove the alias for name from the alias list. */
void unalias_function(char *name) {
	
	size_t length;
	if (name == NULL || name == '\0' || strchr(name, '=') != NULL) { 
		perror("Entered an invalid alias");
		printf("Error at line %d\n", __LINE__);
		return;
	}

	length = strlen(name);
	int i;
	int j;
	for (i = 0; i < aliasCount; ++i) {
		// check for match
		if (strncmp((aliases[i]).name, name, length) == 0) { 
			for (j = i; j < aliasCount; ++j) {
				// shift to left
				aliases[j] = aliases[j + 1]; 
			}

			// decrement 
			--aliasCount; 
		} 
	}
}



/* Gets the value of an alias when given the name returns empty string if the alias name does not exist */
int getAliasValue(char* aliasName, char* returnValue) {
	
	int aliasNameLength = strlen(aliasName);

	int i;
	for(i = 0; i < aliasCount; ++i) {
		// find match
		if(strncmp((aliases[i]).name, aliasName, aliasNameLength) == 0) {
			strcpy(returnValue, (aliases[i]).word);
			return 0;
		}
	}
	return -1;
}



/* takes in name of alias and returns the final resolved value of that alias name or INFIN if it loops infinitely */
char* aliasResolver(char* alias) {
	char* name = malloc(500*sizeof(char)); 
	strcpy(name, alias);

	// keeps track of alias names already encountered 
	char* aliasTracker[100]; 
	int nestedSize = 0; //keeps track of size of names in the tracker
	aliasTracker[nestedSize++] = name;

	// first value
	char* value = malloc(500*sizeof(char));
	if(getAliasValue(name, value) == -1) {
		//perror("Error alias not found.\n");
		//printf("Error at line %d\n", __LINE__);
		free(name);
		return "";
	}

	// loop to find nested 
	while(1) {
		strcpy(name, value);
		if(getAliasValue(name, value) == -1) {
			free(name);
			return value;
		} 

		aliasTracker[nestedSize++] = name;

		int i;
		for(i = 0; i < nestedSize; ++i) {
			// returns "INFIN" if the alias generates an infinite loop 
			if(strcmp(value, aliasTracker[i]) == 0) { 
				strcpy(value, "INFIN");
				free(name);
				return value;
			}
		}
	}
}


void wordArrayAliasExpansion(char* name, int position) {
	
	char* saved3;
	char* result = malloc((strlen(name) + 1) * sizeof(char)); //allocate space

	strcpy(result, name); //copy name over
	char* result2 = malloc((strlen(name) + 1) * sizeof(char));

	strcpy(result2, result); //copy another one since strtok_r changes actual name
	char* pch = strtok_r(result, " ", &saved3); //parse to get each indiviual file
	int tokens = 0; //how many positions we add
	while(pch != NULL)
	{
		tokens++;
		pch = strtok_r(NULL, " ", &saved3);
	}
	char** tempWordArray = (char **) malloc((wordCount+tokens)*sizeof(char *)); //null entry and new wordCount

	memcpy ((char *) tempWordArray, (char *) wordArray, position*sizeof(char *)); //copy all entries from 0 to position of wordArray into tempWordArray
	char** textForLater = malloc((wordCount - position) * sizeof(char *)); //name we add at the end of the wordArray

	int i;
	int index = 0;
	for(i = position + 1; i < wordCount; ++i)
	{
		textForLater[index] = malloc((strlen(wordArray[i]) + 1) * sizeof(char)); //allocate enough space for entry
		
		strcpy(textForLater[index], wordArray[i]); //copy entry into array
		++index;
	}
	char* saved4;
	char* pch2 = strtok_r(result2, " ", &saved4);
	int j = 0;
	--wordCount; //since we are overwriting an entry, need to decrement wordCount beforehand
	while(pch2 != NULL)
	{
		char* pair;
		pair = malloc(strlen(pch2) + 1); //allocate space for word and terminating character
		
		strcpy(pair, pch2); //copy name into pointer
		tempWordArray[position + j] = pair; //word
		++j; //move forward
		++wordCount; //added another word
		pch2 = strtok_r(NULL, " ", &saved4);
	}
	int k;
	index = 0;
	for(k = position + j; k < wordCount; ++k)
	{
		tempWordArray[k] = malloc((strlen(textForLater[index]) + 1)*sizeof(char)); //allocate space
		
		strcpy(tempWordArray[k], textForLater[index++]); //copy over
	}
	tempWordArray[wordCount + 1] = NULL; //null entry
	wordArray = tempWordArray;
	addedWords += j - 1; //how many wordCount we added
}


/* print each alias line by line */
void alias_print_function() {
	int i;
	for(i = 0; i < aliasCount; ++i) {
		printf("%s=%s\n", aliases[i].name, aliases[i].word); 

	}
}

