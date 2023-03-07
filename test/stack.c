#include<stdio.h>
#include "../include/pgm/softfloat_v2.h"
#include<stdint.h>
#include <malloc.h>
#define test_depth 2048
#define stack_padding 0xabcd

void print_stack();

//用0xAA填充栈底512字节
void stack_test_begin(void)
{
	int i;
	uint16_t mem[test_depth];
	for(i = 0; i < test_depth; i++)
	{
		mem[i] = stack_padding;
	}
}
 
//检查栈底，返回栈大小
int stack_test_end(void)
{
	 int i;
	uint16_t mem[test_depth];
	for(i = 0; i < test_depth; i++)
	{
		if(mem[i] != stack_padding)
		{
			return (test_depth - i)*2;
		}
	}
	return 0;
}
 
 #define use_byte 100
void stack_use(void)
{
	 int i;
	char mem[use_byte];
	for(i=0; i<use_byte; i++)
	{
		mem[i] = 0;
	}

}
void wrap_stack_use(){
    stack_use();
}

void wrap_wrap_stack_use(){
     int i;
	char mem[50];
	for(i=0; i<50; i++)
	{
		mem[i] = 0;
	}
    wrap_stack_use();
}

void original(){
	float slope=0.00000275678;
	uint64_t key=15556879l;
	uint64_t seg_key=0;

	int64_t pos_key=(int64_t)(slope*(key-seg_key));
}

void soft(){
	float slope=0.00000275678;
	float32_t _slope;
	memcpy(&(_slope.v), &slope, sizeof(slope));
	uint64_t key=15556879l-0;

	int stack_size;
	stack_test_begin();

	int64_t pos=f32_to_i64(f32_mul(_slope, i64_to_f32(key)));
	stack_size = stack_test_end();
	printf("stack: %d\n", stack_size);
	printf("pos = %d\n", pos);
}

void print_stack(){
	int i;
	uint16_t mem[test_depth];
	for(i = 0; i < test_depth; i++)
	{
		printf("%x ", mem[i]);
	}
	printf("\n");
}

float mul(float a, float b){
	return a*b;
}

#define mul_micro(a, b) a*b
void func_stack_size_test(){
	float a=1.23;
	float b=6.78;
	float c=mul(a, b);
}
void func_stack_size_test1(){
	float a=1.23;
	float b=6.78;
	float c;
	c=a*b;
}

void func_stack_size_test2(){
	float a=1.23;
	float b=6.78;
	float c;
	c=mul_micro(a, b);
}
void test_size(){
	uint32_t *x[4]={0};
}

void get_avi_stack_size(){
	// int chars[100000]={0};
	char c=0;
	while(1){
		printf("stack size: %d B\n", (long)&c-(long)alloca(100));
	}
}



//测试
int main(void)
{
	soft();	
	return 0;
}
