// Alain Ambrose
// Maxwell Kuftic
// Systems Software Spring 2023

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

typedef enum{
	skipsym = 1, identsym = 2, numbersym = 3, plussym = 4, minussym = 5,  
multsym = 6,  slashsym = 7, oddsym = 8,  eqlsym = 9, neqsym = 10, lessym = 11, 
leqsym = 12, gtrsym = 13, geqsym = 14, lparentsym = 15, rparentsym = 16, 
commasym = 17, semicolonsym = 18, periodsym = 19, becomessym = 20,  
beginsym = 21, endsym = 22, ifsym = 23, thensym = 24, whilesym = 25, dosym = 26, 
callsym = 27, constsym = 28, varsym = 29, procsym = 30, writesym = 31,  
readsym = 32, elsesym = 33
}token_types;

typedef struct{ 
	char opCode[3];
	int l;
	int m;
} assemblyCode; 

typedef struct{ 
	int kind; // const = 1, var = 2, proc = 3
	char name[10]; // name up to 11 chars 
	int val; // number (ASCII value) 
	int level; // L level
	int addr; // M address
	int mark; // to indicate unavailable or deleted
} symbol; 

// function definitions
int isSymbol(char c);
int isValid(char c);
int inTable(char c);
int symbolTableCheckLO(char* str);
int symbolTableCheck(char* str, int level);
void program();
void block();
void constDeclaration();
int varDeclaration();
void statement();
void condition();
void expression();
void term();
void factor();
void emit(char* opCode, int l, int m);
void getNextToken();
void procDeclaration();
void markSymbols(int level);

// Global variables
char token[500] = " ";
symbol symbolTable[500];
assemblyCode codes[500];
char **lexArray;
int lexIndex = 0;
int lexArrayIndex = 0;
int symbolTableIndex = 0;
int assemblyCodeIndex = 0;
int level = 0;
int noError = 1;

