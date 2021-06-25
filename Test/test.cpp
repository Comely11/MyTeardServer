#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>

int main()
{
	FILE* fp;
	int ch;

	fp = fopen("file.txt", "w+");
	for (ch = 33; ch <= 100; ch++)
	{
		printf("%s", &ch);
		fputc(ch, fp);
	}
	fclose(fp);

	return(0);
}