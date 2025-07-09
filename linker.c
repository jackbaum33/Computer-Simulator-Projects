#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAXSIZE 500
#define MAXLINELENGTH 1000
#define MAXFILES 6

static inline void printHexToFile(FILE *, int);

typedef struct FileData FileData;
typedef struct SymbolTableEntry SymbolTableEntry;
typedef struct RelocationTableEntry RelocationTableEntry;
typedef struct CombinedFiles CombinedFiles;

struct SymbolTableEntry {
	char label[7];
	char location;
	unsigned int offset;
};

struct RelocationTableEntry {
    unsigned int file;
	unsigned int offset;
	char inst[6];
	char label[7];
};

struct FileData {
	unsigned int textSize;
	unsigned int dataSize;
	unsigned int symbolTableSize;
	unsigned int relocationTableSize;
	unsigned int textStartingLine; // in final executable
	unsigned int dataStartingLine; // in final executable
	int text[MAXSIZE];
	int data[MAXSIZE];
	SymbolTableEntry symbolTable[MAXSIZE];
	RelocationTableEntry relocTable[MAXSIZE];
};

struct CombinedFiles {
	unsigned int textSize;
	unsigned int dataSize;
	unsigned int symbolTableSize;
	unsigned int relocationTableSize;
	int text[MAXSIZE * MAXFILES];
	int data[MAXSIZE * MAXFILES];
	SymbolTableEntry symbolTable[MAXSIZE * MAXFILES];
	RelocationTableEntry relocTable[MAXSIZE * MAXFILES];
};

