#ifndef NOLIBC
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#endif

void writestr(int fd, const char *str)
{
	write(fd, str, strlen(str));
}

void writeint(int fd, unsigned long x)
{
	writestr(fd, ltoa(x));
}

/* prints "$hdr: <$val>\n" */
void print_int(const char *hdr, int val)
{
	writestr(1, hdr);
	writestr(1, ": <");
	writeint(1, val);
	writestr(1, ">\n");
}

/* prints "$hdr: <$val>\n" */
void print_str(const char *hdr, const char *val)
{
	writestr(1, hdr);
	writestr(1, ": <");
	writestr(1, val);
	writestr(1, ">\n");
}

int main(int argc, char **argv, char **envp)
{
	const char *env = envp[0] ? envp[0] : "none";
	int ret;
	int err;

	print_int("pid ", getpid());
	print_int("arg1", argc > 1 ? atol(argv[1]) : 0);

	writestr(1, "msg : <");
	ret = write(1, "hello", 5);
	err = errno;
	writestr(1, ">\n");

	print_int("ret ", ret);
	print_int("err ", err);
	print_int("argc", argc);
	print_str("argv", argv[0]);
	print_str("envp", env);
	return 0;
}
