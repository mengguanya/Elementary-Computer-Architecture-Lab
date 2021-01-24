#include"pipelineSim.h"
#include"translate.h"
InstFetch::InstFetch() {
	PC = 0;
    stall = 0;
	branch2if = { 0,0 };
	if2id = { 0,0 };
	if2cache = { 0,0,0 };
}

bool InstFetch::checkADDR() {
	if ((PC < 0)||(PC >= INSTCACHESIZE)) {
		//instAddrError(PC);
		cout << "inst address error" << endl;
		return 0;
	}
	else if ((branch2if.branchTargetAddress < 0) || (branch2if.branchTargetAddress >= INSTCACHESIZE)) {
		//instAddrError(branch2if.branchTargetAddress);
		cout << "inst address error" << endl;
		return 0;
	}
	return 0;
}

ADDR InstFetch::branchJudge() {
    if (branch2if.branchFlag) {
        if (PC != branch2if.branchTargetAddress / 4 + 1) {
            PC = branch2if.branchTargetAddress / 4;
        }
        branch2if.branchFlag = false;
    }
    return PC;
}

void InstFetch::getInst() {
	checkADDR();
	//if2cache = { 0,branchJudge(),1 };
	if2cache.addr = branchJudge();
	if2cache.enable = 1;
	PC++;
}

INSTCACHE::INSTCACHE() {
	memset(instCache, 0, sizeof(INST) * INSTCACHESIZE);
	elfFile = fopen("bubbleSort", "r");
	//while(fscanf(elfFile, "%x", instCache));
	//fscanf(elfFile, "%x", instCache);
    fseek(elfFile, 0x19c, SEEK_SET);
	fread(instCache, sizeof(INST), 78, elfFile);
}

INST INSTCACHE::getInst(ADDR addr) {
	return instCache[addr];
}

InstDecode::InstDecode() {
    stall = false;
    id2exe.enLoad = false;
    id2exe.enStore = false;
    id2exe.enWrite = false;
    
    id2reg.Enable1 = false;
    id2reg.Enable2 = false;
    id2reg.enWrite = false;
    
    id2branch.addr = 0;
    id2branch.enable = false;

    isBranch = false;
}

Reg::Reg() {
    id2reg.Enable1 = false;
    id2reg.Enable2 = false;
    id2reg.enWrite = false;
    wb2reg.enable = false;
    stallQuest = 0;
    memset(reg, 0, sizeof(reg));
    memset(flag, 0, sizeof(flag));
    reg[2] = 1000;
}

void InstDecode::analysisInst() {
	func7 = getbit(if2id.Inst, 25, 31);
	rs2 = getbit(if2id.Inst, 20, 24);
	rs1 = getbit(if2id.Inst, 15, 19);
	func3 = getbit(if2id.Inst, 12, 14);
	rd = getbit(if2id.Inst, 7, 11);
	OP = getbit(if2id.Inst, 0, 6);
    shamt = ext_signed(getbit(if2id.Inst, 20, 24), 5, 64);

    imm12 = ext_signed(getbit(if2id.Inst, 20, 31), 12, 64);
    imm20 = ext_signed(getbit(if2id.Inst, 12, 31), 20, 64);
    imm7 = ext_signed(getbit(if2id.Inst, 25, 31), 7, 64);
    imm5 = ext_signed(getbit(if2id.Inst, 7, 11), 5, 64);
	
	id2reg.readAddr1 = rs1;
	id2reg.readAddr2 = rs2;
    id2reg.writeAddr = rd;
}

