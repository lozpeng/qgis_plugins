#pragma once

#include <stdio.h>
#include <math.h>
#include <string.h>

typedef unsigned char byte;
const int BYTE_BITS = 8;
const int INT_BIT_SIZE = sizeof(int) * BYTE_BITS;

/**
*  ������ֵѹ���㷨����С����
* 
* �ھ����������£�����ʹ�õ��������������ǱȽ�С�ġ�
��������Ҫ�����ͣ�int�������ͣ�long��Ϊ�����Ĵ������ͣ�
�ڴ����ϵͳ�У���4Bytes��8Bytes����ʾ��
* 
*/
class IntCompressor
{
public:
	static const char* bytes_to_binary_str(byte* v, 
					int byte_count, char* out, int out_size);
	static const char* int_to_binary_str(int v, char* out, int out_size);
	/**
	* ��С�������д���
	*/
	static int int_to_zigzag(int n);
	static int zigzag_to_int(int zigzag);
	static int write_to_buffer(int zz, byte* buf, int size);
	static int read_from_buffer(byte* buf, int max_size);
};