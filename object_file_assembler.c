/**
 * Project 2
 * Assembler code fragment for LC-2K
 */

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
//Every LC2K file will contain less than 1000 lines of assembly.
#define MAXLINELENGTH 1000

/**
 * Requires: readAndParse is non-static and unmodified from project 1a. 
 *   inFilePtr and outFilePtr must be opened. 
 *   inFilePtr must be rewound before calling this function.
 * Modifies: outFilePtr
 * Effects: Prints the correct machine code for the input file. After 
 *   reading and parsing through inFilePtr, the pointer is rewound.
 *   Most project 1a error checks are done. No undefined labels of any
 *   type are checked, and these are instead resolved to 0.
*/
/**
 * This function will be provided in an instructor object file once the
 * project 1a deadline + late days has passed.
*/
extern void print_inst_machine_code(FILE *inFilePtr, FILE *outFilePtr);

struct SymbolLine
{
    char label[7];
    char location;
    int offset;
};

struct RelocationLine
{
int offset;
char opcode[6];
char label[7];
};

struct Header
{
int numText;
int numData;
int numSymbol;
int numRelo;
struct SymbolLine symbolList[65536];
struct RelocationLine relocationList[65536];
};

struct labelAddress
{
char label[MAXLINELENGTH];
int address;
};


int readAndParse(FILE *, char *, char *, char *, char *, char *);
static void checkForBlankLinesInCode(FILE *inFilePtr);
static inline int isNumber(char *);
//static inline void printHexToFile(FILE *, int);

