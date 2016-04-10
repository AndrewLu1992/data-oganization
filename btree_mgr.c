#include <stdio.h>
#include "btree_mgr.h"

RC initIndexManager (void *mgmtData){
    int ret = 0;

    return ret;
}

RC shutdownIndexManager (){
    int ret = 0;

    return ret;
}

// create, destroy, open, and close an btree index
RC createBtree (char *idxId, DataType keyType, int n){
    int ret = 0;

    return ret;
}

RC openBtree (BTreeHandle **tree, char *idxId) {
    int ret = 0;

    return ret;
}

RC closeBtree (BTreeHandle *tree) {
    int ret = 0;

    return ret;
}

RC deleteBtree (char *idxId){
    int ret = 0;

    return ret;
}

// access information about a b-tree
RC getNumNodes (BTreeHandle *tree, int *result) {
    int ret = 0;

    return ret;
}

RC getNumEntries (BTreeHandle *tree, int *result) {
    int ret = 0;

    return ret;
}

RC getKeyType (BTreeHandle *tree, DataType *result) {
    int ret = 0;

    return ret;
}

// index access
RC findKey (BTreeHandle *tree, Value *key, RID *result) {
    int ret = 0;

    return ret;
}

RC insertKey (BTreeHandle *tree, Value *key, RID rid) {
    int ret = 0;

    return ret;
}

RC deleteKey (BTreeHandle *tree, Value *key) {
    int ret = 0;

    return ret;
}

RC openTreeScan (BTreeHandle *tree, BT_ScanHandle **handle) {
    int ret = 0;

    return ret;
}

RC nextEntry (BT_ScanHandle *handle, RID *result) {
    int ret = 0;

    return ret;
}

RC closeTreeScan (BT_ScanHandle *handle) {
    int ret = 0;

    return ret;
}

// debug and test functions
char *printTree (BTreeHandle *tree) {

}

