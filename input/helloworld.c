#include <stdio.h>
#include <unistd.h>

int main(int argc, char const *argv[])
{
	printf("Hello, waiting 2 sec!\n");
	sleep(2);
	return 0;
}