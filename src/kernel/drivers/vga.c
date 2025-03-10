#include "../../lib/stddef.h"
#include "../../lib/stdint.h"
#include "../../lib/string.h"
#include "../interpreter.h"

enum vga_color {
	VGA_COLOR_BLACK = 0,
	VGA_COLOR_BLUE = 1,
	VGA_COLOR_GREEN = 2,
	VGA_COLOR_CYAN = 3,
	VGA_COLOR_RED = 4,
	VGA_COLOR_MAGENTA = 5,
	VGA_COLOR_BROWN = 6,
	VGA_COLOR_LIGHT_GREY = 7,
	VGA_COLOR_DARK_GREY = 8,
	VGA_COLOR_LIGHT_BLUE = 9,
	VGA_COLOR_LIGHT_GREEN = 10,
	VGA_COLOR_LIGHT_CYAN = 11,
	VGA_COLOR_LIGHT_RED = 12,
	VGA_COLOR_LIGHT_MAGENTA = 13,
	VGA_COLOR_LIGHT_BROWN = 14,
	VGA_COLOR_WHITE = 15,
};

uint16_t buffer[2080];

uint8_t background_color = VGA_COLOR_BLACK;

static inline uint16_t vga_entry(unsigned char uc, uint8_t color);
size_t strlen(const char* str);
void terminal_initialize(void);
void terminal_setcolor(uint8_t fg_color, uint8_t bg_color);
void terminal_putentryat(char c, uint8_t color, size_t x, size_t y);
void terminal_putchar(char c);
void print_terminal_name(void);
void terminal_write(const char* data, size_t size);
void terminal_writestring(const char* data);
void terminal_scroll(void);

static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg){
	return fg | bg << 4;
}
 
static inline uint16_t vga_entry(unsigned char uc, uint8_t color){
	return (uint16_t) uc | (uint16_t) color << 8;
}
 
size_t strlen(const char* str){
	size_t len = 0;
	while (str[len])
		len++;
	return len;
}
 
static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;
static uint16_t* const VGA_MEMORY = (uint16_t*) 0xB8000;
size_t terminal_row;
size_t terminal_column;
uint8_t terminal_color;
uint16_t* terminal_buffer;

 
void terminal_initialize(void){
	terminal_row = 0;
	terminal_column = 0;
	terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, background_color);
	terminal_buffer = VGA_MEMORY;
	for (size_t y = 0; y < VGA_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			const size_t index = y * VGA_WIDTH + x;
			terminal_buffer[index] = vga_entry(' ', terminal_color);
		}
	}
	print_terminal_name();
}
void print_terminal_name(){
	terminal_setcolor(VGA_COLOR_MAGENTA, background_color);
	terminal_writestring("GO");
	terminal_setcolor(VGA_COLOR_LIGHT_GREEN, background_color);
	terminal_writestring("#terminal> ");
	terminal_setcolor(VGA_COLOR_MAGENTA, background_color);
}
void terminal_setcolor(uint8_t fg_color, uint8_t bg_color){
	terminal_color = vga_entry_color(fg_color, bg_color);
}
 
void terminal_putentryat(char c, uint8_t color, size_t x, size_t y){
	const size_t index = y * VGA_WIDTH + x;
	if(index >= 1999) terminal_scroll();
	else terminal_buffer[index] = vga_entry(c, color);
}
 
void terminal_putchar(char c){
	switch((uint16_t)c){
	case '\n':
		terminal_row++;
		terminal_column = 0;
		/* terminal_writestring(interpreter_print());
		clear_buffer();
		print_terminal_name(); */
		if(*interpreter_print() == 'c' && *(interpreter_print()+1) == 'l'){
			clear_buffer();
			terminal_initialize();
		}else{
			terminal_writestring("Undefined Command");
			clear_buffer();
			terminal_row++;
			terminal_column = 0;
			print_terminal_name();
		} 
		break;
	case '\b':
		if(terminal_column != 13) terminal_column--;
		terminal_putentryat(' ', terminal_color, terminal_column, terminal_row);
		break;
	case '\t':
		terminal_column++;
		terminal_putentryat(' ', terminal_color, terminal_column, terminal_row);
		terminal_column++;
		terminal_putentryat(' ', terminal_color, terminal_column, terminal_row);
		terminal_column++;
		terminal_putentryat(' ', terminal_color, terminal_column, terminal_row);
		break;
	default:
		interpreter(c);
		terminal_putentryat(c, terminal_color, terminal_column, terminal_row);
		if(++terminal_column == VGA_WIDTH){
			terminal_column = 0;
			if(++terminal_row == VGA_HEIGHT){
				terminal_row = 0;
			}
		}
		break;
	} 
}
 
void terminal_write(const char* data, size_t size){
	for (size_t i = 0; i < size; i++)
		terminal_putchar(data[i]);
}
 
void terminal_writestring(const char* data){
	terminal_write(data, strlen(data));
}
void terminal_scroll(void){
	memcpy(buffer, (uint16_t*) 0xB8000, 4000);
	memcpy((uint16_t*) 0xB8000, buffer+80, 4000);
	terminal_buffer = VGA_MEMORY;
	for (size_t y = 0; y < VGA_HEIGHT; y++) {
		for (size_t x = 13; x < VGA_WIDTH; x++) {
			const size_t index = y * VGA_WIDTH + x;
			terminal_buffer[index] = vga_entry(terminal_buffer[index], terminal_color);
		}
	}
	terminal_row = 24;
	terminal_column = 0;
}
