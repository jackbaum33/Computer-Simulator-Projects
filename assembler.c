#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

//Every LC2K file will contain less than 1000 lines of assembly.
#define MAXLINELENGTH 1000

int readAndParse(FILE *, char *, char *, char *, char *, char *);
static void checkForBlankLinesInCode(FILE *inFilePtr);
static inline int isNumber(char *);
static inline void printHexToFile(FILE *, int);



//make a struct that will store each label and address
struct labelAddress
{
char label[MAXLINELENGTH];
int address;
};

char* intTo16BitBinary(int num) {
    static char binary[16];  // 16 bits + null terminator

    // For negative numbers, calculate two's complement representation
    if (num < 0) {
        num = (1 << 16) + num;  // Convert negative to two's complement for 16-bit
    }

    unsigned short mask = 0x8000;  // Mask for the highest bit
    for (int i = 0; i < 16; i++) {
        if (num & mask) {
            binary[i] = '1';
        } else {
            binary[i] = '0';
        }
        mask >>= 1;  // Shift the mask right for the next bit
    }
    return binary;
}




int
main(int argc, char **argv)
{
    char* binaryReps[8] = {"000","001","010","011","100","101","110","111"};
    struct labelAddress addresses[MAXLINELENGTH];
    char *inFileString, *outFileString;
    FILE *inFilePtr, *outFilePtr;
    char label[MAXLINELENGTH], opcode[MAXLINELENGTH], arg0[MAXLINELENGTH],
            arg1[MAXLINELENGTH], arg2[MAXLINELENGTH];

    if (argc != 3) {
        printf("error: usage: %s <assembly-code-file> <machine-code-file>\n",
            argv[0]);
        exit(1);
    }

    inFileString = argv[1];
    outFileString = argv[2];

    inFilePtr = fopen(inFileString, "r");
    if (inFilePtr == NULL) {
        printf("error in opening %s\n", inFileString);
        exit(1);
    }

    // Check for blank lines in the middle of the code.
    checkForBlankLinesInCode(inFilePtr);

    outFilePtr = fopen(outFileString, "w");
    if (outFilePtr == NULL) {
        printf("error in opening %s\n", outFileString);
        exit(1);
    }

    int currentAddress = 0;
    int largestLabelAddress=0;
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
    currentAddress = 0;
    while(readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2) ) {
    if (!strcmp(opcode, "add")) {
        if(!isNumber(arg0) || !isNumber(arg1) || !isNumber(arg2)) exit(1);
        if(arg0[0]-'0' < 0 || arg0[0]-'0' > 7) exit(1);
        if(arg1[0]-'0' < 0 || arg1[0]-'0' > 7) exit(1);
        if(arg2[0]-'0' < 0 || arg2[0]-'0' > 7) exit(1);
        char hex[27] = "000"; //start binary representation with opcode
        char* arg0rep = binaryReps[arg0[0]-'0'];
        char* arg1rep = binaryReps[arg1[0]-'0'];
        char* arg2rep = binaryReps[arg2[0]-'0'];
        strcat(hex,arg0rep);
        strcat(hex,arg1rep);
        strcat(hex,"0000000000000");
        strcat(hex,arg2rep);
        hex[26] = '\0';

        int num = 0;
        for(int i = 0; hex[i] != '\0'; i++)
        {
            num = num * 2 + (hex[i] - '0');
        }
        printHexToFile(outFilePtr,num);  
    }
    else if (!strcmp(opcode, "nor")) {
        if(!isNumber(arg0) || !isNumber(arg1) || !isNumber(arg2)) exit(1);
        if(arg0[0]-'0' < 0 || arg0[0]-'0' > 7) exit(1);
        if(arg1[0]-'0' < 0 || arg1[0]-'0' > 7) exit(1);
        if(arg2[0]-'0' < 0 || arg2[0]-'0' > 7) exit(1);
        char hex[27] = "001"; //start binary representation with opcode
        char* arg0rep = binaryReps[arg0[0]-'0'];
        char* arg1rep = binaryReps[arg1[0]-'0'];
        char* arg2rep = binaryReps[arg2[0]-'0'];
        strcat(hex,arg0rep);
        strcat(hex,arg1rep);
        strcat(hex,"0000000000000");
        strcat(hex,arg2rep);
        hex[26] = '\0';

        int num = 0;
        for(int i = 0; hex[i] != '\0'; i++)
        {
            num = num * 2 + (hex[i] - '0');
        }
        printHexToFile(outFilePtr,num);
    }
    else if (!strcmp(opcode, "lw")) {
        int offsetField = 0;
        if(!isNumber(arg0) || !isNumber(arg1)) exit(1);
        if(arg0[0]-'0' < 0 || arg0[0]-'0' > 7) exit(1);
        if(arg1[0]-'0' < 0 || arg1[0]-'0' > 7) exit(1);
        char hex[27] = "010"; //start binary representation with opcode
        char* arg0rep = binaryReps[arg0[0]-'0'];
        char* arg1rep = binaryReps[arg1[0]-'0'];
        strcat(hex,arg0rep);
        strcat(hex,arg1rep);
        if(!isNumber(arg2))
        {
            int labelFound = 0;
            for(int i = 0; i <= largestLabelAddress; i++)
           {
            if(!strcmp(addresses[i].label,arg2))
            {
                offsetField = addresses[i].address;
                labelFound = 1;
                break;
            }
           }
           if(!labelFound) exit(1);
           if(offsetField > 32767 || offsetField < -32768) exit(1);
           char* binaryOffset = intTo16BitBinary(offsetField);
           strcat(hex,binaryOffset);
           hex[26] = '\0';
        }
        else
        {
            offsetField = atoi(arg2);
            if(atoi(arg2) > 32767 || atoi(arg2) < -32768) exit(1);
            char* binaryOffset = intTo16BitBinary(offsetField);
            strcat(hex,binaryOffset);
            hex[26] = '\0';           
        }
        int num = 0;
        for(int i = 0; hex[i] != '\0'; i++)
        {
            num = num * 2 + (hex[i] - '0');
        }
        printHexToFile(outFilePtr,num);

    }
    else if (!strcmp(opcode, "sw")) {
                int offsetField = 0;
        if(!isNumber(arg0) || !isNumber(arg1)) exit(1);
        if(arg0[0]-'0' < 0 || arg0[0]-'0' > 7) exit(1);
        if(arg1[0]-'0' < 0 || arg1[0]-'0' > 7) exit(1);
        char hex[27] = "011"; //start binary representation with opcode
        char* arg0rep = binaryReps[arg0[0]-'0'];
        char* arg1rep = binaryReps[arg1[0]-'0'];
        strcat(hex,arg0rep);
        strcat(hex,arg1rep);
        if(!isNumber(arg2))
        {
            int labelFound = 0;
           for(int i = 0; i <= largestLabelAddress; i++)
           {
            if(!strcmp(addresses[i].label,arg2))
            {
                offsetField = addresses[i].address;
                labelFound = 1;
                break;
            }
           }
           if(!labelFound) exit(1);
           if(offsetField > 32767 || offsetField < -32768) exit(1);
           char* binaryOffset = intTo16BitBinary(offsetField);
           strcat(hex,binaryOffset);
           hex[26] = '\0';
        }
        else
        {
            offsetField = atoi(arg2);
            if(atoi(arg2) > 32767 || atoi(arg2) < -32768) exit(1);
            char* binaryOffset = intTo16BitBinary(offsetField);
            strcat(hex,binaryOffset);
            hex[26] = '\0'; 
        }
        int num = 0;
        for(int i = 0; hex[i] != '\0'; i++)
        {
            num = num * 2 + (hex[i] - '0');
        }
        printHexToFile(outFilePtr,num);
    }
    else if (!strcmp(opcode, "beq")) {
        if(!isNumber(arg0) || !isNumber(arg1)) exit(1);
        if(arg0[0]-'0' < 0 || arg0[0]-'0' > 7) exit(1);
        if(arg1[0]-'0' < 0 || arg1[0]-'0' > 7) exit(1);
        char hex[27] = "100"; //start binary representation with opcode
        char* arg0rep = binaryReps[arg0[0]-'0'];
        char* arg1rep = binaryReps[arg1[0]-'0'];
        strcat(hex,arg0rep);
        strcat(hex,arg1rep);
        int branchAddress = 0;
        if(!isNumber(arg2))
        {
           for(int i = 0; i <= largestLabelAddress; i++)
           {
            if(!strcmp(addresses[i].label,arg2))
            {
                branchAddress = addresses[i].address;
                break;
            }
           }
            int offset = branchAddress - currentAddress - 1;
            if(offset > 32767 || offset < -32768) exit(1);
            char* binaryOffset = intTo16BitBinary(offset);
            strcat(hex,binaryOffset);
            hex[26] = '\0'; 
            int num = 0;
            for(int i = 0; hex[i] != '\0'; i++)
            {
                num = num * 2 + (hex[i] - '0');
            }
            printHexToFile(outFilePtr,num);
        }
        else
        {
            if(atoi(arg2) > 32767 || atoi(arg2) < -32768) exit(1);
            branchAddress = atoi(arg2);     
            int offset = branchAddress;
            char* binaryOffset = intTo16BitBinary(offset);
            strcat(hex,binaryOffset);
            hex[26] = '\0'; 
            int num = 0;
            for(int i = 0; hex[i] != '\0'; i++)
            {
            num = num * 2 + (hex[i] - '0');
            }
            printHexToFile(outFilePtr,num);
        }
    }
    else if (!strcmp(opcode, "jalr")) {
        if(!isNumber(arg0) || !isNumber(arg1)) exit(1);
        if(arg0[0]-'0' < 0 || arg0[0]-'0' > 7) exit(1);
        if(arg1[0]-'0' < 0 || arg1[0]-'0' > 7) exit(1);
        char hex[27] = "101"; //start binary representation with opcode
        char* arg0rep = binaryReps[arg0[0]-'0'];
        char* arg1rep = binaryReps[arg1[0]-'0'];
        strcat(hex,arg0rep);
        strcat(hex,arg1rep);
        strcat(hex,"0000000000000000");
        hex[26] = '\0';
        int num = 0;
        for(int i = 0; hex[i] != '\0'; i++)
        {
            num = num * 2 + (hex[i] - '0');
        }
        printHexToFile(outFilePtr,num);
    }
    else if (!strcmp(opcode, "halt")) {
        char hex[27] = "110"; //start binary representation with opcode
        strcat(hex,"0000000000000000000000");
        hex[26] = '\0';
        int num = 0;
        for(int i = 0; hex[i] != '\0'; i++)
        {
            num = num * 2 + (hex[i] - '0');
        }
        printHexToFile(outFilePtr,num);
    }
    else if (!strcmp(opcode, "noop")) {
        char hex[27] = "111"; //start binary representation with opcode
        strcat(hex,"0000000000000000000000");
        hex[26] = '\0';
        int num = 0;
        for(int i = 0; hex[i] != '\0'; i++)
        {
            num = num * 2 + (hex[i] - '0');
        }
        printHexToFile(outFilePtr,num);
    }
    else if(!strcmp(opcode, ".fill"))
    {
        if(isNumber(arg0))
        {
        int val = atoi(arg0);
        printHexToFile(outFilePtr,val);
        }
        else
        {
            int address = 0;
            int labelFound = 0;
           for(int i = 0; i <= largestLabelAddress; i++)
           {
            if(!strcmp(addresses[i].label,arg0))
            {
                address = addresses[i].address;
                labelFound = 1;
                break;
            }
           }
           if(!labelFound) exit(1);
           printHexToFile(outFilePtr,address);
        }
    }
    else //invalid opcode
    {
        exit(1);
    }
    currentAddress++;
    }

    /* here is an example of using printHexToFile. This will print a
       machine code word / number in the proper hex format to the output file */
    // printHexToFile(outFilePtr, 123);

    return(0);
}


