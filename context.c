#define _XOPEN_SOURCE 600
#include <stdio.h>
#include <stdlib.h>
#include <sys/ucontext.h>
#define MEM 64000
#include <stdio.h>
#include <ucontext.h>

static ucontext_t ctx[3];

static void f1 (void) {
    puts("start f1");
    swapcontext(&ctx[1], &ctx[2]);
    puts("finish f1");
}

static void f2 (void) {
    puts("start f2");
    swapcontext(&ctx[2], &ctx[1]);
    puts("finish f2");
}

int main (void) {
    char st1[8192];
    char st2[8192];

		/* Prepares ctx[1]. */
    getcontext(&ctx[1]);
		// Setup stack
    ctx[1].uc_stack.ss_sp = st1;
    ctx[1].uc_stack.ss_size = sizeof st1;
		// When ctx[1] ends swap to ctx[0]
    //ctx[1].uc_link = &ctx[0];
    makecontext(&ctx[1], f1, 0);

		/* Prepares ctx[2]. When ctx[2] ends swap to ctx[0]. */
    getcontext(&ctx[2]);
    ctx[2].uc_stack.ss_sp = st2;
    ctx[2].uc_stack.ss_size = sizeof st2;
		// When ctx[2] ends swap to ctx[1]
    ctx[2].uc_link = &ctx[1];
    // ctx[2].uc_link = &ctx[0]; If we do this, we never print "finish f1".
    makecontext(&ctx[2], f2, 0);

		/* Finally run ctx[2] */
    swapcontext(&ctx[0], &ctx[2]);
    return 0;
}