void InstDecode::getRegAdd() {
    if ((OP == OP_R) || (OP == OP_RW)) {
        id2reg.Enable1 = true;
        id2reg.Enable2 = true;
        id2reg.enWrite = true;
    }
    else if ((OP == OP_S) || (OP == OP_SB)) {
        id2reg.Enable1 = true;
        id2reg.Enable2 = true;
        id2reg.enWrite = false;
    }
    else if ((OP == OP_IL)||(OP == OP_IR) || (OP == OP_IRW) || (OP == OP_IJL) || (OP == OP_ICAL)) {
        id2reg.Enable1 = true;
        id2reg.Enable2 = false;
        id2reg.enWrite = true;
    }
    else if ((OP == OP_UI) || (OP == OP_UIPC) || (OP == OP_UJ)) {
        id2reg.Enable1 = false;
        id2reg.Enable2 = false;
        id2reg.enWrite = true;
    }
    else {
        id2reg.Enable1 = false;
        id2reg.Enable2 = false;
        id2reg.enWrite = false;
    }
}

void InstDecode::getData() {

	id2exe.func3 = func3;
	id2exe.func7 = func7;
	id2exe.OP = OP;
	id2exe.PC = if2id.PC;
	id2exe.Inst = if2id.Inst;

    id2exe.enLoad = false;
    id2exe.enStore = false;

	if ((OP == OP_R)||(OP == OP_RW)) {
		id2exe.data1 = id2reg.readData1;
		id2exe.data2 = id2reg.readData2;
		
		id2exe.enWrite = true;
		id2exe.writeReg = rd;
	}
	else if (OP == OP_IL) {
		id2exe.data1 = id2reg.readData1;
		id2exe.data2 = imm12;//offset == imm12

        id2exe.enLoad = true;

		id2exe.enWrite = true;
		id2exe.writeReg = rd;
	}
	else if ((OP == OP_IR)||(OP == OP_IRW)||(OP == OP_IJL)||(OP == OP_ICAL)) {
		id2exe.data1 = id2reg.readData1;
		id2exe.data2 = imm12;

		id2exe.enWrite = true;
		id2exe.writeReg = rd;
        
        if ((OP == OP_IJL) && (func3 == F3_JALR)) {
            id2branch.addr = id2exe.data1 + id2exe.data2 + ((id2exe.data2 % 2) ? 0 : -1);
            id2branch.enable = true;

            isBranch = true;
            branchAddr = id2branch.addr;
        }
	}
	else if (OP == OP_S) {
		id2exe.data1 = id2reg.readData1;
		id2exe.data2 = id2reg.readData2;
        id2exe.offset = (imm7 << 5) + imm5;
        //id2exe.offset = ext_signed(id2exe.offset, 12, 64);

        id2exe.enStore = true;

		id2exe.enWrite = false;
		id2exe.writeReg = 0;
	}
	else if (OP == OP_SB) {
		id2exe.data1 = id2reg.readData1;
		id2exe.data2 = id2reg.readData2;
        id2exe.offset = (getbit(imm7, 6, 6) << 12) + (getbit(imm5, 0, 0) << 11) + (getbit(imm7, 0, 5) << 5) + (getbit(imm5, 1, 4) << 1);
        id2exe.offset = ext_signed(id2exe.offset, 13, 64);
		id2exe.enWrite = false;
		id2exe.writeReg = 0;

        isBranch = true;
        branchAddr = id2branch.addr;

        if (func3 == F3_BEQ) {
            if (id2exe.data1 == id2exe.data2) {
                id2branch.addr = (if2id.PC * 4 + id2exe.offset);
                id2branch.enable = true;
            }
        }
        else if (func3 == F3_BNE) {
            if (id2exe.data1 != id2exe.data2) {
                id2branch.addr = (if2id.PC * 4 + id2exe.offset);
                id2branch.enable = true;
            }
        }
        else if (func3 == F3_BLT){
            if (id2exe.data1 < id2exe.data2)
            {
                id2branch.addr = (if2id.PC * 4 + id2exe.offset);
                id2branch.enable = true;
            }
        }
        else if (func3 == F3_BGE) {
            if (id2exe.data1 >= id2exe.data2)
            {
                id2branch.addr = (if2id.PC * 4 + id2exe.offset);
                id2branch.enable = true;
            }
        }
	}
	else if ((OP == OP_UI)||(OP==OP_UIPC)) {
		id2exe.data1 = imm12;//offsey == imm12
		id2exe.data2 = 0;

		id2exe.enWrite = true;
		id2exe.writeReg = rd;
	}
    else if (OP == OP_UJ) {
        id2exe.data1= (getbit(imm20, 19, 19) << 20) + (getbit(imm20, 0, 7) << 12) + (getbit(imm20, 8, 8) << 11) + (getbit(imm20, 9, 18) << 1);
        id2exe.data1 = id2exe.data1;
        id2exe.enWrite = true;
        id2exe.writeReg = rd;

        id2branch.addr = if2id.PC * 4 + id2exe.data1;
        id2branch.enable = true;

        isBranch = true;
        branchAddr = id2branch.addr;
    }
    if (id2exe.enWrite) {
        id2reg.writeAddr = id2exe.writeReg;
        id2reg.enWrite = id2exe.enWrite;
    }
}

