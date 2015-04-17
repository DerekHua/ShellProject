/* used for the just "cd" command */
void cd_home_function() {
	// change to home currentDirectory	
	// error
	if(chdir(HOME) == -1) { 				
		printf("Directory not changed. %d\n", __LINE__);
		printf("Error at line %d\n", __LINE__);
		return;
	}
	// change the value of the PWD variable
	setenv_function("PWD", HOME); 
}


/* normal cd function "cd word" */
void cd_function(char *inputDirectory){

	condense(inputDirectory, ' '); 

	char *currentDirectory = malloc(300 * sizeof(char));

	// get current directory
	strcpy(currentDirectory, getenv("PWD")); 

	// if last character is not a slash then adds a slash
	if(currentDirectory[strlen(currentDirectory) - 1] != '/') { 
		strcat(currentDirectory, "/"); 
	}

	// input starts with '.'
	if(inputDirectory[0] == '.') {
		// just dot || dot-slash || ".." and current dir is '/'
		if((strlen(inputDirectory) == 1) || (strlen(inputDirectory) == 2 && inputDirectory[1] == '/') 
			|| (inputDirectory[1] == '.' && strcmp(currentDirectory, "/") == 0)) {
			return;
		}

		// dot slash with more
		else if(strlen(inputDirectory) > 2 && inputDirectory[1] == '/') {
			// remove the "./"
			strcpy(inputDirectory, &inputDirectory[2]);
		}

		// // '.' then something
		// else if(inputDirectory[1] != '.') {
		// 	strcpy(inputDirectory, &inputDirectory[1]);
		// }
 
		// starting with ".."
		else if(inputDirectory[1] == '.' && strcmp(currentDirectory, "/") != 0) {
			int i;
			int slashIndex = 1;
			// -2 to start at the last char, since we added a '/' in the end
			for(i = strlen(currentDirectory) - 2; i != -1; --i) {
				if(currentDirectory[i] == '/') {
					slashIndex = i; 
					break;
				}
			}

			// changing currentDirectory path

			// pushes currentDirectory up a level
			if(slashIndex != 0) { 
				// moves null char to previous slash
				currentDirectory[slashIndex] = '\0'; 
			}

			// returning up to the root 
			else if(slashIndex == 0) {
				currentDirectory[1] = '\0';
			}

			// if more than just ".."
			if(strlen(inputDirectory) > 2) {
				// add a slash to currentDirectory
				strcat(currentDirectory, "/"); 
				// remove the "../"
				strcpy(inputDirectory, &inputDirectory[3]); 
			}
			else { 
				strcpy(inputDirectory, ""); 
			}
		}

		// if currentDirectory is root
		else if(strcmp(currentDirectory, "/") == 0) {
			strcpy(inputDirectory, ""); 
		}
		
	} // end if(inputDirectory[0] == '.')


	// first character is slash (starting at root)
	else if(inputDirectory[0] == '/') {

		// just '/' or "/."
		if(strlen(inputDirectory) == 1 || (strlen(inputDirectory) == 2 && inputDirectory[1] == '.')) {
			// go to root
			if(chdir("/") == -1) {
				perror("Directory not changed");
				printf("Error at line %d\n", __LINE__);
				return;
			}
			// change variable
			setenv_function("PWD", "/"); 
			return;
		} else {			
			strcpy(currentDirectory, "");
		}
	}

	// combines current with the input
	// check if relative
	strcat(currentDirectory, inputDirectory); 
	
	//error check absolute path
	if (chdir(currentDirectory) == -1) { 
		if (chdir(inputDirectory) == -1) {
			perror("Directory not changed");
			printf("Error at line %d\n", __LINE__);
			return;
		}
		// absolute path
		setenv_function("PWD", inputDirectory); 
		return;
	}

	// relative path
	// last character is a slash and currentDirectory isn't "/"
	if(strncmp(&currentDirectory[strlen(currentDirectory) - 1], "/", 1) == 0 && strlen(currentDirectory) != 1) {
		currentDirectory[strlen(currentDirectory) - 1] = '\0'; // remove slash
		setenv_function("PWD", currentDirectory); 
	} else {
		setenv_function("PWD", currentDirectory); 
	}
}