int main(int argc, char **argv)
{
	lexArray = (char**) malloc((sizeof(char*) * 500));

	for(int i = 0; i < 500; i++)
	{
		lexArray[i] = (char*) malloc((sizeof(char) * 500));
	}


	FILE *inputFile = fopen(argv[1], "r");
	FILE *outputFile = fopen("output.txt", "w");
	char *keywords[14] = {"read", "write", "const", "var", "procedure", "call", "begin", "end", "if", "then", "else", "while", "do", "odd"};
	char *symbols[16] = {"+", "-", "*", "/", "=", "<>", "<", "<=", ">", ">=", "(", ")", ",", ";", ".", ":="};
	char inputArray[500]; // array that stores all of the input
	char charArray[500]; // array that stores all of the valid input
	int inputSize = 0;

	char c;
	printf("Source Program:\n\n");
	//prints the entire input file onto the console
	while(1) {
      c = fgetc(inputFile);
      if(feof(inputFile)) {
         break;
      }
      inputArray[inputSize] = c;
      inputSize++;
      printf("%c", c);
    }
    printf("\n\n");
    rewind(inputFile);
    int i, j;
    for(i = 0, j = 0;i < inputSize;i++)
    {
    	//removes non-valid characters from the charArray
		if(!inTable(inputArray[i]))
		{
			continue;
		}
    	//If both the current index and next index are symbols, 
    	//add both symbols to the char array
		else if(isSymbol(inputArray[i]) && isSymbol(inputArray[i+1]))
		{
      		//skips comments
			if(inputArray[i] == '/' && inputArray[i+1] == '*')
			{
				int returnValue = i;
        		//if we eneter a comment, searches for a closing sequence
				while(!(inputArray[i] == '*' && inputArray[i+1] == '/'))
				{
					if(i != strlen(inputArray))
					{
						i++;
					}
					else
					{
						printf("ERROR: Unclosed comment.\n");
						noError = 0;
						exit(1);
						break;
					}
				}
				i+=2;
			}
			else if(inputArray[i] == '<' && inputArray[i+1] == '>')//Adds the <> symbol to the array if found
			{
				charArray[j] = inputArray[i];
				charArray[j+1] = inputArray[i+1];
				charArray[j+2] = '|';
				j+=3;
				i+=1;
			}
			else if(inputArray[i] == '<' && inputArray[i+1] == '=')//Adds that <= symbol to the array if found
			{
				charArray[j] = inputArray[i];
				charArray[j+1] = inputArray[i+1];
				charArray[j+2] = '|';
				j+=3;
				i+=1;
			}
			else if(inputArray[i] == '>' && inputArray[i+1] == '=')//Adds the >= symbol to the array if found
			{
				charArray[j] = inputArray[i];
				charArray[j+1] = inputArray[i+1];
				charArray[j+2] = '|';
				j+=3;
				i+=1;
			}
			else if(inputArray[i] == ':' && inputArray[i+1] == '=')//Adds the := symbol to the charArray if found
			{
				charArray[j] = inputArray[i];
				charArray[j+1] = inputArray[i+1];
				charArray[j+2] = '|';
				j+=3;
				i+=1;
			}

		}
		else if(isSymbol(inputArray[i])) //Adds the symbol (if found) to charArray and a line afterwards.
		{
			charArray[j] = inputArray[i];
			charArray[j+1] = '|';
			j+=2;
		}
		else if(isValid(inputArray[i]) && isSymbol(inputArray[i+1])) //Splits up characters followed by symbols
		{
			charArray[j] = inputArray[i];
			charArray[j+1] = '|';
			j+=2;
		}
		else if(isValid(inputArray[i]) && isValid(inputArray[i+1])) //Verifies the next value can be input
		{
			if(i == inputSize-1 || !inTable(inputArray[i+1]))
			{
				charArray[j] = inputArray[i];
				charArray[j+1] = '|';
				j+=2;
			}
			else
			{
				charArray[j] = inputArray[i];
				j++;
			}
		}
		else if(isValid(inputArray[i]) && !isValid(inputArray[i+1])) //Splits valid characters up from invlaid characters
		{
			charArray[j] = inputArray[i];
			charArray[j+1] = '|';
			j+=2;
		}
   }

	for(int i = 0;inTable(charArray[i]);i++)
	{
		//Group of booleans that classifies what kind of word the
		//tempWord variable is
		int isNumber = 0;
		int isKeyword = 0;
		int isSymbol = 0;

		char tempWord[500] = "";
		int k = 0;
		int j = 0;
		//Loops through the charArray until it finds a bar and stores
		//the word into a temporary variable
		for(j = i, k = 0;(charArray[j] != '|' && inTable(charArray[j]));j++, k++)
		{
			tempWord[k] = charArray[j];
		}
		//printf("%d ", i);
		i = j;
		tempWord[k] = '\0';

		//checks to see if the first letter of our tempWord is a digit
		//if it is it makes sure that the entire thing is digits
		//if not we have to throw an error somewhere
		if(isdigit(tempWord[0]))
		{
			int flag = 0;
			//This loop checks to see if the entire "word" is comprised of numbers
			for(int j = 0; j < strlen(tempWord);j++)
			{
				//sets off a flag if any following characters are not digits
				if(!isdigit(tempWord[j]))
				{
					flag = 1;
				}
			}
			if(flag == 0 && strlen(tempWord) > 5)
			{
				printf("ERROR: digit %d is too long!", atoi(tempWord));
				noError = 0;
				exit(1);
			}
			else if(flag == 0)
			{
				sprintf(lexArray[lexIndex], "%d", numbersym);
				sprintf(lexArray[lexIndex+1], "%s", tempWord);
				lexIndex+=2;
			}
			else
			{
				printf("ERROR: identifier cannot start with a number");
				noError = 0;
				exit(1);
			}
			isNumber = 1;
		}

		//If the word is not a number, checks to see if it is a keyword
		if(isNumber != 1)
		{
			for(int j = 0;j < 14;j++)
			{
				if(0 == strcmp(tempWord, keywords[j]))
				{
					if(j == 0)
					{
						sprintf(lexArray[lexIndex], "%d", readsym);
						lexIndex++;
					}
					if(j == 1)
					{
						sprintf(lexArray[lexIndex], "%d", writesym);
						lexIndex++;
					}
					if(j == 2)
					{
						sprintf(lexArray[lexIndex], "%d", constsym);
						lexIndex++;
					}
					if(j == 3)
					{
						sprintf(lexArray[lexIndex], "%d", varsym);
						lexIndex++;
					}
					if(j == 4)
					{
						sprintf(lexArray[lexIndex], "%d", procsym);
						lexIndex++;
					}
					if(j == 5)
					{
						sprintf(lexArray[lexIndex], "%d", callsym);
						lexIndex++;
					}
					if(j == 6)
					{
						sprintf(lexArray[lexIndex], "%d", beginsym);
						lexIndex++;
					}
					if(j == 7)
					{
						sprintf(lexArray[lexIndex], "%d", endsym);
						lexIndex++;
					}
					if(j == 8)
					{
						sprintf(lexArray[lexIndex], "%d", ifsym);
						lexIndex++;
					}
					if(j == 9)
					{
						sprintf(lexArray[lexIndex], "%d", thensym);
						lexIndex++;
					}
					if(j == 10)
					{
						sprintf(lexArray[lexIndex], "%d", elsesym);
						lexIndex++;
					}
					if(j == 11)
					{
						sprintf(lexArray[lexIndex], "%d", whilesym);
						lexIndex++;
					}
					if(j == 12)
					{
						sprintf(lexArray[lexIndex], "%d", dosym);
						lexIndex++;
					}
					if(j == 13)
					{
						sprintf(lexArray[lexIndex], "%d", oddsym);
						lexIndex++;
					}
					isKeyword = 1;
				}
			}
		}

		//If the word is not a keyword then it checks if it is a symbol
		//i.e * / - +
		if(isKeyword != 1)
		{
			for(int j = 0;j < 16;j++)
			{
				if(0 == strcmp(tempWord, symbols[j]))
				{
					if(j == 0)
					{
						sprintf(lexArray[lexIndex], "%d", plussym);
						lexIndex++;
					}
					if(j == 1)
					{
						sprintf(lexArray[lexIndex], "%d", minussym);
						lexIndex++;
					}
					if(j == 2)
					{
						sprintf(lexArray[lexIndex], "%d", multsym);
						lexIndex++;
					}
					if(j == 3)
					{
						sprintf(lexArray[lexIndex], "%d", slashsym);
						lexIndex++;
					}
					if(j == 4)
					{
						sprintf(lexArray[lexIndex], "%d", eqlsym);
						lexIndex++;
					}
					if(j == 5)
					{
						sprintf(lexArray[lexIndex], "%d", neqsym);
						lexIndex++;
					}
					if(j == 6)
					{
						sprintf(lexArray[lexIndex], "%d", lessym);
						lexIndex++;
					}
					if(j == 7)
					{
						sprintf(lexArray[lexIndex], "%d", leqsym);
						lexIndex++;
					}
					if(j == 8)
					{
						sprintf(lexArray[lexIndex], "%d", gtrsym);
						lexIndex++;
					}
					if(j == 9)
					{
						sprintf(lexArray[lexIndex], "%d", geqsym);
						lexIndex++;
					}
					if(j == 10)
					{
						sprintf(lexArray[lexIndex], "%d", lparentsym);
						lexIndex++;
					}
					if(j == 11)
					{
						sprintf(lexArray[lexIndex], "%d", rparentsym);
						lexIndex++;
					}
					if(j == 12)
					{
						sprintf(lexArray[lexIndex], "%d", commasym);
						lexIndex++;
					}
					if(j == 13)
					{
						sprintf(lexArray[lexIndex], "%d", semicolonsym);
						lexIndex++;
					}
					if(j == 14)
					{
						sprintf(lexArray[lexIndex], "%d", periodsym);
						lexIndex++;
					}
					if(j == 15)
					{
						sprintf(lexArray[lexIndex], "%d", becomessym);
						lexIndex++;
					}
					isSymbol = 1;
				}
			}
		}

		//If the word is not a symbol, keyword, or number it must be an identifier
		//checks to make sure its under 11 characters and prints out identsym
		if(isSymbol != 1 && isKeyword != 1 && isNumber != 1)
		{
			if(strlen(tempWord) <= 11)
			{
				sprintf(lexArray[lexIndex], "%d", identsym);
				sprintf(lexArray[lexIndex+1], "%s", tempWord);
				lexIndex+=2;
			}
			else
			{
				printf("ERROR: identifier %s is too long!", tempWord);
				noError = 0;
				exit(1);
			}
		}
	}

	symbol mainSym;
	mainSym.kind = 3;
	strcpy(mainSym.name, "main");
	mainSym.val = 0;
	mainSym.level = 0;
	mainSym.addr = 3;
	mainSym.mark = 1;
	symbolTable[0] = mainSym;
	symbolTableIndex++;
	program();
	if(noError == 1)
	{
		printf("No errors, program is syntactically correct\n\n");
	}
	FILE *fp;
	fp = fopen("elf.txt", "w");
	printf("Assembly Code:\n");
	printf("Line\tOP\tL\tM\n");
	for(int i = 0; i < assemblyCodeIndex; i++)
	{
		printf("%d\t%s\t%d\t%d\n", i, codes[i].opCode, codes[i].l, codes[i].m);
		
			if(strcmp(codes[i].opCode, "LIT") == 0)
			{
				fprintf(fp, "1 %d %d\n", codes[i].l, codes[i].m);
			}
			if(strcmp(codes[i].opCode, "OPR") == 0)
			{
				fprintf(fp, "2 %d %d\n", codes[i].l, codes[i].m);
			}
			if(strcmp(codes[i].opCode, "LOD") == 0)
			{
				fprintf(fp, "3 %d %d\n", codes[i].l, codes[i].m);
			}
			if(strcmp(codes[i].opCode, "STO") == 0)
			{
				fprintf(fp, "4 %d %d\n", codes[i].l, codes[i].m);
			}
			if(strcmp(codes[i].opCode, "CAL") == 0)
			{
				fprintf(fp, "5 %d %d\n", codes[i].l, codes[i].m);
			}
			if(strcmp(codes[i].opCode, "INC") == 0)
			{
				fprintf(fp, "6 %d %d\n", codes[i].l, codes[i].m);
			}
			if(strcmp(codes[i].opCode, "JMP") == 0)
			{
				fprintf(fp, "7 %d %d\n", codes[i].l, codes[i].m);
			}
			if(strcmp(codes[i].opCode, "JPC") == 0)
			{
				fprintf(fp, "8 %d %d\n", codes[i].l, codes[i].m);
			}
			if(strcmp(codes[i].opCode, "SYS") == 0)
			{
				fprintf(fp, "9 %d %d\n", codes[i].l, codes[i].m);
			}

	}
	printf("\n");

	fclose (inputFile); //Closes the input file
	fclose(fp); //Closes output file
}

