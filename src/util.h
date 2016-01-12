#ifndef UTIL_H
#define UTIL_H
#include <stdio.h>
#include <stdlib.h>

typedef struct Poll {
	int content[33];
	int top;
} Poll;

void initPoll(Poll* poll) {
	poll->top = 0;
}

void pushPoll(Poll* poll, int reg) {
	poll->content[poll->top] = reg;
	poll->top++;
}

int popPoll(Poll* poll) {
	if(poll->top == 0) {
		printf("reg poll empty!\n");
		exit(1);
	}
	poll->top--;
	return poll->content[poll->top];
}
#endif
