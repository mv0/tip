/*
 * We put all the bug config variables in this same object
 * file, so that programs can link against the config parser
 * without having to link against all the rest of bug.
 */
#include "cache.h"

const char *pager_program;
int pager_use_color = 1;