bool Reg::readReg() {
    if (id2reg.Enable1) {
        if (flag[id2reg.readAddr1] > 0) {
            //stall
            stallQuest++;
        }
        else {
            id2reg.readData1 = reg[id2reg.readAddr1];
        }
        id2reg.Enable1 = false;
    }
    if (id2reg.Enable2) {
        if (flag[id2reg.readAddr2] > 0) {
            //stall
            if (id2reg.readAddr1 != id2reg.readAddr2)
                stallQuest++;
        }
        else {
            id2reg.readData2 = reg[id2reg.readAddr2];
        }
        id2reg.Enable2 = false;
    }
    if (id2reg.enWrite) {
        if(id2reg.writeAddr!=0)
            flag[id2reg.writeAddr]++;
        /*cout <<"对寄存器" << id2reg.writeAddr<< "写请求之后的flag:" << endl;
        for (int i = 0; i < 32; i++)
            cout << flag[i] << "    ";
        cout << endl;*/
        
        id2reg.enWrite = false;
    }
    if (stallQuest > 0)
        return true;
    else
        return false;
}

void Reg::writeReg() {
    if (wb2reg.enable) {
        if (wb2reg.addr == 0)//不允许写0寄存器
            return;
        reg[wb2reg.addr] = wb2reg.data;
        //cout << "写" << wb2reg.addr << "号寄存器" << "为" << wb2reg.data << endl;
        if(flag[wb2reg.addr] > 0)
            flag[wb2reg.addr]--;
       /*cout << "对寄存器" << wb2reg.addr << "写之后的flag:" << endl;
        for (int i = 0; i < 32; i++)
            cout << flag[i] << "    ";
        cout << endl;*/
        wb2reg.enable = false;
        if(stallQuest > 0)
            stallQuest--;
    }
}

EXE::EXE() {
    stall = 0;
    id2exe.enLoad = false;
    id2exe.enStore = false;
    id2exe.enWrite = false;

    exe2Mem.enLoad = false;
    exe2Mem.enStore = false;
    exe2Mem.enWrite = false;
}

