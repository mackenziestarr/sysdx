
#include <stdio.h>

int main(int argc, char const *argv[])
{
	int i;
	int para;
	for (i=0; i < 6; i++){
		para = atoi(argv[1]);
		printf("OP%d: [%d]\n", i+1 , para + (21 * i));
	}




	return 0;
}