#include "performance.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <pthread.h>

void do_work() {
	int c = 0;
	for (int i = 0; i < 1000; i++)
	{
		c++;
	}
	return;
}

int main(int argc, char **argv) {
	if (argc != 2) {
		fprintf(stderr, "Syntax: %s <N>\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	
	// parse N from the command line
	int n = atoi(argv[1]);
		
	// process reactivity
	printf("Process reactivity, %d tests...\n", n);
	unsigned long sum = 0;
	timer t;
	int i;

	begin(&t);
	for (i = 0; i < n; i++) {
		pid_t pid = fork();
		if (pid == -1) {
			fprintf(stderr, "Can't fork, error %d\n", errno);
			exit(EXIT_FAILURE);
		} else if (pid == 0) {
			do_work();
			_exit(EXIT_SUCCESS);
		} else {
			wait(0);
		}
	}
	end(&t);
	sum += get_microseconds(&t);

	// compute statistics
	unsigned long process_avg = sum / n;
	printf("Average: %lu microseconds\n", process_avg);
	
	return EXIT_SUCCESS;
}