int main(int argc, char **argv) {
    char *inFileStr, *outFileStr;
    FILE *inFilePtr, *outFilePtr;
    char label[MAXLINELENGTH], opcode[MAXLINELENGTH], arg0[MAXLINELENGTH],
            arg1[MAXLINELENGTH], arg2[MAXLINELENGTH];

    if (argc != 3) {
        printf("error: usage: %s <assembly-code-file> <machine-code-file>\n",
            argv[0]);
        exit(1);
    }

    inFileStr = argv[1];
    outFileStr = argv[2];

    inFilePtr = fopen(inFileStr, "r");
    if (inFilePtr == NULL) {
        printf("error in opening %s\n", inFileStr);
        exit(1);
    }
    // Check for blank lines in the middle of the code.
    checkForBlankLinesInCode(inFilePtr);
    
    outFilePtr = fopen(outFileStr, "w");
    if (outFilePtr == NULL) {
        printf("error in opening %s\n", outFileStr);
        exit(1);
    }


    int currentAddress = 0;
    int largestLabelAddress=0;
    struct labelAddress addresses[MAXLINELENGTH];

    while(readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2)) {
        if(strcmp(label,""))
        {
            struct labelAddress lb;
            strcpy(lb.label,label);
            lb.address = currentAddress;
            addresses[currentAddress] = lb;
            largestLabelAddress = currentAddress;
        }
        currentAddress++;
    }
    for (int i = 0; i <= largestLabelAddress; i++) {
        for (int j = i + 1; j <= largestLabelAddress; j++) {
            if (addresses[i].label[0] != '\0' && addresses[j].label[0] != '\0') {
                if (!strcmp(addresses[i].label, addresses[j].label)) {
                     exit(1); // duplicate label
                }
            }
        }
    }
    rewind(inFilePtr);

    struct Header header = {0};
    bool foundFill = false;
     while (readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2) ) {
        if(!strcmp(opcode,"halt")) 
        {
            if(!strcmp(label,""))
            {
            header.numText++;
            continue;
            }
            else if(label[0] >='A' && label[0]<='Z')
            {
                bool labelAlreadyInSymbolList = false;
        for(int i = 0; i < header.numSymbol; i++)
        {
            if(!strcmp(arg2,header.symbolList[i].label))
            {
                labelAlreadyInSymbolList = true;
                break;
            }
        }
        if(!labelAlreadyInSymbolList)
        {
            struct SymbolLine symbol;
            strcpy(symbol.label,label);
            symbol.location = 'T';
            symbol.offset = header.numText;
            header.symbolList[header.numSymbol] = symbol;
            header.numSymbol++;
        }
        header.numText++;
        continue;
        }
        }

        if(!strcmp(opcode,".fill"))
        {
            foundFill = true;
            break;
        }
        //check global label in arg2
        if(!isNumber(arg2) && (arg2[0] >='A' && arg2[0]<='Z')) //global label in arg2
        {
        bool foundLabel = false;
        for(int i = 0; i < largestLabelAddress + 1; i++)
        {
            if(!strcmp(arg2,addresses[i].label))
            {
                foundLabel = true;
                break;
            }
        }
        if(!foundLabel)
        {
        bool labelAlreadyInSymbolList = false;
        for(int i = 0; i < header.numSymbol; i++)
        {
            if(!strcmp(arg2,header.symbolList[i].label))
            {
                labelAlreadyInSymbolList = true;
                break;
            }
        }
        if(!labelAlreadyInSymbolList)
        {
            struct SymbolLine symbol;
            strcpy(symbol.label,arg2);
            symbol.location = 'U';
            int sectionOffset = 0;
            symbol.offset = sectionOffset;
            header.symbolList[header.numSymbol] = symbol;
            header.numSymbol++;
        }
        }
       }
     //symbol table gets generated from definition of label
        if(strcmp(label,"") && (label[0] >='A' && label[0]<='Z'))
        {
            struct SymbolLine symbol;
            strcpy(symbol.label,label);
            symbol.location = 'T';
            symbol.offset = header.numText;
            header.symbolList[header.numSymbol] = symbol;
            header.numSymbol++;
        }
        //check if local label is defined in arg2
       if(!isNumber(arg2) && (arg2[0] >='a' && arg2[0] <='z'))
       {
        bool foundLabel = false;
        for(int i = 0; i < largestLabelAddress + 1; i++)
        {
            if(!strcmp(addresses[i].label,arg2))
            {
                foundLabel = true;
                break;
            }
        }
        if(!foundLabel)
        {
            printf("undefined local label when used in arg2\n");
            exit(1);
        } 
       }
       //check for beq using undefined label
       if(!strcmp(opcode,"beq") && !isNumber(arg2))
       {
        bool foundLabel = false;
        for(int i = 0; i < largestLabelAddress + 1; i++)
        {
            if(!strcmp(addresses[i].label,arg2))
            {
                foundLabel = true;
                break;
            }
        }
        if(!foundLabel)
        {
            printf("beq using undefined label\n");
            exit(1);
        }
       }
       //add to relocation table
       if(!isNumber(arg2) && (!strcmp(opcode,"lw") || !strcmp(opcode,"sw")))
        {
            struct RelocationLine line;
            strcpy(line.opcode,opcode);
            strcpy(line.label,arg2);
            line.offset = header.numText; //we will have a separate function for .fill
            header.relocationList[header.numRelo] = line;
            header.numRelo++;

        }
        if (!strcmp(opcode, "add")) {
        if(!isNumber(arg0) || !isNumber(arg1) || !isNumber(arg2)) exit(1);
        if(arg0[0]-'0' < 0 || arg0[0]-'0' > 7) exit(1);
        if(arg1[0]-'0' < 0 || arg1[0]-'0' > 7) exit(1);
        if(arg2[0]-'0' < 0 || arg2[0]-'0' > 7) exit(1);
        }
        if (!strcmp(opcode, "nor")) {
        if(!isNumber(arg0) || !isNumber(arg1) || !isNumber(arg2)) exit(1);
        if(arg0[0]-'0' < 0 || arg0[0]-'0' > 7) exit(1);
        if(arg1[0]-'0' < 0 || arg1[0]-'0' > 7) exit(1);
        if(arg2[0]-'0' < 0 || arg2[0]-'0' > 7) exit(1);
        }
        if (!strcmp(opcode, "lw")) {
        if(!isNumber(arg0) || !isNumber(arg1)) exit(1);
        if(arg0[0]-'0' < 0 || arg0[0]-'0' > 7) exit(1);
        if(arg1[0]-'0' < 0 || arg1[0]-'0' > 7) exit(1);
        }
        if (!strcmp(opcode, "sw")) {
        if(!isNumber(arg0) || !isNumber(arg1)) exit(1);
        if(arg0[0]-'0' < 0 || arg0[0]-'0' > 7) exit(1);
        if(arg1[0]-'0' < 0 || arg1[0]-'0' > 7) exit(1);
        }
        if (!strcmp(opcode, "beq")) {
        if(!isNumber(arg0) || !isNumber(arg1)) exit(1);
        if(arg0[0]-'0' < 0 || arg0[0]-'0' > 7) exit(1);
        if(arg1[0]-'0' < 0 || arg1[0]-'0' > 7) exit(1);
        }
        if (!strcmp(opcode, "jalr")) {
        if(!isNumber(arg0) || !isNumber(arg1)) exit(1);
        if(arg0[0]-'0' < 0 || arg0[0]-'0' > 7) exit(1);
        if(arg1[0]-'0' < 0 || arg1[0]-'0' > 7) exit(1);
        }
        header.numText++;
     }
     if(foundFill)
     {
    rewind(inFilePtr);
     for(int i = 0; i < header.numText; i++)
     {
        readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2);
     }
     }
    while (readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2) ) {
        //symbol table gets generated from definition of label
        //check for global label in arg0
         if(!isNumber(arg0) && (arg0[0] >='A' && arg0[0]<='Z')) //global label in arg2
        {
        bool foundLabel = false;
        for(int i = 0; i < largestLabelAddress + 1; i++)
        {
            if(!strcmp(arg0,addresses[i].label))
            {
                foundLabel = true;
                break;
            }
        }
        if(!foundLabel)
        {
            bool labelAlreadyInSymbolList = false;
        for(int i = 0; i < header.numSymbol; i++)
        {
            if(!strcmp(arg0,header.symbolList[i].label))
            {
                labelAlreadyInSymbolList = true;
                break;
            }
        }
        if(!labelAlreadyInSymbolList)
        {
            struct SymbolLine symbol;
            strcpy(symbol.label,arg0);
            symbol.location = 'U';
            int sectionOffset = 0;
            symbol.offset = sectionOffset;
            header.symbolList[header.numSymbol] = symbol;
            header.numSymbol++;
        }
        }
       }
        //check for global label in label
        if(strcmp(label,"") && (label[0] >='A' && label[0]<='Z'))
        {
            struct SymbolLine symbol;
            strcpy(symbol.label,label);
            symbol.location = 'D';
            symbol.offset = header.numData;
            header.symbolList[header.numSymbol] = symbol;
            header.numSymbol++;
        }
        //add relocation table
        if(!isNumber(arg0))
        {
            struct RelocationLine line;
            strcpy(line.opcode,opcode);
            strcpy(line.label,arg0);
            line.offset = header.numData; //we will have a separate function for .fill
            header.relocationList[header.numRelo] = line;
            header.numRelo++;
        }
        header.numData++;
     }


    /* logic
    loop through all text lines (until we see a .fill opcode)
    symbol table:
        when we see a global label, check if label is defined in our labels
        if it isn't, add it to symbolTable
        when we see global arg2, check if the label is defined
        if it isn't add to symbolTable
    relocation table:
        if we see symbol address (in either lw or sw, not beq)
        add to relocation table
    increment numText
    loop through all data lines
    do the same thing in constructing sybmol and relo table
    increment numData
    */

    fprintf(outFilePtr, "%d %d %d %d\n",header.numText,header.numData,header.numSymbol,
    header.numRelo);
    rewind(inFilePtr);
    print_inst_machine_code(inFilePtr,outFilePtr);
    for(int i = 0; i <header.numSymbol; i++)
    {
        fprintf(outFilePtr,"%s %c %d\n",header.symbolList[i].label,
        header.symbolList[i].location,header.symbolList[i].offset);
    }
    for(int i = 0; i <header.numRelo; i++)
    {
        fprintf(outFilePtr,"%d %s %s\n",header.relocationList[i].offset,
        header.relocationList[i].opcode,header.relocationList[i].label);
    }

    return(0);
}