// Returns non-zero if the line contains only whitespace.
static int lineIsBlank(char *line) {
    char whitespace[4] = {'\t', '\n', '\r', ' '};
    int nonempty_line = 0;
    for(int line_idx=0; line_idx < strlen(line); ++line_idx) {
        int line_char_is_whitespace = 0;
        for(int whitespace_idx = 0; whitespace_idx < 4; ++ whitespace_idx) {
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
* NOTE: The code defined below is not to be modifed as it is implimented correctly.
*/

/*
 * Read and parse a line of the assembly-language file.  Fields are returned
 * in label, opcode, arg0, arg1, arg2 (these strings must have memory already
 * allocated to them).
 *
 * Return values:
 *     0 if reached end of file
 *     1 if all went well
 *
 * exit(1) if line is too long.
 */
int
readAndParse(FILE *inFilePtr, char *label, char *opcode, char *arg0,
    char *arg1, char *arg2)
{
    char line[MAXLINELENGTH];
    char *ptr = line;

    /* delete prior values */
    label[0] = opcode[0] = arg0[0] = arg1[0] = arg2[0] = '\0';

    /* read the line from the assembly-language file */
    if (fgets(line, MAXLINELENGTH, inFilePtr) == NULL) {
	/* reached end of file */
        return(0);
    }

    /* check for line too long */
    if (strlen(line) == MAXLINELENGTH-1) {
	printf("error: line too long\n");
	exit(1);
    }

    // Ignore blank lines at the end of the file.
    if(lineIsBlank(line)) {
        return 0;
    }

    /* is there a label? */
    ptr = line;
    if (sscanf(ptr, "%[^\t\n ]", label)) {
	/* successfully read label; advance pointer over the label */
        ptr += strlen(label);
    }

    /*
     * Parse the rest of the line.  Would be nice to have real regular
     * expressions, but scanf will suffice.
     */
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
static inline void 
printHexToFile(FILE *outFilePtr, int word) {
    fprintf(outFilePtr, "0x%08X\n", word);
}