int inTable(char c) // Checks if the passed character is in the valid library of characters.
{
	if((c > '&' && c < '?') || (c > '@' && c < '[') || (c > '`' && c < '{'))
	{
		return 1; //Returns 1 if the character is in the valid library
	}
	return 0; //Returns 0 otherwise
}

int isSymbol(char c) // Checks that the passed character is a valid symbol.
{
	char symbols[13] = {'+', '-', '*', '/', '=', '<', '>', '(', ')', ',', ';', '.', ':'};
	for(int i = 0; i < 13;i++)
	{
		if(c == symbols[i])
		{
			return 1; //Returns 1 if the symbol is found.
		}
	}
	return 0; //Returns 0 otherwise
}

int isValid(char c) // Verrifies that the passed character is not a space, newline, or tab.
{
	if(c != ' ' && c != '\n' && c != '\t')
	{
		return 1; //Returns 1 if the character is not a space, newline, or tab
	}
	return 0; //Returns 0 otherwise
}

int symbolTableCheckLO(char* str)
{
	int length = sizeof(symbolTable)/sizeof(symbolTable[0]);
	for(int i = length-1;i >= 0;i--)
	{
		if(symbolTable[i].mark != 1)
		{
			if(strcmp(str, symbolTable[i].name) == 0)
			{
				return i;
			}
		}
	}
	return -1;
}