void EXE::exe() {
        exe2Mem.enWrite = id2exe.enWrite;
        exe2Mem.writeReg = id2exe.writeReg;
        exe2Mem.enLoad = id2exe.enLoad;
        exe2Mem.enStore = id2exe.enStore;
        exe2Mem.Inst = id2exe.Inst;
        exe2Mem.PC = id2exe.PC;
        exe2Mem.OPcode = id2exe.OP;
        if (id2exe.OP == OP_R)
        {
            //exe2Mem.writeReg = id2exe.writeReg;
            //exe2Mem.enWrite = id2exe.enWrite;
            if (id2exe.func3 == F3_ADD && id2exe.func7 == F7_ADD)
            {
                exe2Mem.regData = id2exe.data1 + id2exe.data2;
            }
            /*else if (id2exe.func3 == F3_MUL && id2exe.func7 == F7_MUL)
            {
                long long temp;
                temp = reg[rs1] * reg[rs2];
                reg[rd] = temp % 100000000;
            }*/
            else if (id2exe.func3 == F3_SUB && id2exe.func7 == F7_SUB)
            {
                exe2Mem.regData = id2exe.data1 - id2exe.data2;
            }
            else if (id2exe.func3 == F3_SLL && id2exe.func7 == F7_SLL)
            {
                exe2Mem.regData = id2exe.data1 << id2exe.data2;
            }
            /*else if (id2exe.func3 == F3_MULH && id2exe.func7 == F7_MULH)
            {
                long long temp;
                temp = reg[rs1] * reg[rs2];
                reg[rd] = temp / 100000000;
            }*/
            else if (id2exe.func3 == F3_SLT && id2exe.func7 == F7_SLT)
            {
                exe2Mem.regData = (id2exe.data1 < id2exe.data2) ? 1 : 0;               
            }
            else if (id2exe.func3 == F3_XOR && id2exe.func7 == F7_XOR)
            {
                exe2Mem.regData = id2exe.data1 ^ id2exe.data2;
            }
            /*else if (id2exe.func3 == F3_DIV && id2exe.func7 == F7_DIV)
            {
                reg[rd] = reg[rs1] / reg[rs2];
            }*/
            else if (id2exe.func3 == F3_SRL && id2exe.func7 == F7_SRL)
            {
                exe2Mem.regData = id2exe.data1 >> getbit(id2exe.data2, 0, 5);
            }
            else if (id2exe.func3 == F3_SRA && id2exe.func7 == F7_SRA)
            {
                int shift = getbit(id2exe.data2, 0, 5);
                int res_shift = id2exe.data1 >> shift;
                res_shift = ext_signed(res_shift, 64 - shift, 64);
                exe2Mem.regData = res_shift;
            }
            else if (id2exe.func3 == F3_OR && id2exe.func7 == F7_OR)
            {
                exe2Mem.regData = id2exe.data1 | id2exe.data2;
            }
            /*else if (id2exe.func3 == F3_REM && id2exe.func7 == F7_REM)
            {
                reg[rd] = reg[rs1] % reg[rs2];
            }*/
            else if (id2exe.func3 == F3_AND && id2exe.func7 == F7_AND)
            {
                exe2Mem.regData = id2exe.data1 & id2exe.data2;
            }
        }
        else if (id2exe.OP == OP_RW) {
            if ((id2exe.func3 == F3_ADDW) && (id2exe.func7 == F7_ADDW)) {
                exe2Mem.regData = getbit(id2exe.data1, 0, 31) + getbit(id2exe.data2, 0, 31);
            }
            if ((id2exe.func3 == F3_SUBW) && (id2exe.func7 == F7_SUBW)) {
                exe2Mem.regData = getbit(id2exe.data1, 0, 31) - getbit(id2exe.data2, 0, 31);

            }
            /*if ((id2exe.func3 == F3_MULW) && (id2exe.func7 == F7_MULW)) {
                reg[rd] = ext_signed(getbit(reg[rs1], 0, 31) * getbit(reg[rs2], 0, 31), 32, 64);
            }
            if ((id2exe.func3 == F3_DIVW) && (id2exe.func7 == F7_DIVW)) {
                reg[rd] = ext_signed(getbit(reg[rs1], 0, 31) / getbit(reg[rs2], 0, 31), 32, 64);
            }
            if ((id2exe.func3 == F3_REMW) && (id2exe.func7 == F7_REMW)) {
                reg[rd] = ext_signed(getbit(reg[rs1], 0, 31) % getbit(reg[rs2], 0, 31), 32, 64);
            }*/
        }
        else if (id2exe.OP == OP_IL)
        {
            //exe2Mem.enWrite = true;
            exe2Mem.writeReg = id2exe.writeReg;
            
            exe2Mem.memAddr = id2exe.data1 + id2exe.data2;
        }
        else if (id2exe.OP == OP_IR) {
            if (id2exe.func3 == F3_ADDI) {
                exe2Mem.regData = id2exe.data1 + id2exe.data2;
            }
            else if (id2exe.func3 == F3_SLLI && id2exe.func7 == F7_SLLI) {
                exe2Mem.regData = id2exe.data1 << id2exe.data2;
            }
            else if (id2exe.func3 == F3_SLTI && id2exe.func7 == F7_SLLI) {
                exe2Mem.regData = (id2exe.data1 < id2exe.data2) ? 1 : 0;
            }
            else if (id2exe.func3 == F3_XORI) {
                exe2Mem.regData = id2exe.data1 ^ id2exe.data2;
            }
            else if (id2exe.func3 == F3_SRLI && id2exe.func7 == F7_SRLI) {
                exe2Mem.regData = id2exe.data1 >> id2exe.data2;
            }
            else if (id2exe.func3 == F3_SRAI && id2exe.func7 == F7_SRAI) {
                exe2Mem.regData = ext_signed(id2exe.data1 >> id2exe.data2, 64 - id2exe.data2, 64);
            }
            else if (id2exe.func3 == F3_ORI) {
                exe2Mem.regData = id2exe.data1 | id2exe.data2;
            }
            else if (id2exe.func3 == F3_ANDI) {
                exe2Mem.regData = id2exe.data1 & id2exe.data2;
            }
        }
        else if (id2exe.OP == OP_IRW) {
            if (id2exe.func3 == F3_ADDIW) {
                exe2Mem.regData = id2exe.data1 + id2exe.data2;
            }
        }
        else if (id2exe.OP == OP_IJL) {
            if (id2exe.func3 == F3_JALR)
            {
                exe2Mem.regData = id2exe.PC + 4;
            }
        }
        else if (id2exe.OP == OP_ICAL) {
            if ((id2exe.func3 == F3_ECALL) && (id2exe.func7 == F7_ECALL)) {

            }
        }
        else if (id2exe.OP == OP_S) {
            exe2Mem.memAddr = id2exe.data1 + id2exe.offset;
            exe2Mem.memData = id2exe.data2;
        }
        
        else if (id2exe.OP == OP_UI)
        {
            if (id2exe.func3 == F3_AUIPC)
            {
                //exe2Mem.enWrite = id2exe.enWrite;
                //exe2Mem.writeReg = id2exe.writeReg;
                exe2Mem.regData = id2exe.PC + id2exe.data1 << 12;
            }
            else if (id2exe.func3 == F3_LUI)
            {
                //exe2Mem.enWrite = id2exe.enWrite;
                //exe2Mem.writeReg = id2exe.writeReg;
                exe2Mem.regData = id2exe.data1 << 12;
            }
        }
        else if (id2exe.OP == OP_UJ)
        {
            //exe2Mem.enWrite = id2exe.enWrite;
            //exe2Mem.writeReg = id2exe.writeReg;
            exe2Mem.regData = id2exe.PC + 4;
        }
}

