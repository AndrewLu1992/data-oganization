#ifndef DBERROR_H
#define DBERROR_H

#include "stdio.h"

/* module wide constants */
#define PAGE_SIZE 4096

/* return code definitions */
typedef int RC;

#define RC_OK 0
#define RC_FILE_NOT_FOUND 1
#define RC_FILE_HANDLE_NOT_INIT 2
#define RC_WRITE_FAILED 3
#define RC_READ_NON_EXISTING_PAGE 4

/* Defined for assignment 1 */
#define RC_FILE_CLOSE_FAILED 5
#define RC_FILE_DELETE_FAILED 6
#define INV_PARAMETER  7

/* error code for buffer management */
#define RC_BM_BP_NOT_FOUND 8
#define RC_BM_BP_REQUEST_PAGE_NUMBER_ILLEGAL 9
#define RC_BM_BP_PAGEFRAME_INIT_FAILED 10
#define RC_BM_BP_FLUSH_PAGE_FAILED 11
#define RC_BM_BP_NO_FRAME_TO_REP 12
#define RC_BM_BP_EMP_POOL       13
#define RC_BM_BP_CREATE_PAGE_FILE_FAILED  14
#define RC_BM_BP_INIT_BUFFER_POOL_FAILED 15 
#define RC_BM_BP_SHUTDOWN_BUFFER_POOL_FAILED 16

#define RC_RM_COMPARE_VALUE_OF_DIFFERENT_DATATYPE 200
#define RC_RM_EXPR_RESULT_IS_NOT_BOOLEAN 201
#define RC_RM_BOOLEAN_EXPR_ARG_IS_NOT_BOOLEAN 202
#define RC_RM_NO_MORE_TUPLES 203
#define RC_RM_NO_PRINT_FOR_DATATYPE 204
#define RC_RM_UNKOWN_DATATYPE 205

/* error code for Record Managerment*/
#define RC_REC_TABLE_CREATE_FAILED 206
#define RC_REC_CREATE_SCHEMA_FAILED 207
#define RC_REC_FILE_NOT_FOUND 207
#define RC_REC_SCHEMA_NOT_FOUND 207

#define RC_IM_KEY_NOT_FOUND 300
#define RC_IM_KEY_ALREADY_EXISTS 301
#define RC_IM_N_TO_LAGE 302
#define RC_IM_NO_MORE_ENTRIES 303


/* holder for error messages */
extern char *RC_message;

/* print a message to standard out describing the error */
extern void printError (RC error);
extern char *errorMessage (RC error);

#define THROW(rc,message) \
  do {			  \
    RC_message=message;	  \
    return rc;		  \
  } while (0)		  \

// check the return code and exit if it is an error
#define CHECK(code)							\
  do {									\
    int rc_internal = (code);						\
    if (rc_internal != RC_OK)						\
      {									\
	char *message = errorMessage(rc_internal);			\
	printf("[%s-L%i-%s] ERROR: Operation returned error: %s\n",__FILE__, __LINE__, __TIME__, message); \
	free(message);							\
	exit(1);							\
      }									\
  } while(0);


#endif
