#include "chip_8.h"


const unsigned int START_ADDRESS = 0x200;
const unsigned int FONTSET_START_ADDRESS = 0x50;

const unsigned int FONTSET_SIZE = 80;
uint8_t fontset[FONTSET_SIZE] = {
	0xF0, 0X90, 0X90, 0X90, 0XF0,
	0X20, 0X60, 0X20, 0X20, 0X70,
	0XF0, 0X10, 0XF0, 0X10, 0XF0,
	0XF0, 0X10, 0XF0, 0X10, 0XF0,
	0X90, 0X90, 0XF0, 0X10, 0X10,
	0XF0, 0X80, 0XF0, 0X10, 0XF0,
	0XF0, 0X80, 0XF0, 0X90, 0XF0,
	0XF0, 0X10, 0X20, 0X40, 0X40,
	0XF0, 0X90, 0XF0, 0X90, 0XF0,
	0XF0, 0X90, 0XF0, 0X10, 0XF0,
	0XF0, 0X90, 0XF0, 0X90, 0X90,
	0XE0, 0X90, 0XE0, 0X90, 0XE0,
	0XF0, 0X80, 0X80, 0X80, 0XF0,
	0XE0, 0X90, 0X90, 0X90, 0XE0,
	0XF0, 0X80, 0XF0, 0X80, 0XF0,
	0XF0, 0X80, 0XF0, 0X80, 0X80
};

chip_8::chip_8()
	: randGen(std::chrono::system_clock::now().time_since_epoch().count())
{
	pc = START_ADDRESS;

	for (unsigned int i = 0; i < FONTSET_SIZE; i++) {
		memory[FONTSET_START_ADDRESS + i] = fontset[i];
	}

	randByte = std::uniform_int_distribution<int>(0, 255U);

	table[0x0] = &chip_8::Table0;
	table[0x1] = &chip_8::OP_1nnn;
	table[0x2] = &chip_8::OP_2nnn;
	table[0x3] = &chip_8::OP_3xkk;
	table[0x4] = &chip_8::OP_4xkk;
	table[0x5] = &chip_8::OP_5xy0;
	table[0x6] = &chip_8::OP_6xkk;
	table[0x7] = &chip_8::OP_7xkk;
	table[0x8] = &chip_8::Table8;
	table[0x9] = &chip_8::OP_9xy0;
	table[0xA] = &chip_8::OP_Annn;
	table[0xB] = &chip_8::OP_Bnnn;
	table[0xC] = &chip_8::OP_Cxkk;
	table[0xD] = &chip_8::OP_Dxyn;
	table[0xE] = &chip_8::TableE;
	table[0xF] = &chip_8::TableF;

	table0[0x0] = &chip_8::OP_00E0;
	table0[0xE] = &chip_8::OP_00EE;

	table8[0x0] = &chip_8::OP_8xy0;
	table8[0x1] = &chip_8::OP_8xy1;
	table8[0x2] = &chip_8::OP_8xy2;
	table8[0x3] = &chip_8::OP_8xy3;
	table8[0x4] = &chip_8::OP_8xy4;
	table8[0x5] = &chip_8::OP_8xy5;
	table8[0x6] = &chip_8::OP_8xy6;
	table8[0x7] = &chip_8::OP_8xy7;
	table8[0xE] = &chip_8::OP_8xyE;

	tableE[0x0] = &chip_8::OP_ExA1;
	tableE[0xE] = &chip_8::OP_Ex9E;

	tableF[0x07] = &chip_8::OP_Fx07;
	tableF[0x0A] = &chip_8::OP_Fx0A;
	tableF[0x15] = &chip_8::OP_Fx15;
	tableF[0x18] = &chip_8::OP_Fx18;
	tableF[0x1E] = &chip_8::OP_Fx1E;
	tableF[0x29] = &chip_8::OP_Fx29;
	tableF[0x33] = &chip_8::OP_Fx33;
	tableF[0x55] = &chip_8::OP_Fx55;
	tableF[0x65] = &chip_8::OP_Fx65;

}

