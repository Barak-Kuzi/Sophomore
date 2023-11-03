#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int countDigit(const char *str);
int digcmp(const char *str1, const char *str2)
{
	int digCount1 = countDigit(str1);
	int digCount2 = countDigit(str2);
	if(digCount1 > digCount2)
		return 1;
	else if(digCount1 < digCount2)
		return 2;
	return 0;	
}

int countDigit(const char *str)
{
	int count = 0;
	const char* ptr = str;
	while(*ptr)
	{
		if(*ptr >= '0' && *ptr <= '9')
			count++;
		ptr++;	
	}
	return count;
}			


int main(int argc, char *argv[])
{
	if (argc != 3)
	{
		return -1;
	}
	return digcmp(argv[1], argv[2]);
}