BRANCHRPEDICTOR::BRANCHRPEDICTOR() {
    branch2if.branchFlag = false;
    branch2if.branchTargetAddress = 0;
}

void BRANCHRPEDICTOR::branchJudge() {
    branch2if.branchFlag = id2branch.enable;
    branch2if.branchTargetAddress = id2branch.addr;
    //reset the predictor
    id2branch.enable = false;
    id2branch.addr = 0;
}

MEM::MEM() {
    stall = false;
    exe2mem.enLoad = false;
    exe2mem.enStore = false;
    exe2mem.enWrite = false;

    mem2wb.enWrite = false;
    mem2cache.enRead = false;
    mem2cache.enWrite = false;
}

void MEM::loadStoreRequest() {
    mem2wb.PC = exe2mem.PC;
    mem2wb.Inst = exe2mem.Inst;
    mem2wb.Data = exe2mem.regData;
    mem2wb.regAddr = exe2mem.writeReg;
    mem2wb.enWrite = exe2mem.enWrite;
    if (exe2mem.enLoad) {
        mem2cache.enRead = true;
        mem2cache.addr = exe2mem.memAddr;
        //mem2cache.data = exe2mem.memData;
    }
    else if (exe2mem.enStore) {
        mem2cache.enWrite = true;
        mem2cache.addr = exe2mem.memAddr;
        mem2cache.data = exe2mem.memData;
        exe2mem.enStore = false;
    }
}

