/*
 * Copyright (c) 2004, Bull S.A..  All rights reserved.
 * Created by: Sebastien Decugis

 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc., 59
 * Temple Place - Suite 330, Boston MA 02111-1307, USA.

 
 * This sample test aims to check the following assertion:
 *
 * When a thread other than the first thread of a process returns,
 * an implicit call to pthread_exit() is made with the returned value 
 * as a parameter.
 
 * The steps are:
 *
 * Same test as 1-2, 3-2, 4-1, but with return in place of pthread_exit.
 * The results shall be the same.
 
  */
 
 
 /* We are testing conformance to IEEE Std 1003.1, 2003 Edition */
 #define _POSIX_C_SOURCE 200112L
 
 /* Some routines are part of the XSI Extensions */
#ifndef WITHOUT_XOPEN
 #define _XOPEN_SOURCE	600
#endif

/********************************************************************************************/
/****************************** standard includes *****************************************/
/********************************************************************************************/
 #include <pthread.h>
 #include <stdarg.h>
 #include <stdio.h>
 #include <stdlib.h> 
 #include <string.h>
 #include <unistd.h>

 #include <sched.h>
 #include <semaphore.h>
 #include <errno.h>
 #include <assert.h>
/********************************************************************************************/
/******************************   Test framework   *****************************************/
/********************************************************************************************/
 #include "testfrmw.h"
 #include "testfrmw.c"
 /* This header is responsible for defining the following macros:
  * UNRESOLVED(ret, descr);  
  *    where descr is a description of the error and ret is an int (error code for example)
  * FAILED(descr);
  *    where descr is a short text saying why the test has failed.
  * PASSED();
  *    No parameter.
  * 
  * Both three macros shall terminate the calling process.
  * The testcase shall not terminate in any other maneer.
  * 
  * The other file defines the functions
  * void output_init()
  * void output(char * string, ...)
  * 
  * Those may be used to output information.
  */

/********************************************************************************************/
/********************************** Configuration ******************************************/
/********************************************************************************************/
#ifndef VERBOSE
#define VERBOSE 1
#endif

/********************************************************************************************/
/***********************************    Test cases  *****************************************/
/********************************************************************************************/

#include "threads_scenarii.c"

/* This file will define the following objects:
 * scenarii: array of struct __scenario type.
 * NSCENAR : macro giving the total # of scenarii
 * scenar_init(): function to call before use the scenarii array.
 * scenar_fini(): function to call after end of use of the scenarii array.
 */

/********************************************************************************************/
/***********************************    Real Test   *****************************************/
/********************************************************************************************/

pthread_key_t tld[3];

/* TLD destructor */
void destructor(void * arg)
{
	*(int *) arg += 1;
}

/* Thread routine */
void * threaded (void * arg)
{
	int ret = 0;
	
	ret = pthread_setspecific(tld[0], arg);
	if (ret != 0)  {  UNRESOLVED(ret, "Failed to set TLD data");  }
	
	ret = pthread_setspecific(tld[1], arg);
	if (ret != 0)  {  UNRESOLVED(ret, "Failed to set TLD data");  }
	
	ret = pthread_setspecific(tld[2], arg);
	if (ret != 0)  {  UNRESOLVED(ret, "Failed to set TLD data");  }
	
	return (void *)1;
}

/* Main routine */
int main (int argc, char *argv[])
{
	int ret=0;
	int ctl=0;
	void * rval;
	pthread_t child;
	int i,j;
	
	output_init();
	
	scenar_init();
	
	for (j=0; j<3; j++)
	{
		ret = pthread_key_create(&tld[j], destructor);
		if (ret != 0)  {  UNRESOLVED(ret, "Failed to create a TLD key");  }
	}
	
	for (i=0; i < NSCENAR; i++)
	{
		if (scenarii[i].detached == 0)
		{
			#if VERBOSE > 0
			output("-----\n");
			output("Starting test with scenario (%i): %s\n", i, scenarii[i].descr);
			#endif
			
			ctl=0;
			
			ret = pthread_create(&child, &scenarii[i].ta, threaded, &ctl);
			switch (scenarii[i].result)
			{
				case 0: /* Operation was expected to succeed */
					if (ret != 0)  {  UNRESOLVED(ret, "Failed to create this thread");  }
					break;
				
				case 1: /* Operation was expected to fail */
					if (ret == 0)  {  UNRESOLVED(-1, "An error was expected but the thread creation succeeded");  }
					break;
				
				case 2: /* We did not know the expected result */
				default:
					#if VERBOSE > 0
					if (ret == 0)
						{ output("Thread has been created successfully for this scenario\n"); }
					else
						{ output("Thread creation failed with the error: %s\n", strerror(ret)); }
					#endif
			}
			if (ret == 0) /* The new thread is running */
			{
				ret = pthread_join(child, &rval);
				if (ret != 0)  {  UNRESOLVED(ret, "Unable to join a thread");  }
					
				if (rval != (void *)1)
				{
					FAILED("pthread_join() did not retrieve the pthread_exit() param");
				}
				
				if (ctl != 3)  {  FAILED("The TLD destructors were not called");  }
			}
		}
	}
	
	for (j=0; j<3; j++)
	{
		ret = pthread_key_delete(tld[j]);
		if (ret != 0)  {  UNRESOLVED(ret, "Failed to delete a TLD key");  }
	}
	
	scenar_fini();
	#if VERBOSE > 0
	output("-----\n");
	output("All test data destroyed\n");
	output("Test PASSED\n");
	#endif
	
	PASSED;
	return 0;
}
