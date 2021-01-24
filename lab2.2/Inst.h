#pragma once
#ifndef INST_H
#define INST_H

/******Type: R ********/
#define OP_R	0x33
#define	OP_RW	0x3B

/******Type: I ********/
#define	OP_IL	0x03
#define	OP_IR	0x13
#define	OP_IRW	0x1B
#define	OP_IJL	0x67	
#define	OP_ICAL	0x73

/******Type: S *******/
#define	OP_S	0x23

/******Type: SB *******/
#define	OP_SB	0x63

/****Type: U & UJ ******/
#define	OP_UIPC	0x17	
#define	OP_UI	0x37
#define	OP_UJ	0x6f

/******** FUNCT3 **********/
//Type: R
#define F3_ADD	0X00
#define F3_MUL	0X00
#define F3_SUB	0X00
#define F3_SLL	0X01
#define F3_MULH	0X01
#define F3_SLT	0X02
#define F3_XOR	0X04
#define F3_DIV	0X04
#define F3_SRL	0X05
#define F3_SRA	0X05
#define F3_OR	0X06
#define F3_REM	0X06
#define F3_AND	0X07

#define F3_ADDW	0X00
#define F3_SUBW	0X00
#define F3_MULW	0X00
#define F3_DIVW	0X04
#define F3_REMW	0X06

//Type I
#define F3_LB	0X00
#define F3_LH	0X01
#define F3_LW	0X02
#define F3_LD	0X03
#define F3_ADDI	0X00
#define F3_SLLI	0X01
#define F3_SLTI	0X02
#define F3_XORI	0X04
#define F3_SRLI	0X05
#define F3_SRAI	0X05
#define F3_ORI	0X06
#define F3_ANDI	0X07
#define F3_ADDIW	0X00
#define F3_JALR	0X00
#define F3_ECALL	0X00

//Type: S
#define F3_SB	0X00
#define F3_SH	0X01
#define F3_SW	0X02
#define F3_SD	0X03

//Type: SB
#define F3_BEQ	0X00
#define F3_BNE	0X01
#define F3_BLT	0X04
#define F3_BGE	0X05
#define F3_AUIPC	0X17
#define F3_LUI	0X17
#define F3_JAL	0X6f
/******** FUNCT7 **********/
/******** FUNCT3 **********/
//Type: R
#define F7_ADD	0X00
#define F7_MUL	0X01
#define F7_SUB	0X20
#define F7_SLL	0X00
#define F7_MULH	0X01
#define F7_SLT	0X00
#define F7_XOR	0X00
#define F7_DIV	0X01
#define F7_SRL	0X00
#define F7_SRA	0X20
#define F7_OR	0X00
#define F7_REM	0X01
#define F7_AND	0X00

#define F7_ADDW	0X00
#define F7_SUBW	0X20
#define F7_MULW	0X01
#define F7_DIVW	0X01
#define F7_REMW	0X01

//Type: I
#define F7_SLLI	0X00
#define F7_SRLI	0X00
#define F7_SRAI	0X00

#define F7_ECALL	0X00

/*struct INST
{
	unsigned char b[4];
};*/
typedef int INST;
typedef long long REG;

typedef long long DATA;
typedef long long ADDR;
typedef int REGADDR;


#endif // !INST_H