#include"Read_Elf.h"
#include"string"
FILE *elf=NULL;
Elf64_Ehdr elf64_hdr;

//Program headers
unsigned long long	padr=0;
unsigned short		psize=0;
unsigned int		pnum=0;

//Section Headers
unsigned long long	sadr=0;
unsigned short		ssize=0;
unsigned short		snum=0;

//Symbol table
unsigned int symnum=0;
unsigned int symadr=0;
unsigned int symsize=0;

//用于指示 包含节名称的字符串是第几个节（从零开始计数）
unsigned int index=0;

//字符串表在文件中地址，其内容包括.symtab和.debug节中的符号表
unsigned int stradr=0;

/*the following is writed by mengguanya*/
bool open_file(char *file_path)
{
	file = fopen(file_path, "rb");
	if (file)
		return true;
	else
	{
		printf("the file is not exist\n");
		return false;
	}
}

bool seek_addr(unsigned long long addr) {
	if (fseek(file, addr, SEEK_SET) != -1) {
		return true;
	}
	else
		return false;
}

void read_elf(char *file_path,char *res_path)
{
	if(!open_file(file_path))
		return ;

	elf = fopen(res_path, "w");

	fprintf(elf,"ELF Header:\n");
	read_Elf_header();

	fprintf(elf,"\n\nSection Headers:\n");
	read_elf_sections();

	fprintf(elf,"\n\nProgram Headers:\n");
	read_Phdr();

	fprintf(elf,"\n\nSymbol table:\n");
	read_symtable();

	fclose(elf);
}

void read_Elf_header()
{
	//file should be relocated
	fread(&elf64_hdr,1,sizeof(elf64_hdr),file);
		
	fprintf(elf," magic number:  ");
	for (int i = 0; i < 16; i++) {
		fprintf(elf, "%02x  ", elf64_hdr.e_ident[i]);
	}
	fprintf(elf, "\n");
	fprintf(elf," Class:				ELF64\n");
	
	fprintf(elf," Data:				little-endian\n");
		
	int version = *(int*)(&elf64_hdr.e_version);
	fprintf(elf," Version:				%u\n", version);

	fprintf(elf," OS/ABI:				System V ABI\n");
	
	fprintf(elf," ABI Version:			0\n");
	
	unsigned short type = *(unsigned short*)(&elf64_hdr.e_type);
	if (type == 1) {
		fprintf(elf, " Type:				REL\n");
	}
	else if (type == 2) {
		fprintf(elf, " Type:				EXEC\n");
	}
	else if (type == 3) {
		fprintf(elf, " Type:				DYN\n");
	}
	else {
		fprintf(elf, " Type:				UNKONWN\n");
	}

	unsigned short machine = *(unsigned short*)(&elf64_hdr.e_machine);
	if (machine == 0) {
		fprintf(elf, " Machine:				None\n");
	}
	else if(machine == 2){
		fprintf(elf, " Machine:				SPARC\n");
	}
	else if (machine == 3) {
		fprintf(elf, " Machine:				Intel x86\n");
	}
	else if (machine == 0xf3) {
		fprintf(elf, " Machine:				RISC-V\n");
	}
	else
	{
		fprintf(elf, " Machine:				UNKONWN\n");
	}
	int Version = *(int*)(&elf64_hdr.e_version);
	fprintf(elf," Version:				%d\n", Version);
	
	entry = *(unsigned long long*)(&elf64_hdr.e_entry);
	fprintf(elf," Entry point address:			0x%llx\n",entry);

	padr = *(unsigned long long*)(&elf64_hdr.e_phoff);
	fprintf(elf," Start of program headers:		%lld bytes into  file\n",padr);

	sadr = *(unsigned long long*)(&elf64_hdr.e_shoff);
	fprintf(elf," Start of section headers:		%lld bytes into  file\n",sadr);

	unsigned int flags = *(unsigned int*)(&elf64_hdr.e_flags);
	fprintf(elf," Flags:				0x%x\n",flags);

	unsigned short ehsize = *(unsigned short*)(&elf64_hdr.e_ehsize);
	fprintf(elf," Size of this header:			%d Bytes\n",ehsize);

	psize = *(unsigned short*)(&elf64_hdr.e_phentsize);
	fprintf(elf," Size of program headers:		%d Bytes\n",psize);

	pnum = *(unsigned short*)(&elf64_hdr.e_phnum);
	fprintf(elf," Number of program headers:		%d\n",pnum);

	ssize = *(unsigned short*)(&elf64_hdr.e_shentsize);
	fprintf(elf," Size of section headers:		%d Bytes\n",ssize);
	
	snum = *(unsigned short*)(&elf64_hdr.e_shnum);
	fprintf(elf," Number of section headers:		%d\n",snum);

	index = *(unsigned short*)(&elf64_hdr.e_shstrndx);
	fprintf(elf, " Section header string table index:	%u\n", index);
}
unsigned long long find_shstrtab(){
	unsigned long long addr_shstrtab = 0;
	Elf64_Shdr elf64_shdr;
	fseek(file,sadr+index*sizeof(Elf64_Shdr),SEEK_SET);
	fread(&elf64_shdr, sizeof(Elf64_Shdr), 1, file);
	addr_shstrtab = *(unsigned long long*) & elf64_shdr.sh_offset;
	//printf("addr_shstrtab %llx\n", addr_shstrtab);
	return addr_shstrtab;
}