/*
* NOTE: The code defined below is not to be modifed as it is implemented correctly.
*/

// Returns non-zero if the line contains only whitespace.
int lineIsBlank(char *line) {
    char whitespace[4] = {'\t', '\n', '\r', ' '};
    int nonempty_line = 0;
    for(int line_idx=0; line_idx < strlen(line); ++line_idx) {
        int line_char_is_whitespace = 0;
        for(int whitespace_idx = 0; whitespace_idx < 4; ++whitespace_idx) {
            if(line[line_idx] == whitespace[whitespace_idx]) {
                line_char_is_whitespace = 1;
                break;
            }
        }
        if(!line_char_is_whitespace) {
            nonempty_line = 1;
            break;
        }
    }
    return !nonempty_line;
}

// Exits 2 if file contains an empty line anywhere other than at the end of the file.
// Note calling this function rewinds inFilePtr.
static void checkForBlankLinesInCode(FILE *inFilePtr) {
    char line[MAXLINELENGTH];
    int blank_line_encountered = 0;
    int address_of_blank_line = 0;
    rewind(inFilePtr);

    for(int address = 0; fgets(line, MAXLINELENGTH, inFilePtr) != NULL; ++address) {
        // Check for line too long
        if (strlen(line) >= MAXLINELENGTH-1) {
            printf("error: line too long\n");
            exit(1);
        }

        // Check for blank line.
        if(lineIsBlank(line)) {
            if(!blank_line_encountered) {
                blank_line_encountered = 1;
                address_of_blank_line = address;
            }
        } else {
            if(blank_line_encountered) {
                printf("Invalid Assembly: Empty line at address %d\n", address_of_blank_line);
                exit(2);
            }
        }
    }
    rewind(inFilePtr);
}