void MEM::loadReply() {
    if (exe2mem.enLoad) {
        mem2wb.Data = mem2cache.data;
        exe2mem.enLoad = false;
    }
}

void MemManager::RWCache() {
    if (mem2cache.addr >= DATACACHESIZE || mem2cache.addr < 0) {
        cout << "read addr error" << endl;
        return;
        //exit(0);
    }
    if(mem2cache.enRead) {
        mem2cache.data = dataCache[mem2cache.addr/4];
        mem2cache.enRead = false;
    }
    else if (mem2cache.enWrite) {
        dataCache[mem2cache.addr/4] = mem2cache.data;
        /*if ((mem2cache.data == 1)&&(mem2cache.addr/4==241)) {
            for (int i = 0; i < 48; i = i + 4)
            cout << dataCache[(928 + i) / 4] << "    ";
            getchar();
        }*/
        cout << "写" << mem2cache.addr/4 << "地址为" << mem2cache.data << endl;
                for (int i = 0; i < 100; i = i + 4)
            cout << dataCache[(928 + i)/4] << "    ";
        cout << endl;
        mem2cache.enWrite = false;
    }
}

WB::WB() {
    stall = false;
    mem2wb.enWrite = false;

    wb2reg.enable = false;
}

void WB::wb() {
    wb2reg.addr = mem2wb.regAddr;
    wb2reg.data = mem2wb.Data;
    wb2reg.enable = mem2wb.enWrite;
}

CONTROL::CONTROL() {
    for (int i = 0; i < 5; i++) {
        stall[i] = 0;
    }
}

MemManager::MemManager() {
    memset(dataCache, 0, sizeof(dataCache));
}

void CONTROL::Stall() {
    if (regSignal)
    {
        for (int i = 0; i < 5; i++) {
            stall[i] = 0;
        }
        stall[0] = true;
        stall[1] = true;
    }
    else
    {
        stall[0] = false;
        stall[1] = false;
    }
}

int Cycle = 0;
int Inst = 0;

InstFetch	IF;
InstDecode	ID;
EXE			Exe;
MEM			Mem;
WB			wb;
INSTCACHE	instCache;
Reg			reg;
BRANCHRPEDICTOR	branchPredictor;
MemManager  memmanager;
CONTROL     control;

void wbStage() {
    if (!wb.stall) {
        wb.mem2wb = Mem.mem2wb;
        wb.wb();

        reg.wb2reg = wb.wb2reg;
        reg.writeReg();
    }
}

void memStage() {
    if (!Mem.stall) {
        Mem.exe2mem = Exe.exe2Mem;
        Mem.loadStoreRequest();
        memmanager.mem2cache = Mem.mem2cache;
        memmanager.RWCache();
        Mem.mem2cache = memmanager.mem2cache;
        Mem.loadReply();
    }
}

void exeStage() {
    if (!Exe.stall) {
        Exe.id2exe = ID.id2exe;
        Exe.exe();
    }
}

