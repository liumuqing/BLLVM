#include <stdio.h>
const char * first_func(const char ** argv, int i) {
	printf("test");
	return argv[i];
}
int main(int argc, char ** argv) {
	printf("%s %s", first_func(argv, argc-1));
}
