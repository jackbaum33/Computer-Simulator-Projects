#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

//DO NOT CHANGE THE FOLLOWING DEFINITIONS 

// Machine Definitions
#define MEMORYSIZE 65536 /* maximum number of words in memory (maximum number of lines in a given file)*/
#define NUMREGS 8 /*total number of machine registers [0,7]*/

// File Definitions
#define MAXLINELENGTH 1000 /* MAXLINELENGTH is the max number of characters we read */

typedef struct 
stateStruct {
    int pc;
    int mem[MEMORYSIZE];
    int reg[NUMREGS];
    int numMemory;
} stateType;

void printState(stateType *);


char* binaryString(int);
int offsetToInt(const char*);

int 
main(int argc, char **argv)
{
    int numInstructions = 0;
    char line[MAXLINELENGTH];
    stateType state;
    FILE *filePtr;

    if (argc != 2) {
        printf("error: usage: %s <machine-code file>\n", argv[0]);
        exit(1);
    }

    filePtr = fopen(argv[1], "r");
    if (filePtr == NULL) {
        printf("error: can't open file %s , please ensure you are providing the correct path", argv[1]);
        perror("fopen");
        exit(2);
    }

    /* read the entire machine-code file into memory */
    for (state.numMemory=0; fgets(line, MAXLINELENGTH, filePtr) != NULL; state.numMemory++) {
		    if (state.numMemory >= MEMORYSIZE) {
			      fprintf(stderr, "exceeded memory size\n");
			      exit(2);
		    }
		    if (sscanf(line, "%x", state.mem+state.numMemory) != 1) {
			      fprintf(stderr, "error in reading address %d\n", state.numMemory);
			      exit(2);
		    }
		    printf("memory[%d]=0x%08X\n", state.numMemory, state.mem[state.numMemory]);
    }
    for(int i = 0; i < NUMREGS; i++)
    {
        state.reg[i] = 0;
    }
    state.pc = 0;
    while(true)
    {
        printState(&state);
        int currentNum = state.mem[state.pc];
        char* numString = binaryString(currentNum);
        char opcode0 = numString[0];
        char opcode1 = numString[1];
        char opcode2 = numString[2];
        char opcode[4];
        opcode[0] = opcode0;
        opcode[1] = opcode1;
        opcode[2] = opcode2;
        opcode[3] = '\0';

        if(!strcmp(opcode,"000")) //add
        {
            char regA[4];
            char regB[4];
            char destReg[4];

            regA[0] = numString[3];
            regA[1] = numString[4];
            regA[2] = numString[5];
            regA[3] = '\0';

            regB[0] = numString[6];
            regB[1] = numString[7];
            regB[2] = numString[8];
            regB[3] = '\0';

            destReg[0] = numString[22];
            destReg[1] = numString[23];
            destReg[2] = numString[24];
            destReg[3] = '\0';

            int regAVal = 0;
            int regBVal = 0;
            int destRegVal = 0;

            regAVal += (regA[0] == '1') ? 4 : 0;
            regAVal += (regA[1] == '1') ? 2 : 0;
            regAVal += (regA[2] == '1') ? 1 : 0;

            regBVal += (regB[0] == '1') ? 4 : 0;
            regBVal += (regB[1] == '1') ? 2 : 0;
            regBVal += (regB[2] == '1') ? 1 : 0;

            destRegVal += (destReg[0] == '1') ? 4 : 0;
            destRegVal += (destReg[1] == '1') ? 2 : 0;
            destRegVal += (destReg[2] == '1') ? 1 : 0;

            state.reg[destRegVal] = state.reg[regAVal]
            +state.reg[regBVal];
            state.pc++;
        }
        else if(!strcmp(opcode,"001")) //nor
        {
            char regA[4];
            char regB[4];
            char destReg[4];

            regA[0] = numString[3];
            regA[1] = numString[4];
            regA[2] = numString[5];
            regA[3] = '\0';

            regB[0] = numString[6];
            regB[1] = numString[7];
            regB[2] = numString[8];
            regB[3] = '\0';

            destReg[0] = numString[22];
            destReg[1] = numString[23];
            destReg[2] = numString[24];
            destReg[3] = '\0';

            int regAVal = 0;
            int regBVal = 0;
            int destRegVal = 0;

            regAVal += (regA[0] == '1') ? 4 : 0;
            regAVal += (regA[1] == '1') ? 2 : 0;
            regAVal += (regA[2] == '1') ? 1 : 0;

            regBVal += (regB[0] == '1') ? 4 : 0;
            regBVal += (regB[1] == '1') ? 2 : 0;
            regBVal += (regB[2] == '1') ? 1 : 0;

            destRegVal += (destReg[0] == '1') ? 4 : 0;
            destRegVal += (destReg[1] == '1') ? 2 : 0;
            destRegVal += (destReg[2] == '1') ? 1 : 0;
            state.reg[destRegVal] = ~(state.reg[regAVal] | state.reg[regBVal]);
            state.pc++;
        }
        else if(!strcmp(opcode,"010")) //lw
        {
            char regA[4];
            char regB[4];

            regA[0] = numString[3];
            regA[1] = numString[4];
            regA[2] = numString[5];
            regA[3] = '\0';

            regB[0] = numString[6];
            regB[1] = numString[7];
            regB[2] = numString[8];
            regB[3] = '\0';

            char offset[17];
            for(int i = 15; i >=0; i--)
            {
                offset[i] = numString[9+i];
            }
            offset[16] = '\0';

            int regAVal = 0;
            int regBVal = 0;

            regAVal += (regA[0] == '1') ? 4 : 0;
            regAVal += (regA[1] == '1') ? 2 : 0;
            regAVal += (regA[2] == '1') ? 1 : 0;

            regBVal += (regB[0] == '1') ? 4 : 0;
            regBVal += (regB[1] == '1') ? 2 : 0;
            regBVal += (regB[2] == '1') ? 1 : 0;
            int16_t offsetVal = offsetToInt(offset);
            state.pc++;
            state.reg[regBVal] = state.mem[state.reg[regAVal] + offsetVal];
        }
        else if(!strcmp(opcode,"011")) //sw
        {
            char regA[4];
            char regB[4];

            regA[0] = numString[3];
            regA[1] = numString[4];
            regA[2] = numString[5];
            regA[3] = '\0';

            regB[0] = numString[6];
            regB[1] = numString[7];
            regB[2] = numString[8];
            regB[3] = '\0';

            char offset[17];
            for(int i = 15; i >=0; i--)
            {
                offset[i] = numString[9+i];
            }
            offset[16] = '\0';
            int regAVal = 0;
            int regBVal = 0;

            regAVal += (regA[0] == '1') ? 4 : 0;
            regAVal += (regA[1] == '1') ? 2 : 0;
            regAVal += (regA[2] == '1') ? 1 : 0;

            regBVal += (regB[0] == '1') ? 4 : 0;
            regBVal += (regB[1] == '1') ? 2 : 0;
            regBVal += (regB[2] == '1') ? 1 : 0;
            int16_t offsetVal = offsetToInt(offset);
            state.pc++;
            state.mem[state.reg[regAVal] + offsetVal] = state.reg[regBVal];
        }
        else if(!strcmp(opcode,"100")) //beq
        {
            char regA[4];
            char regB[4];

            regA[0] = numString[3];
            regA[1] = numString[4];
            regA[2] = numString[5];
            regA[3] = '\0';

            regB[0] = numString[6];
            regB[1] = numString[7];
            regB[2] = numString[8];
            regB[3] = '\0';

            char offset[17];
            for(int i = 15; i >=0; i--)
            {
                offset[i] = numString[9+i];
            }
            offset[16] = '\0';
            int regAVal = 0;
            int regBVal = 0;

            regAVal += (regA[0] == '1') ? 4 : 0;
            regAVal += (regA[1] == '1') ? 2 : 0;
            regAVal += (regA[2] == '1') ? 1 : 0;

            regBVal += (regB[0] == '1') ? 4 : 0;
            regBVal += (regB[1] == '1') ? 2 : 0;
            regBVal += (regB[2] == '1') ? 1 : 0;
            int16_t offsetVal = offsetToInt(offset);
            if(state.reg[regAVal] == state.reg[regBVal])
            {
                state.pc = state.pc + 1 + offsetVal;
            }
            else
            {
                state.pc++;
            }
        }
        else if(!strcmp(opcode,"101")) //jalr
        {
            char regA[4];
            char regB[4];

            regA[0] = numString[3];
            regA[1] = numString[4];
            regA[2] = numString[5];
            regA[3] = '\0';

            regB[0] = numString[6];
            regB[1] = numString[7];
            regB[2] = numString[8];
            regB[3] = '\0';

            int regAVal = 0;
            int regBVal = 0;

            regAVal += (regA[0] == '1') ? 4 : 0;
            regAVal += (regA[1] == '1') ? 2 : 0;
            regAVal += (regA[2] == '1') ? 1 : 0;

            regBVal += (regB[0] == '1') ? 4 : 0;
            regBVal += (regB[1] == '1') ? 2 : 0;
            regBVal += (regB[2] == '1') ? 1 : 0;
            int currentPC = state.pc;
            state.reg[regBVal] = currentPC + 1;
            state.pc = state.reg[regAVal];
        }
        else if(!strcmp(opcode,"111")) //noop
        {
            state.pc++;
        }
        else //halt
        {
            state.pc++;
            numInstructions++;
            printf("machine halted\n");
            printf("total of %d instructions executed\n",numInstructions);
            break;
        }
        numInstructions++;
    }
    printf("final state of machine:\n");
    printState(&state);

    //Your code ends here! 
    return(0);
}

