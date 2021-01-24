#include"console.h"
using namespace std;

int main() {
	cout << "welcome to use the CPU simulator!\the Instruction set architecture: RISCV64I\nMicro architecture: 5-level pipelines" << endl;
	/*cout << "please input the file path of elf format" << endl;
	string	elfPath;
	cin >> elfPath;
	FILE* elfFile = fopen(elfPath.c_str(), "r");
	if (!elfFile) {
		//openElfError(elfFile);
		cout << "the elf file error" << endl;
		exit(0);
	}

	analysisElf(elfFile);*/
	while (true)
		pipelineSim();
	return 0;
}