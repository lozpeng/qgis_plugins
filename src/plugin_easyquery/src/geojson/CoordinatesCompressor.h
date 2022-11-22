#pragma once

#include <stdio.h>
#include <math.h>
#include <string.h>

typedef unsigned char byte;
const int BYTE_BITS = 8;
const int INT_BIT_SIZE = sizeof(int) * BYTE_BITS;

/**
*  坐标数值压缩算法，对小整数
* 
* 在绝大多数情况下，我们使用到的整数，往往是比较小的。
往往又需要以整型（int）或长整型（long）为基本的传输类型，
在大多数系统中，以4Bytes和8Bytes来表示。
* 
*/
class IntCompressor
{
public:
	static const char* bytes_to_binary_str(byte* v, 
					int byte_count, char* out, int out_size);
	static const char* int_to_binary_str(int v, char* out, int out_size);
	/**
	* 对小整数进行处理
	*/
	static int int_to_zigzag(int n);
	static int zigzag_to_int(int zigzag);
	static int write_to_buffer(int zz, byte* buf, int size);
	static int read_from_buffer(byte* buf, int max_size);
};