int symbolTableCheck(char* str, int level)
{
	int length = sizeof(symbolTable)/sizeof(symbolTable[0]);
	for(int i = length-1;i >= 0;i--)
	{
		if(symbolTable[i].mark != 1)
		{
			if(strcmp(str, symbolTable[i].name) == 0 && symbolTable[i].level == level)
			{
				return i;
			}
		}
	}
	return -1;
}

void program()
{
	getNextToken();
	block();
	if(atoi(token) != periodsym)
	{
		printf("ERROR: program must end with period\n");
		noError = 0;
		exit(1);
	}
	emit("SYS", 0, 3);
}

void block()
{
	level++;
	int jmpaddr;
	jmpaddr = assemblyCodeIndex;
	emit("JMP",0,0);
	constDeclaration();
	int numVars = varDeclaration();
	procDeclaration();
	codes[jmpaddr].m = assemblyCodeIndex * 3;
	emit("INC", 0, numVars + 3);
	statement();
	if(level != 1)
	{
		emit("OPR", 0, 0);
	}
	markSymbols(level);
	level--;
}

void constDeclaration()
{
	while(atoi(token) == constsym)
	{
		do {
			getNextToken();
			if(atoi(token) != identsym)
			{
				printf("ERROR: const, var, and read keywords must be followed by identifier\n");
				noError = 0;
				exit(1);
			}
			getNextToken();
			if(symbolTableCheck(token, level) != -1)
			{
				printf("ERROR: symbol name has already been declared\n");
				noError = 0;
				exit(1);
			}

			// Saves Identifier name
		  	symbol currentSym;
			strcpy(currentSym.name, token);

			getNextToken();
			if(atoi(token) != eqlsym)
			{
				printf("ERROR: constants must be assigned with =\n");
				noError = 0;
				exit(1);
			}

			getNextToken();
			if(atoi(token) != numbersym)
			{
				printf("ERROR: constants must be assigned an integer value\n");
				noError = 0;
				exit(1);
			}

		  // sets the struct members to their respective values
		  // and adds the current symbol to the symbolTable
		  getNextToken();
		  int storedValue = atoi(token);
		  currentSym.kind = 1;
		  currentSym.val = storedValue;
		  currentSym.level = level;
		  currentSym.addr = 0;
		  currentSym.mark = 0;
		  symbolTable[symbolTableIndex] = currentSym;
		  symbolTableIndex++;

		  getNextToken();
		} while(atoi(token) == commasym);
		if(atoi(token) != semicolonsym)
		{
			printf("ERROR: constant declarations must be followed by a semicolon\n");
			noError = 0;
			exit(1);
		}
		getNextToken();
	}
}

