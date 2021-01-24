#include"CPU_Inst_Sim.h"

//指令运行数
long long inst_num = 0;
long long endPC = 3;
FILE* Mem;
long long data_Mem[10000000];
void fetch_Inst(int &Inst) {
    /*if (fscanf(Mem, "%x", &Inst)==NULL) {
        printf("read file error!\n");
    }*/
    fread(&Inst, sizeof(Inst), 1, Mem);
    printf("%llx\n", PC);
}
long long read_Data(unsigned long long add) {
    printf("read: Mem[%llx] = %llx\n", add,data_Mem[add/4]);
    fflush(stdout);
    return data_Mem[add/4];
}
void write_Data(unsigned long long add, long long data) {
    printf("write: Mem[%llx] = %llx\n", add, data);
    fflush(stdout);
    data_Mem[add/4] = data;
}

void display_Mem() {
    unsigned long long add;
    unsigned long long len;
    scanf("%llx %llx", &add, &len);
    add = add;
    len = len / 4;
    for (int i = 0; i < len; i++) {
        unsigned long long temp;
        temp = read_Data(add);
        add = add + 4;
        printf("%lld    ", temp);
        fflush(stdout);
    }
    printf("\n");
    fflush(stdout);
}

void display_Reg() {
    for (int i = 0; i < 32; i++) {
        printf("REG [%d]: %lld    ", i, reg[i]);
        fflush(stdout);
        if (!(i % 4) && i != 0) {
            printf("\n");
            fflush(stdout);
        }
    }
    printf("\n");
    fflush(stdout);
}

void translate_Inst(int inst) {
    func7 = getbit(inst, 25, 31);
    rs2 = getbit(inst, 20, 24);
    rs1 = getbit(inst, 15, 19);
    func3 = getbit(inst, 12, 14);
    rd = getbit(inst, 7, 11);
    OP = getbit(inst, 0, 6);
    shamt = getbit(inst, 20, 24);

    imm12 = getbit(inst, 20, 31);
    imm20 = getbit(inst, 12, 31);
    imm7 = getbit(inst, 25, 31);
    imm5 = getbit(inst, 7, 11);
}