void idStage() {
    if (!ID.stall) {
        ID.if2id = IF.if2id;
        if (ID.isBranch) {//the pre instruction is branch 
            if (ID.branchAddr != ID.if2id.PC) {
                ID.if2id.PC = 0;
                ID.if2id.Inst = 0;
            }
            ID.isBranch = false;
            ID.branchAddr = 0;
        }
        ID.analysisInst();
        ID.getRegAdd();
        reg.id2reg = ID.id2reg;
        int Stop = reg.readReg();
        ID.id2reg = reg.id2reg;
        ID.getData();
        if (Stop) {
            ID.id2exe.enLoad = false;
            ID.id2exe.enStore = false;
            ID.id2exe.enWrite = false;
            ID.id2exe.Inst = 0;
            ID.id2exe.PC = 0;

            if (ID.id2reg.enWrite) {
                reg.flag[ID.id2reg.writeAddr]--;
            }

            ID.id2branch.enable = false;
            ID.id2reg.Enable1 = false;
            ID.id2reg.Enable1 = false;
            ID.id2reg.enWrite = false;

            
        }
       
    }
    else {
        ID.id2reg.Enable1 = false;
        ID.id2reg.Enable2 = false;
        ID.id2reg.enWrite = false;

        ID.id2exe.enLoad = false;
        ID.id2exe.enStore = false;
        ID.id2exe.enWrite = false;

        ID.id2branch.enable = false;

        Exe.exe2Mem.enLoad = false;
        Exe.exe2Mem.enStore = false;
        Exe.exe2Mem.enWrite = false;

        ID.isBranch = false;
    }
}

void ifStage() {
    if (!IF.stall) {
        IF.branch2if = branchPredictor.branch2if;
        IF.getInst();
        IF.if2cache.Inst = instCache.getInst(IF.if2cache.addr);
        IF.if2id.Inst = IF.if2cache.Inst;
        IF.if2id.PC = IF.if2cache.addr;
        Inst++;
    }
    else {
        
    }
}

void controlModule() {
    control.regSignal = reg.stallQuest;
    control.Stall();
    IF.stall = control.stall[0];
    ID.stall = control.stall[1];
}

void branchpredictorModule() {
    branchPredictor.id2branch = ID.id2branch;
    branchPredictor.branchJudge();

    ID.id2branch.enable = false;
    ID.id2branch.addr = 0;
}

void pipelineSim() {
    wbStage();
    memStage();
    exeStage();
    controlModule();
    idStage();
    controlModule();
    ifStage();
    branchpredictorModule();
    //printf_s("%04x:%08x %04x:%08x %04x:%08x %04x:%08x %04x:%08x\n", IF.if2id.PC * 4+ 0x19c, IF.if2id.Inst, ID.id2exe.PC * 4 + 0x19c, ID.id2exe.Inst, Exe.exe2Mem.PC * 4 + 0x19c, Exe.exe2Mem.Inst, Mem.mem2wb.PC * 4 + 0x19c, Mem.mem2wb.Inst, wb.mem2wb.PC * 4+ 0x19c, wb.mem2wb.Inst);
    
    if (IF.if2id.PC * 4 + 0x19c == 0X22c) {
        for (int i = 0; i < 100; i = i + 4)
            cout << memmanager.dataCache[(928 + i)/4] << "    ";
        cout << endl;
        //exit(-1);
        getchar();
    }

    Cycle++;
	//printf("%x	%llx\n", ID.if2id.PC,ID.if2id.Inst);
	/*printf("PC:%08x	Inst:%08x	OP:%07x	func3:%03x	func7:%07x\n", ID.id2exe.PC, ID.id2exe.Inst, ID.id2exe.OP, ID.id2exe.func3, ID.id2exe.func7);
	printf("data1:%016x	data2:%016x	enWrite:%d	resReg:%05x", ID.id2exe.data1, ID.id2exe.data2, ID.id2exe.enWrite, ID.id2exe.writeReg);*/
	//char c = getchar()
    /*char c = ' ';
    if (c == 'q') {
        cout << "finished!" << endl;
        cout << "the number of Inst is " << Inst << "; \n" << "the number of Cycle is " << Cycle << endl;
        cout << "the CPI is" << ((1.0 * Cycle) / Inst) << endl;
    }*/
}

