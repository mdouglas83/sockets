#include <iostream>
#include <stdio.h>

int main (int argc, char *argv[]) {
	if (argc == 2) {
		FILE *ls = popen(argv[1], "r");
		char buf[256];
		while (fgets(buf, sizeof(buf), ls) != 0) {
	    	fputs(buf, stdout);
		}
		pclose(ls);
	}
}