void execute_Inst()
{
    if (OP == OP_R)
    {
        if (func3 == F3_ADD && func7 == F7_ADD)
        {
            reg[rd] = reg[rs1] + reg[rs2];
        }
        else if (func3 == F3_MUL && func7 == F7_MUL)
        {
            long long temp;
            temp = reg[rs1] * reg[rs2];
            reg[rd] = temp % 100000000;
        }
        else if (func3 == F3_SUB && func7 == F7_SUB)
        {
            reg[rd] = reg[rs1] - reg[rs2];
        }
        else if (func3 == F3_SLL && func7 == F7_SLL)
        {
            reg[rd] = reg[rs1] << reg[rs2];
        }
        else if (func3 == F3_MULH && func7 == F7_MULH)
        {
            long long temp;
            temp = reg[rs1] * reg[rs2];
            reg[rd] = temp / 100000000;
        }
        else if (func3 == F3_SLT && func7 == F7_SLT)
        {
            reg[rd] = (reg[rs1] < reg[rs2]) ? 1 : 0;
        }
        else if (func3 == F3_XOR && func7 == F7_XOR)
        {
            reg[rd] = reg[rs1] ^ reg[rs2];
        }
        else if (func3 == F3_DIV && func7 == F7_DIV)
        {
            reg[rd] = reg[rs1] / reg[rs2];
        }
        else if (func3 == F3_SRL && func7 == F7_SRL)
        {
            reg[rd] = reg[rs1] >> getbit(reg[rs2], 0, 5);
        }
        else if (func3 == F3_SRA && func7 == F7_SRA)
        {
            int shift = getbit(reg[rs2], 0, 5);
            int res_shift = reg[rs1] >> shift;
            res_shift = ext_signed(res_shift, 64-shift, 64);
            reg[rd] = res_shift;
        }
        else if (func3 == F3_OR && func7 == F7_OR)
        {
            reg[rd] = reg[rs1] | reg[rs2];
        }
        else if (func3 == F3_REM && func7 == F7_REM)
        {
            reg[rd] = reg[rs1] % reg[rs2];
        }
        else if (func3 == F3_AND && func7 == F7_AND)
        {
            reg[rd] = reg[rs1] & reg[rs2];
        }
        PC = PC + 4;
    }
    else if (OP == OP_RW) {
        if ((func3 == F3_ADDW) && (func7 == F7_ADDW)) {
            reg[rd] = ext_signed(getbit(reg[rs1],0,31) + getbit(reg[rs2],0,31),32,64);
        }
        if ((func3 == F3_SUBW) && (func7 == F7_SUBW)) {
            reg[rd] = ext_signed(getbit(reg[rs1], 0, 31) - getbit(reg[rs2], 0, 31), 32, 64);
        }
        if ((func3 == F3_MULW) && (func7 == F7_MULW)) {
            reg[rd] = ext_signed(getbit(reg[rs1], 0, 31) * getbit(reg[rs2], 0, 31), 32, 64);
        }
        if ((func3 == F3_DIVW) && (func7 == F7_DIVW)) {
            reg[rd] = ext_signed(getbit(reg[rs1], 0, 31) / getbit(reg[rs2], 0, 31), 32, 64);
        }
        if ((func3 == F3_REMW) && (func7 == F7_REMW)) {
            reg[rd] = ext_signed(getbit(reg[rs1], 0, 31) % getbit(reg[rs2], 0, 31), 32, 64);
        }
        PC += 4;
    }
    else if (OP == OP_IL)
    {
        int offset = imm12;
        unsigned long long add = reg[rs1] + ext_signed(offset, 12, 64);
        unsigned long long temp;
        if (func3 == F3_LB)
        {
            reg[rd] = ext_signed(read_Data(add), 8, 64);
        }
        else if (func3 == F3_LH) {
            reg[rd] = ext_signed(read_Data(add), 16, 64);
        }
        else if (func3 == F3_LW) {
            reg[rd] = ext_signed(read_Data(add), 32, 64);
        }
        else if (func3 == F3_LD) {
            reg[rd] = read_Data(add);
        }
        PC = PC + 4;
    }
    else if (OP == OP_IR) {
        if (func3 == F3_ADDI) {
            reg[rd] = reg[rs1] + ext_signed(imm12,12,64);
        }
        else if(func3 == F3_SLLI && func7 == F7_SLLI) {
            reg[rd] = reg[rs1] << shamt;
        }
        else if (func3 == F3_SLTI && func7 == F7_SLLI) {
            if (reg[rs1] << ext_signed(imm12, 12, 64)) {
                reg[rd] = 1;
            }
            else
                reg[rd] = 0;
        }
        else if (func3 == F3_XORI) {
            reg[rd] = reg[rs1] ^ ext_signed(imm12, 12, 64);
        }
        else if (func3 == F3_SRLI && func7 == F7_SRLI) {
            reg[rd] = reg[rs1] >> shamt;
        }
        else if (func3 == F3_SRAI && func7 == F7_SRAI) {
            reg[rd] = ext_signed(reg[rs1] >> shamt,64-shamt,64);
        }
        else if (func3 == F3_ORI) {
            reg[rd] = reg[rs1] | ext_signed(imm12, 12, 64);
        }
        else if (func3 == F3_ANDI) {
            reg[rd] = reg[rs1] & ext_signed(imm12, 12, 64);
        }
        PC = PC + 4;
    }
    else if (OP == OP_IRW) {
    if (func3 == F3_ADDIW) {
        reg[rd] = ext_signed(reg[rs1] + ext_signed(imm12, 12, 64), 32, 64);
        }
        PC = PC + 4;
    }
    else if (OP == OP_IJL) {
        if (func3 == F3_JALR)
        {
            reg[rd] = PC + 4;
            PC = reg[rs1] + ext_signed(imm12, 12, 64);
            if (PC % 2) {
                PC = PC - 1;
            }
        }
    }
    else if (OP == OP_ICAL) {
        if (func3 == F3_ECALL && func7 == F7_ECALL) {

        }
    }
    else if (OP == OP_S) {
        int shift=(imm7 << 5) + imm5;
        long long add = ext_signed(shift, 12, 64);
        if (func3 == F3_SB) {
            write_Data(reg[rs1] + add, getbit(reg[rs2], 0, 7));
        }
        if (func3 == F3_SH) {
            write_Data(reg[rs1] + add, getbit(reg[rs2], 0, 15));
        }
        if (func3 == F3_SW) {
            write_Data(reg[rs1] + add, getbit(reg[rs2], 0, 31));
        }
        if (func3 == F3_SD) {
            write_Data(reg[rs1] + add, reg[rs2]);
        }
        PC = PC + 4;
    }
    else if (OP == OP_SB) {
        int shift = (getbit(imm7, 6, 6) << 12) + (getbit(imm5, 0, 0) << 11) + (getbit(imm7, 0, 5) << 5) + (getbit(imm5, 1, 4) << 1);
        shift = ext_signed(shift, 13, 32);
        if (func3 == F3_BEQ) {
            if (reg[rs1] == reg[rs2]) {
                PC += shift;
                fseek(Mem, PC, SEEK_SET);
            }
            else {
                PC += 4;
            }
        }
        else if (func3 == F3_BNE) {
            if (reg[rs1] != reg[rs2]) {
                PC += shift;
                fseek(Mem, PC, SEEK_SET);
            }
            else {
                PC += 4;
            }
        }
        else if (func3 == F3_BLT) {
            if (reg[rs1] < reg[rs2]) {
                PC += shift;
                fseek(Mem, PC, SEEK_SET);
            }
            else {
                PC += 4;
            }
        }
        else if (func3 == F3_BGE) {
            if (reg[rs1] >= reg[rs2]) {
                PC += shift;
                fseek(Mem, PC, SEEK_SET);
            }
            else {
                PC += 4;
            }
        }

    }
    else if (OP==OP_UI)
    {
        if (func3==F3_AUIPC)
        {
            reg[rd] = PC + ext_signed(imm20, 20, 64) << 12;
        }
        else if (func3==F3_LUI)
        {
            reg[rd] = ext_signed(imm20, 20, 64) << 12;
        }
        PC = PC + 4;
    }
    else if(OP == OP_UJ)
    {
            if (rd != 0) {
                reg[rd] = PC + 4;
            }
            int shift = (getbit(imm20, 19, 19) << 20) + (getbit(imm20, 0, 7) << 12) + (getbit(imm20, 8, 8) << 11) + (getbit(imm20, 9, 18) << 1);
            shift = ext_signed(shift, 21, 64);
            PC = shift + PC;
            fseek(Mem, PC, SEEK_SET);
    }
    /*
    else if (OP == OP_SCALL)//系统调用指令
    {
    if (func3 == F3_SCALL && func7 == F7_SCALL)
    {
        if (reg[17] == 64)////printf
        {
            int place = 0, c = 0;
            const void* t = &memory[reg[11] >> 2];
            reg[10] = write((unsigned int)reg[10], t, (unsigned int)reg[12]);
        }
        else if (reg[17] == 63)//scanf
        {

        }
        else if (reg[17] == 169)//time
        {

        }
        else if (reg[17] == 93)//exit
        {
            exit_flag = 1;
        }
        else
        {

        }
    }*/
    else {
        printf("func7: %x  rs2: %x  rs1: %x  func3: %x  rd: %x  OP: %x\n", func7, rs2, rs1, func3, rd, OP);
        fflush(stdout);
    }
    inst_num++;
}

