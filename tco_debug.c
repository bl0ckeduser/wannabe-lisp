#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wannabe-lisp.h"

/* ====================================================== */

/* 
 * The purpose of this code is to record the greatest
 * evaluation depth, or number of nested eval calls,
 * that occurs while evaluating a given expression.
 *
 * This functionality is made available through the 
 * special function  "max-space", which is fully described 
 * in the README file. The test file "tco-test.txt" demoes
 * this functionality by applying "max-space" to a small
 * variety of procedures, some of which should be TCOed.
 *
 * The purpose of all this is to test whether TCO is working.
 */

int frames = 0;
int frames_usage_max = 0;

void new_frame()
{
	++frames;
	if (frames > frames_usage_max) {
		frames_usage_max = frames;
	}
}

void close_frame()
{
	--frames;
}

/* ====================================================== */
