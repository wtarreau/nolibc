#ifndef NOLIBC
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#endif

int main(int argc, char **argv, char **envp)
{
	int ret;
	int err;

	printf("pid : <%d>\n", getpid());
	printf("arg1: <%ld>\n", argc > 1 ? atol(argv[1]) : 0);
	ret = printf("msg : <%s>\n", argc > 2 ? argv[2] : "hello");
	err = errno;

	printf("ret : <%d>\n", ret);
	printf("err : <%d>\n", err);
	printf("argc: <%d>\n", argc);
	printf("argv: <%s>\n", argv[0]);
	printf("envp: <%s>\n", envp[0] ? envp[0] : "none");
	return 0;
}