void chip_8::Cycle()
{
	opcode = (memory[pc] << 8u) | memory[pc + 1];

	pc += 2;

	((*this).*(table[(opcode & 0xF000u) >> 12u]))();

	if (delayTimer > 0) {
		--delayTimer;
	}

	if (soundTimer > 0) {
		--soundTimer;
	}
}

void chip_8::LoadROM(char const* filename)
{
	std::ifstream file(filename, std::ios::binary | std::ios::ate);

	if (file.is_open()) {
		std::streampos size = file.tellg();
		char* buffer = new char[size];

		file.seekg(0, std::ios::beg);
		file.read(buffer, size);
		file.close();

		for (long i = 0; i < size; i++) {
			memory[START_ADDRESS + i] = buffer[i];
		}

		delete[] buffer;
	}
}

void chip_8::OP_00E0()
{
	memset(video, 0, VIDEO_WIDTH * VIDEO_HEIGHT);
}

void chip_8::OP_00EE()
{
	--sp;
	pc = stack[sp];
}

void chip_8::OP_1nnn()
{
	uint16_t address = opcode & 0x0FFFu;
	pc = address;
}

void chip_8::OP_2nnn()
{
	uint16_t address = opcode & 0x0FFFu;

	stack[sp] = pc;
	++sp;
	pc = address;
}

void chip_8::OP_3xkk()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t byte = opcode & 0x00FFu;

	if (registers[Vx] == byte) {
		pc += 2;
	}
}

void chip_8::OP_4xkk()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t byte = opcode & 0x00FFu;

	if (registers[Vx] != byte) {
		pc += 2;
	}
}

void chip_8::OP_5xy0()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	if (registers[Vx] == registers[Vy]) {
		pc += 2;
	}
}

void chip_8::OP_6xkk()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t byte = opcode & 0x00FFu;

	registers[Vx] = byte;
}

void chip_8::OP_7xkk()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t byte = opcode & 0x00FFu;

	registers[Vx] += byte;
}

void chip_8::OP_8xy0()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	registers[Vx] = registers[Vy];
}

void chip_8::OP_8xy1()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	registers[Vx] |= registers[Vy];
}

void chip_8::OP_8xy2()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	registers[Vx] &= registers[Vy];
}

void chip_8::OP_8xy3()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	registers[Vx] ^= registers[Vy];
}

void chip_8::OP_8xy4()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	uint16_t sum = registers[Vx] + registers[Vy];

	if (sum > 255) {
		registers[0xF] = 1;
	}
	else {
		registers[0xF] = 0;
	}

	registers[Vx] = sum & 0xFFu;
}

void chip_8::OP_8xy5()
{
	uint8_t Vx = (opcode & 0x0F00) >> 8u;
	uint8_t Vy = (opcode & 0x00F0) >> 4u;

	if (registers[Vx] > registers[Vy]) {
		registers[0xF] = 1;
	}
	else {
		registers[0xF] = 0;
	}

	registers[Vx] -= registers[Vy];
}

void chip_8::OP_8xy6()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	registers[0xF] = registers[Vx] & 1u;

	registers[Vx] >>= 1;
}

void chip_8::OP_8xy7()
{
	uint8_t Vx = (opcode & 0x0F00) >> 8u;
	uint8_t Vy = (opcode & 0x00F0) >> 4u;

	if (registers[Vy] > registers[Vx]) {
		registers[0xF] = 1;
	}
	else {
		registers[0xF] = 0;
	}

	registers[Vx] = registers[Vy] - registers[Vx];
}

void chip_8::OP_8xyE()
{
	uint8_t Vx = (opcode & 0x0F00) >> 8u;

	registers[0xF] = (registers[Vx] & 0x80) >> 7u;

	registers[Vx] <<= 1;
}

void chip_8::OP_9xy0()
{
	uint8_t Vx = (opcode & 0x0F00) >> 8u;
	uint8_t Vy = (opcode & 0x00F0) >> 4u;

	if (registers[Vx] != registers[Vy]) {
		pc += 2;
	}
}

