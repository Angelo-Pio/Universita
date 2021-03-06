#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "e.h"

void check_perror(int res, const char* msg);

#define BUF_SIZE 8192

int copy(const char* src, const char* dst) {

    int res;
    void* buf;

    // allocazione buffer
    buf = malloc(BUF_SIZE);
    if (buf == NULL) {
        fprintf(stderr, "malloc");
        exit(EXIT_FAILURE);
    }

    // open files
    int fdin = open(src, O_RDONLY);
    check_perror(fdin, "open");

    int fdout = open(dst, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    check_perror(fdout, "open");

    // copy loop
    while(1) {
        ssize_t r = read(fdin, buf, BUF_SIZE);
        check_perror(r, "read");
        if (r==0) break;
        
        ssize_t w = write(fdout, buf, r);
        check_perror(w, "write");
    }

    // close files
    res = close(fdin);
    check_perror(res, "close");

    res = close(fdout);
    check_perror(res, "close");
    
    // allocazione buffer
    free(buf);

    return EXIT_SUCCESS;
}

void check_perror(int res, const char* msg) {
    if (res != -1) return;  
    perror(msg);
    exit(EXIT_FAILURE);
}
