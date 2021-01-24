#pragma once
#ifndef PIPELINESIM
#define PIPELINESIM

#include<iostream>

#include"Inst.h"
#include"signal.h"
#include"Auxi.h"
using namespace std;
//#include"errorProcess.h"

void pipelineSim();

#define INSTCACHESIZE 5000
#define	DATACACHESIZE 5000

class InstFetch
{
public:
	bool	stall;
	ADDR PC;
	InstFetch();
	BRANCH2IF	branch2if;
	IF2ID		if2id;
	IF2CACHE	if2cache;
	void getInst();
private:
	bool checkADDR();
	ADDR branchJudge();
};

class InstDecode
{
public:
	bool	stall;
	IF2ID	if2id;
	ID2EXE	id2exe;
	ID2REG	id2reg;
	ID2BRANCH id2branch;
	bool	isBranch;
	ADDR	branchAddr;
	InstDecode();
	void analysisInst();
	void getRegAdd();
	void getData();
private:
	//各个指令解析段
	unsigned int OP;
	unsigned int func3, func7;
	unsigned int shamt;
	unsigned int rs1, rs2, rd;
	long long imm12;
	long long imm20;
	long long imm7;
	long long imm5;

};

class EXE
{
public:
	EXE();
	bool		stall;
	ID2EXE		id2exe;
	EXE2MEM		exe2Mem;
	void exe();
private:
};

class MEM
{
public:
	MEM();
	bool		stall;
	EXE2MEM		exe2mem;
	MEM2CACHE	mem2cache;
	MEM2WB		mem2wb;
	void		loadStoreRequest();
	void		loadReply();
private:

};

class WB
{
public:
	WB();
	bool	stall;
	WB2REG	wb2reg;
	MEM2WB	mem2wb;
	void	wb();
private:

};

class Reg
{
public:
	Reg();
	REG		reg[32];
	int		flag[32];
	int		stallQuest;
	ID2REG	id2reg;
	WB2REG	wb2reg;
	bool	readReg();
	void	writeReg();
private:
};

class MemManager {
public:
	MemManager();
	MEM2CACHE	mem2cache;
	DATA		dataCache[DATACACHESIZE];
	void		RWCache();
private:

};

class BRANCHRPEDICTOR
{
public:
	BRANCHRPEDICTOR();
	BRANCH2IF	branch2if;
	ID2BRANCH	id2branch;
	void branchJudge();
private:

};

class INSTCACHE
{
public:
	INSTCACHE();

	INST	instCache[INSTCACHESIZE];
	INST	getInst(ADDR addr);
private:
	FILE* elfFile;
};

class CONTROL {
public:
	CONTROL();
	bool	stall[5];
	bool	regSignal;
	void	Stall();
};

#endif // !PIPELINESIM