/*
 * Read and parse a line of the assembly-language file.  Fields are returned
 * in label, opcode, arg0, arg1, arg2 (these strings must have memory already
 * allocated to them).
 *
 * Return values:
 *     0 if reached end of file
 *     1 if all went well
 */
int readAndParse(FILE *inFilePtr, char *label,
	char *opcode, char *arg0, char *arg1, char *arg2) {

    char line[MAXLINELENGTH];
    char *ptr = line;

    // delete prior values
    label[0] = opcode[0] = arg0[0] = arg1[0] = arg2[0] = '\0';

    // read the line from the assembly-language file
    if (fgets(line, MAXLINELENGTH, inFilePtr) == NULL) {
		// reached end of file
        return(0);
    }

    // check for line too long
    if (strlen(line) >= MAXLINELENGTH-1) {
		printf("error: line too long\n");
		exit(1);
    }

    // Ignore blank lines at the end of the file.
    if(lineIsBlank(line)) {
        return 0;
    }

    // is there a label?
    ptr = line;
    if (sscanf(ptr, "%[^\t\n ]", label)) {
		// successfully read label; advance pointer over the label */
        ptr += strlen(label);
    }

    // Parse the rest of the line.  Would be nice to have real regular expressions, but scanf will suffice.
    sscanf(ptr, "%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]",
        opcode, arg0, arg1, arg2);
    return(1);
}

static inline int
isNumber(char *string)
{
    int num;
    char c;
    return((sscanf(string, "%d%c",&num, &c)) == 1);
}

// Prints a machine code word in the proper hex format to the file
// static inline void 
// printHexToFile(FILE *outFilePtr, int word) {
//     fprintf(outFilePtr, "0x%08X\n", word);
// }