int varDeclaration()
{
	int numVars = 0;
	while(atoi(token) == varsym)
	{
		do {
			numVars++;
			getNextToken();
			if(atoi(token) != identsym)
			{
				printf("ERROR: const, var, and read keywords must be followed by identifier\n");
				noError = 0;
				exit(1);
			}
			getNextToken();
			if(symbolTableCheck(token, level) != -1) // We need to make sure that duplicate variable names in different scopes are allowed
			{
				printf("ERROR: symbol name has already been declared\n");
				noError = 0;
				exit(1);
			}

			// sets the struct members to their respective values
		  	// and adds the current symbol to the symbolTable
		  	symbol currentSym;
		  	strcpy(currentSym.name, token);
		  	currentSym.kind = 2;
		  	currentSym.val = 0;
		  	currentSym.level = level;
		  	currentSym.addr =  numVars + 2;
		  	currentSym.mark = 0;
		  	symbolTable[symbolTableIndex] = currentSym;
		  	symbolTableIndex++;
		  	getNextToken();
		}while(atoi(token) == commasym);
		if(atoi(token) != semicolonsym)
		{
			printf("ERROR: variable declarations must be followed by a semicolon\n");
			noError = 0;
			exit(1);
		}
		getNextToken();
	}
	return numVars;
}

