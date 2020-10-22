#pragma once
#include <cstdint>
#include <chrono>
#include <random>


const unsigned int KEY_COUNT = 16;
const unsigned int MEMORY_SIZE = 4096;
const unsigned int REGISTER_COUNT = 16;
const unsigned int STACK_LEVELS = 16;
const unsigned int VIDEO_HEIGHT = 32;
const unsigned int VIDEO_WIDTH = 64;


class chip_8
{
public:
	chip_8();

	void Cycle();

	void LoadROM(char const* filename);

	uint8_t keypad[KEY_COUNT]{};
	uint32_t video[VIDEO_WIDTH * VIDEO_HEIGHT]{};

private:
	void OP_00E0(); // CLS
	void OP_00EE(); // RET
	void OP_1nnn(); // JP addr
	void OP_2nnn(); // CALL addr
	void OP_3xkk(); // SE Vx, byte
	void OP_4xkk(); // SNE Vx, byte
	void OP_5xy0(); // SE Vx, Vy
	void OP_6xkk(); // LD Vx, byte
	void OP_7xkk(); // ADD Vx, byte
	void OP_8xy0(); // LD Vx, Vy
	void OP_8xy1(); // OR Vx, Vy
	void OP_8xy2(); // AND Vx, Vy
	void OP_8xy3(); // XOR Vx, Vy
	void OP_8xy4(); // ADD Vx, Vy
	void OP_8xy5(); // SUB Vx, Vy
	void OP_8xy6(); // SHR Vx
	void OP_8xy7(); // SUBN Vx, Vy
	void OP_8xyE(); // SHL Vx {, Vy}
	void OP_9xy0(); // SNE Vx, Vy
	void OP_Annn(); // LD I, addr
	void OP_Bnnn(); // JP V0, addr
	void OP_Cxkk(); // RND Vx, byte
	void OP_Dxyn(); // DRW Vx, Vy, nibble
	void OP_Ex9E(); // SKP Vx
	void OP_ExA1(); // SKNP Vx
	void OP_Fx07(); // LD Vx, DT
	void OP_Fx0A(); // LD Vx, k
	void OP_Fx15(); // LD DT, Vx
	void OP_Fx18(); // LD ST, Vx
	void OP_Fx1E(); // ADD I, Vx
	void OP_Fx29(); // LD F, Vx
	void OP_Fx33(); // LD B, Vx
	void OP_Fx55(); // LD [I], Vx
	void OP_Fx65(); // LD Vx, [I]


	uint8_t registers[REGISTER_COUNT]{};
	uint8_t memory[MEMORY_SIZE]{};
	uint16_t index{};
	uint16_t pc{};
	uint16_t stack[STACK_LEVELS]{};
	uint8_t sp{};
	uint8_t delayTimer{};
	uint8_t soundTimer{};
	uint16_t opcode{};

	std::default_random_engine randGen;
	std::uniform_int_distribution<uint8_t> randByte;


	void Table0() {
		((*this).*(table0[opcode & 0x000Fu]))();
	}
	void Table8() {
		((*this).*(table8[opcode & 0x000Fu]))();
	}
	void TableE() {
		((*this).*(tableE[opcode & 0x000Fu]))();
	}
	void TableF() {
		((*this).*(tableF[opcode & 0x000Fu]))();
	}
	void OP_NULL() {}

	typedef void (chip_8::* Chip8Func)();
	Chip8Func table[0xF + 1]{ &chip_8::OP_NULL };
	Chip8Func table0[0xE + 1]{ &chip_8::OP_NULL };
	Chip8Func table8[0xE + 1]{ &chip_8::OP_NULL };
	Chip8Func tableE[0xE + 1]{ &chip_8::OP_NULL };
	Chip8Func tableF[0x65 + 1]{ &chip_8::OP_NULL };
};

