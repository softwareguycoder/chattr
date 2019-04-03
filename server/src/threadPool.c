/*
 * threadPool.c
 *
 *  Created on: Feb 3, 2019
 *      Author: bhart
 */

#include <client_list.h>
#include "stdafx.h"
#include "threadPool.h"


// Internal, file-scoped global counter of how many threads
int thread_count = 0;