void find_Name(char *name,unsigned long long addr_shstrtab,unsigned int name_addr) {

	unsigned long long name_str = addr_shstrtab + name_addr;
	fseek(file, name_str, SEEK_SET);
	int c = 0;
	fread(&name[c], 1, 1, file);
	while (name[c] != '\0') {
		c++;
		fread(&name[c], 1, 1, file);
	}
}

void read_elf_sections()
{
	Elf64_Shdr elf64_shdr;
		
	if (!seek_addr(sadr)) {
		printf("section address error!\n");
		return;
	}

	char sh_name[50];

	unsigned long long shstrtab = find_shstrtab();

	const char* SH_TYPE[] = { "NULL","PROGBITS","SYMTAB","STRTAB","RELA","HASH","DYNAMIC","NOTE","NOBITS","REL","SHLIB","DYNSYM","","","INIT_ARR","FINI_ARR"};
	fprintf(elf, "num	name	type		addr			offset			size			entsize			flags		link		info		addralign\n");
	for(int c=0;c<snum;c++)
	{
		fprintf(elf," [%3d]\n",c);

		fseek(file, sadr + c * sizeof(Elf64_Shdr), SEEK_SET);
		fread(&elf64_shdr, sizeof(elf64_shdr),1 , file);
		find_Name(sh_name, shstrtab,(*(unsigned int*)&elf64_shdr.sh_name));

		fprintf(elf,"	%s	", sh_name);
		if ((*(unsigned int*) & elf64_shdr.sh_type) <= 15)
			fprintf(elf, "%s	", SH_TYPE[*(unsigned int*) & elf64_shdr.sh_type]);
		else
			fprintf(elf, "0x%08x	", *(unsigned int*) & elf64_shdr.sh_type);
		
		unsigned long long sh_addr = *(unsigned long long*) & elf64_shdr.sh_addr;
		fprintf(elf,"0x%016llx	", sh_addr);
		
		unsigned long long sh_offset = *(unsigned long long*) & elf64_shdr.sh_offset;
		fprintf(elf,"0x%016llx	", sh_offset);

		unsigned long long sh_size = *(unsigned long long*) & elf64_shdr.sh_size;
		fprintf(elf,"0x%016llx	", sh_size);

		unsigned long long sh_entsize = *(unsigned long long*) &elf64_shdr.sh_entsize;
		fprintf(elf,"0x%016llx	", sh_entsize);

		unsigned long long sh_flags = *(unsigned long long*) & elf64_shdr.sh_flags;
		if (sh_flags == 0x0)
			fprintf(elf, "			");
		else if (sh_flags == 0x3) {
			fprintf(elf, "	W A		");
		}
		else if (sh_flags == 0x6) {
			fprintf(elf, "	A X		");
		}
		else if (sh_flags == 0X30) {
			fprintf(elf, "	M S		");
		}
		else
			fprintf(elf,"0x%016llx	", sh_flags);
		
		unsigned int sh_link = *(unsigned int*) & elf64_shdr.sh_link;
		fprintf(elf,"0x%08x	", sh_link);

		unsigned int sh_info = *(unsigned int*) & elf64_shdr.sh_info;
		fprintf(elf,"0x%08x	", sh_info);

		unsigned long long sh_addrallign = *(unsigned long long*) & elf64_shdr.sh_addralign;
		fprintf(elf,"0x016%llx	\n", sh_addrallign);
		
		if (strcmp(sh_name, ".symtab") == 0) {
			symadr = sh_offset;
			symsize = sh_size;
		}
		else if(strcmp(sh_name, ".strtab") == 0)
			stradr = sh_offset;
		else if (strcmp(sh_name, ".text") == 0) {
			cadr = sh_offset;
			csize = sh_size;
			vadr = sh_addr;
		}
 	}
}

