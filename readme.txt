Assignment 3
	In this assignment you are creating a record manager. The record manager handles tables with a fixed schema. Clients can insert records, delete records, update records, and scan through the records in a table. 

Personnel information
	Ting Ma  <tma11@hawk.iit.edu>
	Shuo Yan<syan9@hawk.iit.edu >
	Congwei She<cshe@hawk.iit.edu >
	Chen Wang<Cwang90@hawk.iit.edu >

File list
	buffer_mgr_sts.c
	buffer_mgr_sts.h
	buffer_mgr.c
	buffer_mgr.h
	dberror.c
	dberror.h
	dt.h
	expr.c
	expr.h
	record_mgr.c
	record_mgr.h
	rm_serializer.c
	tables.h
	storage_mgr.c
	storage_mgr.h
	test_assign3_1.c
	test_assign3_1_modify.c
	test_helper.h
	test_expr.c
	readme.txt
	makefile	

Milestone
	3/3 meeting and talk about how to fulfill it
	3/4 beginning coding
	3/20 personal test and debug
	3/27 merge
	3/31 make readme.txt and add header

Installation instruction
	1. make test_assign3_1_modify
	2. make test_assign3_1
	3. ./test_assign3_1_modify
		xxxxxx
	4. ./test_assign3_1
		xxxxxx
	5.  make clean
		rm *.o test_assign3_1_modify test_assign3_1

Function description
/**************************************************************************
 • function name
   initTableHeader

 • description
   Inite table Header which store table informations
 
 • parameters
    char *name, Schema *schema, BM_BufferPool *bm
 
 • return value
    return ret
 ***************************************************************************/

/**************************************************************************
 • function name
   CreateRecordPage 

 • description
   Create the page which store the record
 
 • parameters
    int PageNum, RM_TableData *rel
 
 • return value
    return ret
 ***************************************************************************/

/**************************************************************************
 • function name
   CreateRecordPage 

 • description
   create table which store pages
 
 • parameters
    char *name, Schema *schema
 
 • return value
    return ret
 ***************************************************************************/

/**************************************************************************
 • function name
   parsePageHeader

 • description
   parse page header which used to fetch table information
 
 • parameters
    char * page, Schema *schema
 
 • return value
    return ret
 ***************************************************************************/

/**************************************************************************
 • function name
   openTable

 • description
   get table information from the disk and set the information to the rel 
 
 • parameters
    RM_TableData *rel, char *name
 
 • return value
    return ret
 ***************************************************************************/

/**************************************************************************
 • function name
   closeTable

 • description
   close table and save table information to disk
 
 • parameters
    RM_TableData *rel
 
 • return value
    return ret
 ***************************************************************************/

/**************************************************************************
 • function name
   deleteTable

 • description
   delete the table and destroy pages files
 
 • parameters
    char *name
 
 • return value
    return ret
 ***************************************************************************/

/**************************************************************************
 • function name
   getNumTuples

 • description
   getNum total Tuples fromt he table header
 
 • parameters
    RM_TableData *rel
 
 • return value
    return totalslot
 ***************************************************************************/

/**************************************************************************
 • function name
   insertRecord

 • description
   insert Record at the end of the file 
 
 • parameters
    RM_TableData *rel, Record *record
 
 • return value
    return ret
 ***************************************************************************/

/**************************************************************************
 • function name
   deleteRecord

 • description
   delete record by set the record to ############
 
 • parameters
    RM_TableData *rel, RID id
 
 • return value
    return ret
 ***************************************************************************/

/**************************************************************************
• function name 
  updateRecord

• description
   update Record data for order slot
 
 • parameters
    RM_TableData *rel, Record *record
 
 • return value
    return ret
 ***************************************************************************/

/**************************************************************************
• function name 
  getRecord

• description
   get record by the order slot
 
 • parameters
    RM_TableData *rel, RID id, Record *record
 
 • return value
    return ret
 ***************************************************************************/

/**************************************************************************
• function name
  startScan

• description
   fullfill the scanhelper structure which used for scan table 
 
 • parameters
   RM_TableData *rel, RM_ScanHandle *scan, Expr *cond
 
 • return value
    return ret
 ***************************************************************************/

/**************************************************************************
 • function name
   next

 • description
   get the next record by the ordered slot id and page id 
 
 • parameters
   RM_ScanHandle *scan, Record *record
 
 • return value
    return ret
 ***************************************************************************/

Additional dberror
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

	/* error code for Record Managerment*/
	#define RC_REC_TABLE_CREATE_FAILED 206
	#define RC_REC_CREATE_SCHEMA_FAILED 207
	#define RC_REC_FILE_NOT_FOUND 208
	#define RC_REC_SCHEMA_NOT_FOUND 209
	#define RC_REC_PIN_PAGE_NON_EXIT 210
	#define RC_INVALID_SLOT_NUMBER 211
	#define RC_RECORD_REMOVED 212

Data structure
	typedef enum DataType {
  		DT_INT = 0,
  		DT_STRING = 1,
  		DT_FLOAT = 2,
  		DT_BOOL = 3
	} DataType;

	typedef struct Value {
  		DataType dt;
  		union v {
    		int intV;
    		char *stringV;
    		float floatV;
    		bool boolV;
  		} v;
	} Value;

	typedef struct RID {
  		int page;
  		int slot;
	} RID;

	typedef struct Record
	{
  		RID id;
  		char *data;
	} Record;

	//BM_BufferPool *BM;
	typedef struct RM_ScanHelper{
    		struct Expr *conditon;
    		RID rid;
    		int stopSign;
    		int current;
	}RM_ScanHelper;

// TableData: Management Structure for a Record Manager to handle one relation
	typedef struct RM_TableData
	{
 		char *name;
  		Schema *schema;
  		void *mgmtData;
  		void *TableHeader;
	} RM_TableData;

// Table Header: Maintain the table page, recorder information
	typedef struct RM_TableHeader
	{
    		char tableName[30];
    		int numRecorders;
    		int totalPages;
    		int totalslot;
    		int recordersPerPage;
    		int numSchemaAttr;
    		int key;
	} RM_TableHeader;


	enum type {
    		Block_EmpPage = 0,
    		Block_Full = 1,
    		Block_Cemetery = 2,
    		Block_Used = 3,
	}type;

	typedef struct RM_BlockHeader {
    		int blockID;
    		int freeSlot;
    		int type;
    		int RecordsCapacity;
    		int numRecords;
	}RM_BlockHeader;

// Bookkeeping for scans
	typedef struct RM_ScanHandle
	{
  		RM_TableData *rel;
  		void *mgmtData;
	} RM_ScanHandle;

Extra credit:
	TID and tombstones

Additional files:
    record_mgr.c
	buffer_mgr_sts.c
	buffer_mgr_sts.h
	test_assign3_1_modify.c

Test cases
	We add test case for tombstones

Note:
	We modified RM_TableData structure to implement scan function.
