#include "record_mgr.h"
#include "buffer_mgr_stat.h"
#include "dberror.h"
#include "buffer_mgr.h"

#include <stdio.h>
#include <stdlib.h>

BM_BufferPool *bm;

// table and manager
RC initRecordManager (void *mgmtData){
	int ret = 0;

    bm = MAKE_POOL();
    
    ret = createPageFile("database.bin");
    if(ret != RC_OK) {
        printf("Create Page Fail\n");
        return ret;
    }
    
    ret = initBufferPool(bm, "database.bin", 5, RS_LFU, NULL);
    if (ret != RC_OK) {
        printf("Init Buffer POOL Fail\n");
        return RC_BM_BP_INIT_BUFFER_POOL_FAILED;
    }

	return ret;
}

RC shutdownRecordManager (){
	int ret = 0;

    ret = shutdownBufferPool(bm);
    if(ret != 0) {
        printf("shutdown buffer pool fail\n");
        return ret;
    }
        
    bm = NULL;

	return ret;
}

RC createTable (char *name, Schema *schema) {
	int ret = 0;

    printf("Enter %s\n", __func__);
    
    if(schema == NULL) {
        printf("schema is NULL\n");
        return RC_REC_TABLE_CREATE_FAILED;
    }
    
    ret = createPageFile(name);
    
    if (ret != RC_OK) {
        printf("Create Table Page File %s Fail\n", name);
        return RC_REC_TABLE_CREATE_FAILED;
    }

    

	return ret;
}

RC openTable (RM_TableData *rel, char *name) {
	int ret = 0;

	return ret;
}

RC closeTable (RM_TableData *rel){
	int ret = 0;

	return ret;
}

RC deleteTable (char *name){
	int ret = 0;

	return ret;
}

int getNumTuples (RM_TableData *rel) {
	int ret = 0;

	return ret;
}

// handling records in a table
RC insertRecord (RM_TableData *rel, Record *record) {
	int ret = 0;

	return ret;
}

RC deleteRecord (RM_TableData *rel, RID id) {
	int ret = 0;

	return ret;
}

RC updateRecord (RM_TableData *rel, Record *record) {
	int ret = 0;

	return ret;
}

RC getRecord (RM_TableData *rel, RID id, Record *record) {
	int ret = 0;

	return ret;
}

// scans
RC startScan (RM_TableData *rel, RM_ScanHandle *scan, Expr *cond) {
	int ret = 0;

	return ret;
}

RC next (RM_ScanHandle *scan, Record *record) {
	int ret = 0;

	return ret;
}

RC closeScan (RM_ScanHandle *scan) {
	int ret = 0;

	return ret;
}

// dealing with schemas
int getRecordSize (Schema *schema) {
	int ret = 0;

	return ret;
}

Schema *createSchema (int numAttr, char **attrNames, DataType *dataTypes, int *typeLength, int keySize, int *keys) {
	int ret = 0;
    Schema *schm;

    schm = malloc(sizeof(Schema));
    if (schm == NULL) {
        printf("malloc schema size fail\n");
        return schm;
    }
    
    schm->numAttr = numAttr; 
    schm->attrNames = attrNames;
    schm->dataTypes = dataTypes;
    schm->typeLength = typeLength;
    schm->keyAttrs = keys; 
    schm->keySize = keySize;

	return schm;
}

RC freeSchema (Schema *schema){
	int ret = 0;

	return ret;
}

// dealing with records and attribute values
RC createRecord (Record **record, Schema *schema) {
	int ret = 0;

	return ret;
}

RC freeRecord (Record *record) {
	int ret = 0;

	return ret;
}

RC getAttr (Record *record, Schema *schema, int attrNum, Value **value) {
	int ret = 0;

	return ret;
}

RC setAttr (Record *record, Schema *schema, int attrNum, Value *value) {
	int ret = 0;

	return ret;
}