void read_Phdr()
{
	fseek(file, padr, SEEK_SET);
	fprintf(elf,"  num	Type		Offset			VirtAddr			PhysAddr			FileSiz			MemSiz		Flags		Align\n");
	Elf64_Phdr elf64_phdr;
	for (int c = 0; c < pnum; c++)
	{
		fprintf(elf, " [%3d]\n", c);
		//file should be relocated
		fread(&elf64_phdr, 1, sizeof(elf64_phdr), file);

		const char* P_TYPE[] = { "NULL","LOAD","DYNAMIC","INTERP","NOTE","SHLIB","PHDR","LOPROC","HIPROC" };
		unsigned int p_type = *(unsigned int*)&(elf64_phdr.p_type);
		if (p_type <= 6) {
			fprintf(elf, "	%s	", P_TYPE[p_type]);
		}
		else if (p_type == 0x70000000)
			fprintf(elf, "	%s	", P_TYPE[7]);
		else if (p_type == 0x7fffffff)
			fprintf(elf, "	%s	", P_TYPE[8]);

		unsigned long long p_offset = *(unsigned long long*)&(elf64_phdr.p_offset);
		fprintf(elf, "  0x%016llx	", p_offset);

		unsigned long long p_vaddr = *(unsigned long long*)&(elf64_phdr.p_vaddr);
		fprintf(elf, "  0x%016llx	", p_vaddr);
		
		unsigned long long p_paddr = *(unsigned long long*)&(elf64_phdr.p_paddr);
		fprintf(elf, "  0x%016llx	", p_paddr);

		unsigned long long p_filesz= *(unsigned long long*)&(elf64_phdr.p_filesz);
		fprintf(elf, "  0x%016llx	", p_filesz);

		unsigned long long p_memsz = *(unsigned long long*)&(elf64_phdr.p_memsz);
		fprintf(elf, "  0x%016llx	", p_memsz);
		
		const char* P_FLAG[] = { "none","X","W","W X","R","R X","R W","R W X" };
		unsigned int p_flags = *(unsigned int*)&(elf64_phdr.p_flags);
		if (p_flags <= 7)
			fprintf(elf, "  %s	", P_FLAG[p_flags]);
		else
			fprintf(elf, "  UNKNOWN	");

		unsigned long long p_align = *(unsigned long long*)&(elf64_phdr.p_align);
		fprintf(elf, "  0x%016llx	\n", p_align);
	}
}

void read_symtable()
{
	Elf64_Sym elf64_sym;

	symnum = symsize / sizeof(elf64_sym);

	const char* ST_BIND[] = { "LOCAL","GLOBAL","WEAK" };
	const char* ST_TYPE[] = { "NOTYPE","OBJECT","FUNC","SECTION","FILE" };
	fprintf(elf, "num	value		size			type	bind	vis	ndx	name\n");
	fseek(file, symadr, SEEK_SET);
	for(int c=0;c<symnum;c++)
	{
		fseek(file, symadr + c * sizeof(elf64_sym), SEEK_SET);
		fprintf(elf," [%3d]   ",c);
		//file should be relocated
		fread(&elf64_sym,1,sizeof(elf64_sym),file);

		unsigned long long st_value = *(unsigned long long*)(&elf64_sym.st_value);
		fprintf(elf, "0x%016llx	", st_value);
		
		unsigned long long st_size = *(unsigned long long*)(&elf64_sym.st_size);
		fprintf(elf, "0x%016llx	", st_size);

		unsigned char st_info = *(unsigned char*)(&elf64_sym.st_info);
		unsigned short st_type = st_info % 0x10;
		unsigned short st_bind = st_info / 0x10;
		
		if ((st_type <= 4))
			fprintf(elf, "%s	", ST_TYPE[st_type]);
		else
			fprintf(elf, "0x%04x", st_type);

		if ((st_bind <= 2))
			fprintf(elf, "%s	", ST_BIND[st_bind]);
		else
			fprintf(elf, "0x%04x	", st_bind);

		fprintf(elf, "DEFAULT	");
		
		unsigned short st_shndx = *(unsigned short*)(&elf64_sym.st_shndx);
		if (st_shndx == 0xfff1) {
			fprintf(elf, "ABS	");
		}
		else if (st_shndx == 0xfff2) {
			fprintf(elf, "COMMON	");
		}
		else if (st_shndx == 0) {
			fprintf(elf, "UNDEF	");
		}
		else
			fprintf(elf, "0x%04x	", st_shndx);

		char name[50];
		unsigned int st_name = *(unsigned int*)(&elf64_sym.st_name);
		find_Name(name, stradr, st_name);
		fprintf(elf, "%s\n", name);
		//fprintf(elf,"\n");
		
		if (strcmp(name, "main")==0) {
			madr = st_value;
			endPC = st_value + st_size;
		}
		else if (strcmp(name, "__global_pointer$")==0) {
			gp = st_value;
		}
	}
}

int main(int argc, char * argv[]) {
	read_elf(argv[1], argv[2]);
	return 0;
}