int main(int argc, char *argv[]) {
	char *inFileStr, *outFileStr;
	FILE *inFilePtr, *outFilePtr; 

	unsigned int i, j, k, l; //for loop variables

    if (argc <= 2 || argc > 8 ) {
        printf("error: usage: %s <MAIN-object-file> ... <object-file> ... <output-exe-file>, with at most 5 object files\n",
				argv[0]);
		exit(1);
	}

	outFileStr = argv[argc - 1];
	unsigned int numFiles = argc - 2;
	outFilePtr = fopen(outFileStr, "w");
	if (outFilePtr == NULL) {
		printf("error in opening %s\n", outFileStr);
		exit(1);
	}

	FileData files[MAXFILES];
  // read in all files and combine into a "master" file
	for (i = 0; i < argc - 2; ++i) {
		inFileStr = argv[i+1];

		inFilePtr = fopen(inFileStr, "r");
		printf("opening %s\n", inFileStr);

		if (inFilePtr == NULL) {
			printf("error in opening %s\n", inFileStr);
			exit(1);
		}

		char line[MAXLINELENGTH];
		unsigned int textSize, dataSize, symbolTableSize, relocationTableSize;

		// parse first line of file
		fgets(line, MAXSIZE, inFilePtr);
		sscanf(line, "%d %d %d %d",
				&textSize, &dataSize, &symbolTableSize, &relocationTableSize);

		files[i].textSize = textSize;
		files[i].dataSize = dataSize;
		files[i].symbolTableSize = symbolTableSize;
		files[i].relocationTableSize = relocationTableSize;

		// read in text section
		int instr;
		for (j = 0; j < textSize; ++j) {
			fgets(line, MAXLINELENGTH, inFilePtr);
			instr = strtol(line, NULL, 0);
			files[i].text[j] = instr;
		}

		// read in data section
		int data;
		for (j = 0; j < dataSize; ++j) {
			fgets(line, MAXLINELENGTH, inFilePtr);
			data = strtol(line, NULL, 0);
			files[i].data[j] = data;
		}

		// read in the symbol table
		char label[7];
		char type;
		unsigned int addr;
		for (j = 0; j < symbolTableSize; ++j) {
			fgets(line, MAXLINELENGTH, inFilePtr);
			sscanf(line, "%s %c %d",
					label, &type, &addr);
			files[i].symbolTable[j].offset = addr;
			strcpy(files[i].symbolTable[j].label, label);
			files[i].symbolTable[j].location = type;
			if(!strcmp(label,"Stack") && type != 'U')
			{
				exit(1); //we cannot define stack
			}
		}

		// read in relocation table
		char opcode[7];
		for (j = 0; j < relocationTableSize; ++j) {
			fgets(line, MAXLINELENGTH, inFilePtr);
			sscanf(line, "%d %s %s",
					&addr, opcode, label);
			files[i].relocTable[j].offset = addr;
			strcpy(files[i].relocTable[j].inst, opcode);
			strcpy(files[i].relocTable[j].label, label);
			files[i].relocTable[j].file	= i;
		}
		fclose(inFilePtr);
	} // end reading files

	// *** INSERT YOUR CODE BELOW ***
	//    Begin the linking process
	//    Happy coding!!!

	//step 1: calculate text/data positions
	unsigned int totalText = 0;
	for(i = 0; i < numFiles; i++)
	{
		files[i].textStartingLine = totalText;
		totalText += files[i].textSize;
	}
	unsigned int totalData = 0;
	for(i = 0; i < numFiles; i++)
	{
		files[i].dataStartingLine = totalData;
		totalData += files[i].dataSize;
	}

	//step 2: read in symbol table/check for errors
	//duplicate label definition
	for(i = 0; i < numFiles; i++)
	{
		for(j = 0; j < files[i].symbolTableSize; j++)
		{
			if(files[i].symbolTable[j].location != 'U')
			{
				for(k = 0; k < numFiles; k++)
				{
					for(l = 0; l < files[k].symbolTableSize; l++)
					{
						if(!strcmp(files[i].symbolTable[j].label,
						files[k].symbolTable[l].label) && 
						(i != k || j != l) &&
						files[k].symbolTable[l].location != 'U')
						{
							exit(1); //duplicate label
						}
					}
				}
			}
		}
	}

	//undefined global label
	for(i = 0; i < numFiles; i++)
	{
		for(j = 0; j < files[i].symbolTableSize; j++)
		{
			if(files[i].symbolTable[j].location == 'U' && 
			strcmp("Stack",files[i].symbolTable[j].label))
			{
				int foundLabel = 0;
				for(k = 0; k < numFiles; k++)
				{
					for (l = 0; l < files[k].symbolTableSize; l++)
					{
						if(!strcmp(files[i].symbolTable[j].label,
						files[k].symbolTable[l].label) &&
						files[k].symbolTable[l].location != 'U')
						 {
							foundLabel = 1;
							break;
						 }
					}
					if(foundLabel) break;
				}
				if(!foundLabel)
				{
					exit(1); //undefined global label
				}
			}
		}
	}

	//"Stack" defined as a global label
	for(i = 0; i < numFiles; i++)
	{
		for(j = 0; j < files[i].symbolTableSize; j++)
		{
			if(files[i].symbolTable[j].location != 'U' &&
			!strcmp(files[i].symbolTable[j].label,"Stack"))
			{
				exit(1); //we cannot ever define stack
			}
		}
	}

	//step 3: create combined text and data section
	CombinedFiles finalFile;
	finalFile.textSize = totalText;
	finalFile.dataSize = totalData;

	unsigned int currentTextIndex = 0;
	for(i = 0; i < numFiles; i++)
	{
		for(j = 0; j < files[i].textSize; j++)
		{
			finalFile.text[currentTextIndex] = files[i].text[j];
			currentTextIndex++;
		}
	}

	unsigned int currentDataIndex = 0;
	for(i = 0; i < numFiles; i++)
	{
		for(j = 0; j < files[i].dataSize; j++)
		{
			finalFile.data[currentDataIndex] = files[i].data[j];
			currentDataIndex++;
		}
	}

	//step 4: fix refs based on relocation table
	for(i = 0; i < numFiles; i++)
	{
		for(j = 0; j < files[i].relocationTableSize; j++)
		{
			RelocationTableEntry* currentReloc = &files[i].relocTable[j];
			int entryInTextSection = (strcmp(currentReloc->inst,".fill"));
			int currentTargetAddress = 0;
			if(!strcmp(currentReloc->label,"Stack"))
			{
				currentTargetAddress = finalFile.textSize + finalFile.dataSize;
			}
			else
			{
				if(currentReloc->label[0] >= 'A' && currentReloc->label[0] <= 'Z')
				{
					int foundLabel = 0;
					for(k = 0; k < numFiles; k++)
					{
						for(l = 0; l < files[k].symbolTableSize; l++)
						{
							if(!strcmp(currentReloc->label,files[k].symbolTable[l].label) &&
							files[k].symbolTable[l].location != 'U')
							{
								if(files[k].symbolTable[l].location == 'T')
								{
									currentTargetAddress = 
									files[k].textStartingLine + 
									files[k].symbolTable[l].offset;
								}
								else if(files[k].symbolTable[l].location == 'D')
								{
									currentTargetAddress = 
									finalFile.textSize + 
									files[k].dataStartingLine + 
									files[k].symbolTable[l].offset;
								}
								foundLabel = 1;
								break;
							}
						}
						if(foundLabel) break;
					}
				}
				else //local symbol
				{
					int currentOffset;
					if(entryInTextSection)
					{
						//lw or sw
						currentOffset = files[i].text[currentReloc->offset] & 0xFFFF;
					}
					else
					{
						currentOffset = files[i].data[currentReloc->offset];
					}
					if(currentOffset < files[i].textSize)
					{
						currentTargetAddress = files[i].textStartingLine + 
						currentOffset;
					}
					else
					{
						currentTargetAddress = finalFile.textSize + files[i].dataStartingLine + 
						(currentOffset - files[i].textSize);
					}
				}
			}
			if(entryInTextSection)
			{
				unsigned int offset = files[i].textStartingLine + currentReloc->offset;
				int currentInstruction = finalFile.text[offset];
				int currentOpcode = (currentInstruction >> 22) & 0x7;
				int currentRegA = (currentInstruction >> 19) & 0x7;
				int currentRegB = (currentInstruction >> 16) & 0x7;
				finalFile.text[offset] = 
				(currentOpcode << 22) |
				(currentRegA << 19) | 
				(currentRegB << 16) |
				(currentTargetAddress & 0xFFFF);
			}
			else
			{
				unsigned int offset = files[i].dataStartingLine + currentReloc->offset;
				finalFile.data[offset] = currentTargetAddress;
			}
		}
	}
	//step 5: print final output
	for(i = 0; i < finalFile.textSize; i++)
	{
		printHexToFile(outFilePtr,finalFile.text[i]);
	}
	for(int i = 0; i < finalFile.dataSize; i++)
	{
		printHexToFile(outFilePtr,finalFile.data[i]);
	}
    /* here is an example of using printHexToFile. This will print a
       machine code word / number in the proper hex format to the output file */
    //printHexToFile(outFilePtr, 123);
	fclose(outFilePtr);
} // main

// Prints a machine code word in the proper hex format to the file
static inline void 
printHexToFile(FILE *outFilePtr, int word) {
    fprintf(outFilePtr, "0x%08X\n", word);
}