void procDeclaration()
{
	while(atoi(token) == procsym)
	{
		getNextToken();
		if(atoi(token) != identsym)
		{
			printf("ERROR: procedure call must be followed by an identifier.\n");
			noError = 0;
			exit(1);
		}
		getNextToken();
		symbol currentSym;
		strcpy(currentSym.name, token);
		currentSym.kind = 3;
		currentSym.val = 0;
		currentSym.level = level;
		currentSym.addr =  assemblyCodeIndex;
		currentSym.mark = 0;
		symbolTable[symbolTableIndex] = currentSym;
		symbolTableIndex++;
		getNextToken();
		if(atoi(token) != semicolonsym)
		{
			printf("ERROR: procedure declarations must be followed by a semicolon\n");
			noError = 0;
			exit(1);
		}
		getNextToken();
		block();
		if(atoi(token) != semicolonsym)
		{
			printf("ERROR: procedure declarations must be followed by a semicolon\n");
			noError = 0;
			exit(1);
		}
		getNextToken();
	}
}

void statement()
{
	int symIndex;
	int jpcIndex;
	int loopIndex;
	if(atoi(token) == identsym)
	{
		getNextToken();
		symIndex = symbolTableCheckLO(token);
		if(symIndex == -1)
		{
			printf("ERROR: undeclared identifier\n");
			noError = 0;
			exit(1);
		}
		if(symbolTable[symIndex].kind != 2)
		{
			printf("ERROR: only variable values may be altered\n");
			noError = 0;
			exit(1);
		}
		getNextToken();
		if(atoi(token) != becomessym)
		{
			printf("ERROR: assignment statements must use :=\n");
			noError = 0;
			exit(1);
		}
		getNextToken();
		expression();
		emit("STO", level - symbolTable[symIndex].level, symbolTable[symIndex].addr);
		return;
	}
	if(atoi(token) == callsym)
	{
		getNextToken();
		if(atoi(token) != identsym)
		{
			printf("ERROR: call must be followed by an identifier.\n");
			noError = 0;
			exit(1);
		}
		getNextToken();
		symIndex = symbolTableCheckLO(token);
		if(symIndex == -1)
		{
			printf("ERROR: undeclared identifier\n");
			noError = 0;
			exit(1);
		}
		if(symbolTable[symIndex].kind != 3)
		{
			printf("ERROR: call must be followed by procedure\n");
			noError = 0;
			exit(1);
		}
		emit("CAL",0,symbolTable[symIndex].addr * 3);
		getNextToken();
	}
	if(atoi(token) == beginsym)
	{
		do{
			getNextToken();
			statement();
		}while(atoi(token) == semicolonsym);

		if(atoi(token) != endsym)
		{
			printf("ERROR: begin must be followed by end\n");
			noError = 0;
			exit(1);
		}
		getNextToken();
		return;
	}
	if(atoi(token) == ifsym)
	{
		getNextToken();
		condition();
		jpcIndex = assemblyCodeIndex;
		emit("JPC", 0, jpcIndex * 3);
		if(atoi(token) != thensym)
		{
			printf("ERROR: if must be followed by then\n");
			noError = 0;
			exit(1);
		}
		getNextToken();
		statement();
		codes[jpcIndex].m = assemblyCodeIndex * 3;
		return;
	}
	if(atoi(token) == whilesym)
	{
		getNextToken();
		loopIndex = assemblyCodeIndex;
		condition();
		if(atoi(token) != dosym)
		{
			printf("ERROR: while must be followed by do\n");
			noError = 0;
			exit(1);
		}
		getNextToken();
		jpcIndex = assemblyCodeIndex;
		emit("JPC", 0, jpcIndex * 3);
		statement();
		emit("JMP",0,loopIndex * 3);
		codes[jpcIndex].m = assemblyCodeIndex;
		return;
	}
	if(atoi(token) == readsym)
	{
		getNextToken();
		if(atoi(token) != identsym)
		{
			printf("ERROR: const, var, and read keywords must be followed by identifier\n");
			noError = 0;
			exit(1);
		}
		getNextToken();
		symIndex = symbolTableCheckLO(token);
		if(symIndex == -1)
		{
			printf("ERROR: undeclared identifier\n");
			noError = 0;
			exit(1);
		}
		if(symbolTable[symIndex].kind != 2)
		{
			printf("ERROR: only variable values may be altered\n");
			noError = 0;
			exit(1);
		}
		getNextToken();
		emit("SYS", 0, 2);
		emit("STO", level - symbolTable[symIndex].level, symbolTable[symIndex].addr);
		return;
	}
	if(atoi(token) == writesym)
	{
		getNextToken();
		expression();
		emit("SYS", 0, 1);
		return;
	}
}

