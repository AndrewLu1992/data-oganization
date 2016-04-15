#ifndef BTREE_MGR_H
#define BTREE_MGR_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "dberror.h"
#include "tables.h"
#include "btree_mgr.h"
#include "buffer_mgr.h"
#include "storage_mgr.h"
#include "record_mgr.h"

// structure for accessing btrees
typedef struct BTreeHandle {
  DataType keyType;
  char *idxId;
  void *mgmtData;
} BTreeHandle;

typedef struct BT_ScanHandle {
  BTreeHandle *tree;
  void *mgmtData;
} BT_ScanHandle;

typedef struct BT_Info {
    int rootPageNum;
    int totalNodes;
    int totalPages;
    int totalKeys;
    DataType keyType;
    int height;
    int numNodes;
    int numEntry;
    int N;  //Number of search key in a node
} BT_Info;

typedef enum NodeType {
    NT_ROOT=0,
    NT_LEAF,
    NT_NON_LEAF,
} NodeType;

typedef enum NodeOperation {
    NS_SIMPLE_CASE=0,
    NS_LEAF_OVERFLOW,
    NS_NON_LEAF_OVERFLOW,
    NS_ROOT_OVERFLOW,
} NodeStatus;

typedef struct Node {
    int PageID;
    int NodeID;
    int NodeType;
    int NumOfKeys;
    int parent;
    int sibling;
    union key {
        int* intV;
        //char *stringV;
        float* floatV;
        bool* boolV;
    } key;
    union pointers {
        int * pArr;
        struct RID * RIDArr;
    } pointers;
} Node;

// init and shutdown index manager
extern RC initIndexManager (void *mgmtData);
extern RC shutdownIndexManager ();

// create, destroy, open, and close an btree index
extern RC createBtree (char *idxId, DataType keyType, int n);
extern RC openBtree (BTreeHandle **tree, char *idxId);
extern RC closeBtree (BTreeHandle *tree);
extern RC deleteBtree (char *idxId);

// access information about a b-tree
extern RC getNumNodes (BTreeHandle *tree, int *result);
extern RC getNumEntries (BTreeHandle *tree, int *result);
extern RC getKeyType (BTreeHandle *tree, DataType *result);

// index access
extern RC findKey (BTreeHandle *tree, Value *key, RID *result);
extern RC insertKey (BTreeHandle *tree, Value *key, RID rid);
extern RC deleteKey (BTreeHandle *tree, Value *key);
extern RC openTreeScan (BTreeHandle *tree, BT_ScanHandle **handle);
extern RC nextEntry (BT_ScanHandle *handle, RID *result);
extern RC closeTreeScan (BT_ScanHandle *handle);

// debug and test functions
extern char *printTree (BTreeHandle *tree);

#endif // BTREE_MGR_H