void chip_8::OP_Annn()
{
	uint16_t address = (opcode & 0x0FFFu);

	index = address;
}

void chip_8::OP_Bnnn()
{
	uint16_t address = opcode & 0x0FFFu;

	pc = registers[0x0] + address;
}

void chip_8::OP_Cxkk()
{
	uint8_t Vx = (opcode & 0x0F00) >> 8u;
	uint8_t byte = opcode & 0x00FFu;

	registers[Vx] = (static_cast<uint8_t>(randByte(randGen))) & byte;
}

void chip_8::OP_Dxyn()
{
	uint8_t Vx = (opcode & 0x0F00) >> 8u;
	uint8_t Vy = (opcode & 0x00F0) >> 4u;
	uint8_t height = opcode & 0x000Fu;

	uint8_t xPos = registers[Vx] % VIDEO_WIDTH;
	uint8_t yPos = registers[Vy] % VIDEO_HEIGHT;

	registers[0xF] = 0;

	for (unsigned int row = 0; row < height; ++row) {
		uint8_t spriteByte = memory[index + row];

		for (unsigned int col = 0; col < 8; ++col) {
			uint8_t spritePixel = spriteByte & (0x80u >> col);
			uint32_t* screenPixel = &video[(yPos + row) * VIDEO_WIDTH + (xPos + col)];

			if (spritePixel) {
				if (*screenPixel == 0xFFFFFFFF) {
					registers[0xF] = 1;
				}

				*screenPixel ^= 0xFFFFFFFF;
			}
		}
	}
}

void chip_8::OP_Ex9E()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	uint8_t key = registers[Vx];

	if (keypad[key]) {
		pc += 2;
	}
}

void chip_8::OP_ExA1()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	uint8_t key = registers[Vx];

	if (!keypad[key]) {
		pc += 2;
	}
}

void chip_8::OP_Fx07()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	registers[Vx] = delayTimer;
}

void chip_8::OP_Fx0A()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	if (keypad[0]) {
		registers[Vx] = 0;
	}
	else if (keypad[1]) {
		registers[Vx] = 1;
	}
	else if (keypad[2]) {
		registers[Vx] = 2;
	}
	else if (keypad[3]) {
		registers[Vx] = 3;
	}
	else if (keypad[4]) {
		registers[Vx] = 4;
	}
	else if (keypad[5]) {
		registers[Vx] = 5;
	}
	else if (keypad[6]) {
		registers[Vx] = 6;
	}
	else if (keypad[7]) {
		registers[Vx] = 7;
	}
	else if (keypad[8]) {
		registers[Vx] = 8;
	}
	else if (keypad[9]) {
		registers[Vx] = 9;
	}
	else if (keypad[10]) {
		registers[Vx] = 10;
	}
	else if (keypad[11]) {
		registers[Vx] = 11;
	}
	else if (keypad[12]) {
		registers[Vx] = 12;
	}
	else if (keypad[13]) {
		registers[Vx] = 13;
	}
	else if (keypad[14]) {
		registers[Vx] = 14;
	}
	else if (keypad[15]) {
		registers[Vx] = 15;
	}
	else {
		pc -= 2;
	}
}

void chip_8::OP_Fx15()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	delayTimer = registers[Vx];
}

void chip_8::OP_Fx18()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	soundTimer = registers[Vx];
}

void chip_8::OP_Fx1E()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	index += registers[Vx];
}

void chip_8::OP_Fx29()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t digit = registers[Vx];

	index = FONTSET_START_ADDRESS + (5 * digit);
}

void chip_8::OP_Fx33()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t value = registers[Vx];

	memory[index + 2] = value % 10;
	value /= 10;

	memory[index + 1] = value % 10;
	value /= 10;

	memory[index] = value % 10;
}

void chip_8::OP_Fx55()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	for (unsigned int i = 0; i <= Vx; i++) {
		memory[index + i] = registers[i];
	}
}

void chip_8::OP_Fx65()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	for (unsigned int i = 0; i <= Vx; i++) {
		registers[i] = memory[index + i];
	}
}
