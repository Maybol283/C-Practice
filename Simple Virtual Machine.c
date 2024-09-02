#include <stdio.h>
#include <stdint.h>
#include <signal.h>
/* windows only */
#include <Windows.h>
#include <conio.h>

enum
{
    R_R0 = 0,
    R_R1,
    R_R2,
    R_R3,
    R_R4,
    R_R5,
    R_R6,
    R_R7,
    R_PC, /* program counter */
    R_COND,
    R_COUNT
};


enum
{
    OP_BR = 0, /* branch */
    OP_ADD,    /* add  */
    OP_LD,     /* load */
    OP_ST,     /* store */
    OP_JSR,    /* jump register */
    OP_AND,    /* bitwise and */
    OP_LDR,    /* load register */
    OP_STR,    /* store register */
    OP_RTI,    /* unused */
    OP_NOT,    /* bitwise not */
    OP_LDI,    /* load indirect */
    OP_STI,    /* store indirect */
    OP_JMP,    /* jump */
    OP_RES,    /* reserved (unused) */
    OP_LEA,    /* load effective address */
    OP_TRAP    /* execute trap */
};

enum
{
    FL_POS = 1 << 0, /* P */
    FL_ZRO = 1 << 1, /* Z */
    FL_NEG = 1 << 2, /* N */
};

#define MEMORY_MAX (1 << 16)
uint16_t memory[MEMORY_MAX];  /* 65536 locations */
uint16_t reg[R_COUNT];

uint16_t sign_extend(uint16_t x, int bit_count)
{
    if ((x >> (bit_count - 1)) & 1) {
        x |= (0xFFFF << bit_count);
    }
    return x;
}

void update_flags(uint16_t r)
{
    if (reg[r] == 0)
    {
        reg[R_COND] = FL_ZRO;
    }
    else if (reg[r] >> 15) /* a 1 in the left-most bit indicates negative */
    {
        reg[R_COND] = FL_NEG;
    }
    else
    {
        reg[R_COND] = FL_POS;
    }
}

int main(int argc, const char* argv[])
{
    //@{Load Arguments}
        if (argc < 2)
    {
        /* show usage string */
        printf("lc3 [image-file1] ...\n");
        exit(2);
    }

    for (int j = 1; j < argc; ++j)
    {
        if (!read_image(argv[j]))
        {
            printf("failed to load image: %s\n", argv[j]);
            exit(1);
        }
    }
    //@{Setup}

    /* since exactly one condition flag should be set at any given time, set the Z flag */
    reg[R_COND] = FL_ZRO;

    /* set the PC to starting position */
    /* 0x3000 is the default */
    enum { PC_START = 0x3000 };
    reg[R_PC] = PC_START;

    int running = 1;
    while (running)
    {
        /* FETCH */
        uint16_t instr = mem_read(reg[R_PC]++);
        uint16_t op = instr >> 12;

        switch (op)
        {
            case OP_ADD:
            {
            /* destination register (DR) */
            uint16_t r0 = (instr >> 9) & 0x7;
            /* first operand (SR1) */
            uint16_t r1 = (instr >> 6) & 0x7;
            /* whether we are in immediate mode */
            uint16_t imm_flag = (instr >> 5) & 0x1;

            if (imm_flag)
            {
                uint16_t imm5 = sign_extend(instr & 0x1F, 5);
                reg[r0] = reg[r1] + imm5;
            }
            else
            {
                uint16_t r2 = instr & 0x7;
                reg[r0] = reg[r1] + reg[r2];
            }

            update_flags(r0);
            }
                break;
            case OP_AND:
            {
                /* destination register (DR) */
                uint16_t r0 = (instr >> 9) & 0x7;
                /* first operand (SR1) */
                uint16_t r1 = (instr >> 6) & 0x7;
                /* whether we are in immediate mode */
                uint16_t imm_flag = (instr >> 5) & 0x1;

                if (imm_flag)
                {
                    uint16_t imm5 = sign_extend(instr & 0x1F, 5);
                    reg[r0] = reg[r1] + imm5;
                }
                else
                {
                    uint16_t r2 = instr & 0x7;
                    reg[r0] = reg[r1] + reg[r2];
                }

                update_flags(r0);
            }
                break;
            case OP_NOT:
            {
                /* destination register (DR) */
                uint16_t r0 = (instr >> 9) & 0x7;
                /* first operand (SR1) */
                uint16_t r1 = (instr >> 6) & 0x7;

                reg[r0] = ~reg[r1];
            
                update_flags(r0);
            }
                break;
            case OP_BR:
            {
                /* sign extended program counter offset amount */
                uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
                //Take the condition flag from instructions
                uint16_t condition = (instr >> 9) & 0x7;
                if (condition & reg[R_COND])
                {
                    reg[R_PC] += pc_offset;
                }

            }
                break;
            case OP_JMP:
            {
                /* Base Register Jump Location Bit */
                uint16_t r1 = (instr >> 6) & 0x7;
                reg[R_PC] = reg[r1];
                
            }
                break;
            case OP_JSR:
            {   
                // Save the return address in R7
                reg[R_R7] = reg[R_PC];
                
                // Extract the base register (for JSRR)
                uint16_t r1 = (instr >> 6) & 0x7;
                
                // Sign-extend the 11-bit PC offset
                uint16_t pc_offset = sign_extend(instr & 0x7FF, 11);
                
                // Determine if it's JSR or JSRR
                if ((instr >> 11) & 0x1)
                {
                    // JSR: PC-relative addressing
                    reg[R_PC] += pc_offset;
                } 
                else 
                {
                    // JSRR: Register-based addressing
                    reg[R_PC] = reg[r1];
                }
            }
                break;
            case OP_LD:
            {   
                //Get destination reg from instructions
                uint16_t r0 = (instr >> 9) & 0x7;
                //Sign extend instruction to get offset value
                uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
                //Update selected reg with current Programme Counter + Offset
                reg[r0] = mem_read(reg[R_PC] + pc_offset);
                update_flags(r0);
            }
                break;
            case OP_LDI:
            {
                /* destination register (DR) */
                uint16_t r0 = (instr >> 9) & 0x7;
                /* PCoffset 9*/
                uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
                /* add pc_offset to the current PC, look at that memory location to get the final address */
                reg[r0] = mem_read(mem_read(reg[R_PC] + pc_offset));
                update_flags(r0);
            }
                break;
            case OP_LDR:
            {
                /* destination register (DR) */
                uint16_t r0 = (instr >> 9) & 0x7;
                /* Base Register Jump Location Bit */
                uint16_t r1 = (instr >> 6) & 0x7;
                /* sign extend pc_offset until 5*/
                uint16_t pc_offset = sign_extend(instr & 0x1F, 5);
                /* add pc_offset to the current PC, look at that memory location to get the final address */
                reg[r0] = mem_read(reg[r1] + pc_offset);
                update_flags(r0);
            }
                break;
            case OP_LEA:
                //@{LEA}
                break;
            case OP_ST:
                //@{ST}
                break;
            case OP_STI:
                //@{STI}
                break;
            case OP_STR:
                //@{STR}
                break;
            case OP_TRAP:
                //@{TRAP}
                break;
            case OP_RES:
            case OP_RTI:
            default:
                //@{BAD OPCODE}
                break;
        }
    }
    //@{Shutdown}
}