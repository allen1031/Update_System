#include <stdio.h>
#include "test.h"

int main(){
	char *addr = getipaddr();
	printf("%s\n",addr);
}