/*
* DO NOT MODIFY ANY OF THE CODE BELOW. 
*/

void printState(stateType *statePtr) {
    int i;
    printf("\n@@@\nstate:\n");
    printf("\tpc %d\n", statePtr->pc);
    printf("\tmemory:\n");
    for (i=0; i<statePtr->numMemory; i++) {
        printf("\t\tmem[ %d ] 0x%08X\n", i, statePtr->mem[i]);
    }
    printf("\tregisters:\n");
	  for (i=0; i<NUMREGS; i++) {
	      printf("\t\treg[ %d ] %d\n", i, statePtr->reg[i]);
	  }
    printf("end state\n");
}

// convert a 16-bit number into a 32-bit Linux integer
// static inline int convertNum(int num) 
// {
//     return num - ( (num & (1<<15)) ? 1<<16 : 0 );
// }

/*
* Write any helper functions that you wish down here. 
*/

char* binaryString(int num)
{
static char numString[25];
int bits = 25;
for (int i = bits - 1; i >= 0; i--) {
        numString[i] = (num & 1) ? '1' : '0'; // Check the least significant bit
        num >>= 1; // Shift right by 1 to process the next bit
    }
return numString;
}

int offsetToInt(const char* binary) {
    int length = strlen(binary);
    int result = 0;
    int isNegative = (binary[0] == '1');
    // Iterate through each character in the binary string
    for (int i = 0; i < length; i++) {
        // Shift the result left by 1 (equivalent to multiplying by 2)
        result <<= 1;
        
        // Add the current bit (either 0 or 1) to the result
        if (binary[i] == '1') {
            result |= 1;  // Set the last bit to 1 if the character is '1'
        }
    }

    // If the number is negative, we need to convert it to two's complement
    if (isNegative) {
        int maxVal = 1 << length;  // 2^n, where n is the number of bits
        result -= maxVal;  // Subtract 2^n to get the negative value
    }

    return result;
}