int main(int argc, char* argv[]) {
    /*initial reg*/
    reg[1] = 0;
    reg[2] = 10000;

    bool flag = 0;
    Mem = fopen("../quicksort", "r");
    //int data = 0x00108133;
    //fwrite(&data, sizeof(int), 1, Mem);
    if (Mem == NULL) {
        printf("FILE ERROR\n");
        fflush(stdout);
        return 0;
    }
    fseek(Mem, 0x3d8, SEEK_SET);
        //char Inst[4];
        int Inst=0;
        char IN;
        while (PC!=0x45c)
        {
            fetch_Inst(Inst);
            printf("Mem[%llx]:%x\n", PC, Inst);
            fflush(stdout);
            translate_Inst(Inst);
            execute_Inst();
        }
        display_Reg();
        while(true)
            display_Mem();
        
        /*printf("Inst i: fatch inst and exe, Inst m addr len : show the memory, Inst r: show all reg64I \n");
        while (PC!=0x2d0) {
            scanf("%c", &IN);
            switch (IN)
            {
            case 'i':
                fetch_Inst(Inst);
                printf("Mem[%llx]:%x\n", PC,Inst);
                translate_Inst(Inst);
                execute_Inst();
                break;
            case 'm':
                display_Mem();
                break;
            case 'r':
                display_Reg();
                break;
            case 'q':
                flag = 1;
                break;
            default:
                break;
            }
            if (flag) {
                printf("finished!\n");
                break;
            }
            getchar();
        }*/
}