void condition()
{
	if(atoi(token) == oddsym)
	{
		getNextToken();
		expression();
		emit("OPR", 0, 11); // symbolTable[symbolTableIndex].level
	}
	else
	{
		expression();
		if(atoi(token) == eqlsym)
		{
			getNextToken();
			expression();
			emit("OPR", 0, 5);
		}
		else if(atoi(token) == neqsym)
		{
			getNextToken();
			expression();
			emit("OPR", 0, 6);
		}
		else if(atoi(token) == lessym)
		{
			getNextToken();
			expression();
			emit("OPR", 0, 7);
		}
		else if(atoi(token) == leqsym)
		{
			getNextToken();
			expression();
			emit("OPR", 0, 8);
		}
		else if(atoi(token) == gtrsym)
		{
			getNextToken();
			expression();
			emit("OPR", 0, 9);
		}
		else if(atoi(token) == geqsym)
		{
			getNextToken();
			expression();
			emit("OPR", 0, 10);
		}
		else
		{
			printf("ERROR: condition must contain comparison operator\n");
			noError = 0;
			exit(1);
		}
	}
}

void expression()
{
	if(atoi(token) == minussym)
	{
		getNextToken();
		term();
		while(atoi(token) == plussym || atoi(token) == minussym)
		{
			if(atoi(token) == plussym)
			{
				getNextToken();
				term();
				emit("OPR", 0, 1);
			}
			else
			{
				getNextToken();
				term();
				emit("OPR", 0, 2);
			}
		}
	}
	else
	{
		if(atoi(token) == plussym)
		{
			getNextToken();
		}
		term();
		while(atoi(token) == plussym || atoi(token) == minussym)
		{
			if(atoi(token) == plussym)
			{
				getNextToken();
				term();
				emit("OPR", 0, 1);
			}
			else
			{
				getNextToken();
				term();
				emit("OPR", 0, 2);
			}
		}
	}
}

void term()
{
	factor();
	while(atoi(token) == multsym || atoi(token) == slashsym)
	{
		if(atoi(token) == multsym)
		{
			getNextToken();
			factor();
			emit("OPR", 0, 3);
		}
		else
		{
			getNextToken();
			factor();
			emit("OPR", 0, 4);
		}
	}
}

void factor()
{
	int symIndex;
	if(atoi(token) == identsym)
	{
		getNextToken();
		symIndex = symbolTableCheckLO(token);
		if(symIndex == -1)
		{
			printf("ERROR: undeclared identifier\n");
			noError = 0;
			exit(1);
		}
		if(symbolTable[symIndex].kind == 1)
		{
			emit("LIT",0,symbolTable[symIndex].val);
		}
		else
		{
			emit("LOD",level - symbolTable[symIndex].level,symbolTable[symIndex].addr);
		}
		getNextToken();
	}
	else if(atoi(token) == numbersym)
	{
		getNextToken();
		emit("LIT",0,atoi(token));
		getNextToken();
	}
	else if(atoi(token) == lparentsym)
	{
		getNextToken();
		expression();
		if(atoi(token) != rparentsym)
		{
			printf("ERROR: right parenthesis must follow left parenthesis\n");
			noError = 0;
			exit(1);
		}
		getNextToken();
	}
	else
	{
		printf("ERROR: arithmetic equations must contain operands, parentheses, numbers, or symbols\n");
		noError = 0;
		exit(1);
	}
}

void emit(char* opCode, int l, int m)
{
	if(assemblyCodeIndex > 499)
	{
		printf("ERROR: Code index is too large\n");
		noError = 0;
		exit(1);
	}
	assemblyCode temp;
	strcpy(temp.opCode, opCode);
	temp.l = l;
	temp.m = m;
	codes[assemblyCodeIndex] = temp;
	assemblyCodeIndex++;
}

void getNextToken()
{
	strcpy(token, lexArray[lexArrayIndex]);
	lexArrayIndex++;
}

void markSymbols(int level)
{
	for(int i = 0; i <= symbolTableIndex; i++)
	{
		if(symbolTable[i].level == level && symbolTable[i].kind != 3)
		{
			symbolTable[i].mark = 1;
		}
	}
}
