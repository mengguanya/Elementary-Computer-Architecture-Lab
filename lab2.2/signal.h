#pragma once
#ifndef SIGNAL
#define SIGNAL

#include"Inst.h"

struct IF2ID
{
	REGADDR	PC;
	INST	Inst;
};

struct ID2EXE
{
	REGADDR	PC;
	INST	Inst;

	int		OP,
			func3,
			func7;
	DATA	data1;// source data 
	DATA	data2;
	DATA	offset;
	
	REGADDR	writeReg; //the address of  register to write 
	bool	enWrite;	//whether  write register

	bool	enLoad;
	bool	enStore;
};

struct EXE2MEM
{
	REGADDR	PC;
	INST	Inst;

	int		OPcode;
	
	ADDR	memAddr;
	DATA	memData;
	bool	enLoad;
	bool	enStore;

	DATA	regData;
	REGADDR	writeReg; //the address of  register to write 
	bool	enWrite;	//whether  write register
};

struct MEM2WB
{
	ADDR	PC;
	INST	Inst;

	REGADDR	regAddr;
	DATA	Data;
	bool	enWrite;
};

struct IF2CACHE
{
	INST	Inst;
	ADDR	addr;
	bool	enable;
};

struct BRANCH2IF
{
	bool	branchFlag;
	ADDR	branchTargetAddress;
};

struct ID2REG
{
	DATA	readData1;
	REGADDR	readAddr1;
	bool	Enable1;
	
	DATA	readData2;
	REGADDR	readAddr2;
	bool	Enable2;

	bool	enWrite;
	REGADDR	writeAddr;
};

struct ID2BRANCH
{
	ADDR	addr;
	bool	enable;
};

struct MEM2CACHE
{
	DATA	data;
	ADDR	addr;
	bool	enRead;
	bool	enWrite;
};

struct WB2REG
{
	DATA	data;
	ADDR	addr;
	bool	enable;
};

#endif // !signal