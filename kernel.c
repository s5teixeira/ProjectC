/* Maria Bui, Ryan Valler, Steph Teixeira, Troy Josti */

void printString(char*);
void printChar(char);
void readString(char*);
void readSector(char*, int);
void readFile(char*, char*, int*);
void executeProgram(char*);
void terminate();
void handleInterrupt21(int, int, int, int);

void main() {
/*	char buffer[13312];
	int sectorsRead;
*/	makeInterrupt21();
	interrupt(0x21, 4, "shell", 0, 0);
/*	interrupt(0x21, 3, "messag", buffer, &sectorsRead);
	if (sectorsRead > 0)
		interrupt(0x21, 0, buffer, 0, 0);
	else
		interrupt(0x21, 0, "messag not found\r\n", 0, 0);
	interrupt(0x21, 4, "tstpr1", 0, 0);
	interrupt(0x21, 4, "tstpr2", 0, 0);
*/	while(1);
}

void printString(char* chars) {
	while (*chars != 0x0) {
		interrupt(0x10, 0xe*256+*chars, 0, 0, 0);
		chars++;
	}
}

void printChar(char c) {
	interrupt(0x10, 0xe*256+c, 0, 0, 0);
}

void readString(char* chars) {
	int i = 0;
	char key;

	while (i < 80) {
		key = interrupt(0x16, 0, 0, 0, 0);

		if (key == 0x8) {
			if (i != 0) {
				i--;
				interrupt(0x10, 0xe*256+0x8, 0, 0, 0);
				interrupt(0x10, 0xe*256+' ', 0, 0, 0);
				interrupt(0x10, 0xe*256+0x8, 0, 0, 0);
			}
		}
		else {
			chars[i] = key;

			if (key == 0xd) {
				interrupt(0x10, 0xe*256+0xd, 0, 0, 0);
				interrupt(0x10, 0xe*256+0xa, 0, 0, 0);
				chars[i+1] = 0xa;
				chars[i+2] = 0x0;
				return;
			}

			interrupt(0x10, 0xe*256+key, 0, 0, 0);
			i++;
		}
	}
}

void readSector(char* buffer, int sector) {
	interrupt(0x13, 0x2*256+1, buffer, 0*256+sector+1, 0*256+0x80);
}

void readFile(char* filename, char* buffer, int* sectorsRead) {
	char dir[512];
	int fileentry, i, match = 0;

	readSector(dir, 2);
	*sectorsRead = 0;

	for (fileentry = 0; fileentry < 512; fileentry += 32) {
/*		if (filename[0] == dir[fileentry+0] &&
			filename[1] == dir[fileentry+1] &&
			filename[2] == dir[fileentry+2] &&
			filename[3] == dir[fileentry+3] &&
			filename[4] == dir[fileentry+4] &&
			filename[5] == dir[fileentry+5]) {
			for (i = 6; i < 32; i++) {
				if (dir[fileentry+i] != 0) {
					readSector(buffer, dir[fileentry+i]);
					buffer += 512;
					(*sectorsRead)++;
				}
				else return;
			}
		}
*/		for (i = 0; i < 6; i++) {
			if (filename[i] == dir[fileentry+i]) {
				if (filename[i] == 0 && dir[fileentry+i] == 0) {
					for (i = 6; i < 32; i++) {
						if (dir[fileentry+i] != 0) {
							readSector(buffer, dir[fileentry+i]);
							buffer += 512;
							(*sectorsRead)++;
						}
						else return;
					}
				}
				if (i == 5) {
					for (i = 6; i < 32; i++) {
						if (dir[fileentry+i] != 0) {
							readSector(buffer, dir[fileentry+i]);
							buffer += 512;
							(*sectorsRead)++;
						}
						else return;
					}
				}
			}
			else break;
		}
	}
}

void executeProgram(char* name) {
	char buffer[13312];
	int sectorsRead, address;

	readFile(name, buffer, &sectorsRead);

	for (address = 0; address < 13312; address++)
		putInMemory(0x2000, address, buffer[address]);

	if (sectorsRead > 0)
		launchProgram(0x2000);
}

void terminate() {
	char shellname[6];

	shellname[0]='s';
	shellname[1]='h';
	shellname[2]='e';
	shellname[3]='l';
	shellname[4]='l';
	shellname[5]='\0';

	executeProgram(shellname);
}

void handleInterrupt21(int ax, int bx, int cx, int dx) {
	if (ax == 0)
		printString(bx);
	else if (ax == 1)
		readString(bx);
	else if (ax == 2)
		readSector(bx, cx);
	else if (ax == 3)
		readFile(bx, cx, dx);
	else if (ax == 4)
		executeProgram(bx);
	else if (ax == 5)
		terminate();
	else
		printString("function not found");
}
