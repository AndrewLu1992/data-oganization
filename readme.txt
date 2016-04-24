Assignment 4
       In this assignment we are implementing a B+-tree index. Write functions that  are used to create or delete a b-tree index and functions that are used to find, insert, and delete keys in/from a given B+-tree.

Personnel information
	Ting Ma  <tma11@hawk.iit.edu>
	Shuo Yan<syan9@hawk.iit.edu >
	Congwei She<cshe@hawk.iit.edu >
	Chen Wang<Cwang90@hawk.iit.edu >

File list
	btree_mgr.c
	btree_mgr.h
	buffer_mgr_algorithm.c
	buffer_mgr_algorithm.h
	buffer_mgr_stat.c
	buffer_mgr_stat.h
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
	storage_mgr.c
	storage_mgr.h
	tables.h
	test_assign4_1.c
	test_helper.h
    

Milestone
	4/1 meeting and talk about how to fulfill it
	4/2 beginning coding
	4/16 test and debug
	4/24 make readme.txt and add header

Installation instruction
    1. make test_assign4_1
    2. make test_expr
    3. ./test_assign4_1
    4. ./test_expr
    5. make clean

Data structure
	typedef struct bTree{    		struct Value value;    		struct RID rid;	}bTree;

Function description

/**************************************************************************
 • function name
   initIndexManager
 
 • description
   initial index manger
 
 • parameters
   void *mgmtData
 
 • return value
   return RC_OK
 ***************************************************************************/

/**************************************************************************
 • function name
   shutdownIndexManager
 
 • description
   shut down index manger
 
 • parameters
   void
 
 • return value
   return RC_OK
 ***************************************************************************/

/**************************************************************************
 • function name
   createBtree
 
 • description
   create a B-Tree 
 
 • parameters
   char *idxId, DataType keyType, int n
 
 • return value
   return ret
 ***************************************************************************/

/**************************************************************************
 • function name
   openBtree
 
 • description
   open a B-Tree
 
 • parameters
   BTreeHandle **tree, char *idxId
 
 • return value
   return Ret
 ***************************************************************************/

/**************************************************************************
 • function name
   closeBtree
 
 • description
   close a B-Tree
 
 • parameters
   BTreeHandle *tree
 
 • return value
   return RC_OK
 ***************************************************************************/

/**************************************************************************
 • function name
   deleteBtree
 
 • description
   delete a B-Tree
 
 • parameters
   char *idxId
 
 • return value
   return ret
 ***************************************************************************/

/**************************************************************************
 • function name
   getNumNodes
 
 • description
   get the number of nodes of a B-Tree
 
 • parameters
   BTreeHandle *tree, int *result
 
 • return value
   return RC_OK
 ***************************************************************************/

/**************************************************************************
 • function name
   getNumEntries
 
 • description
   get the number of entries of a B-Tree
 
 • parameters
   BTreeHandle *tree, int *result
 
 • return value
   return ret
 ***************************************************************************/

/**************************************************************************
 • function name
   getKeyType
 
 • description
   get the key type of a B-Tree
 
 • parameters
   BTreeHandle *tree, DataType *result
 
 • return value
   return ret
 ***************************************************************************/

/**************************************************************************
 • function name
   findKey
 
 • description
   find RID of a key
 
 • parameters
   BTreeHandle *tree, Value *key, RID *result
 
 • return value
   return RC
 ***************************************************************************/

/**************************************************************************
 • function name
   insertKey
 
 • description
   insert a key to the B-Tree
 
 • parameters
   BTreeHandle *tree, Value *key, RID rid
 
 • return value
   return RC
 ***************************************************************************/

/**************************************************************************
 • function name
   deleteKey
 
 • description
   delete a key from a B-Tree
 
 • parameters
   BTreeHandle *tree, Value *key
 
 • return value
   return RC
 ***************************************************************************/

/**************************************************************************
 • function name
   openBtreeScan
 
 • description
   open a tree scan
 
 • parameters
   BTreeHandle *tree, BT_ScanHandle **handle
 
 • return value
 return RC_OK
 ***************************************************************************/

/**************************************************************************
 • function name
   nextEntry
 
 • description
   get the next record by the ordered slot id and page id
 
 • parameters
   BT_ScanHandle *handle, RID *result
 
 • return value
   return RC
 ***************************************************************************/

/**************************************************************************
 • function name
   closeTreeScan
 
 • description
   close a tree scan
 
 • parameters
   BT_ScanHandle *handle
 
 • return value
   return RC_OK
 ***************************************************************************/

Additional dberror:

+/* error code for btree management*/
+#define RC_CREATE_NODE_FAIL 400
+#define RC_CANNOT_MALLOC_STRING_TYPE 401;

Additional files:
btree_mgr.c
btree_mgr.h
test_assign4_1.c

