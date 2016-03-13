#include "record_mgr.h"
#include "buffer_mgr_stat.h"
#include "dberror.h"
#include "buffer_mgr.h"
#include "tables.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

BM_BufferPool *BM;

// table and manager
RC initRecordManager (void *mgmtData){
	int ret = 0;

    BM = MAKE_POOL();
    
/*    
    ret = createPageFile("database.bin");
    if(ret != RC_OK) {
        printf("Create Page Fail\n");
        return ret;
    }
    ret = initBufferPool(BM, "database.bin", 5, RS_LFU, NULL);
    if (ret != RC_OK) {
        printf("Init Buffer POOL Fail\n");
        return RC_BM_BP_INIT_BUFFER_POOL_FAILED;
    }
*/
	return ret;
}

RC shutdownRecordManager (){
	int ret = 0;

    ret = shutdownBufferPool(BM);
    if(ret != 0) {
        printf("shutdown buffer pool fail\n");
        return ret;
    }
        
    BM = NULL;

	return ret;
}

RC initTableHeader(char *name, Schema *schema) {
    int ret, HeaderPageNum = 0;
    RM_TableHeader TableHeader;
    unsigned int offset=0 , size = 0;

    BM_PageHandle *page = MAKE_PAGE_HANDLE();

    ret = pinPage(BM, page, HeaderPageNum);
    if (ret != RC_OK){
        printf("%s pin page fail\n", __func__);
        return ret;
    }

    //fullfile the pageHeader structure
    memcpy(TableHeader.tableName, name, sizeof(*name));
    TableHeader.numRecorders = 0;
    TableHeader.totalPages = 1;
    TableHeader.totalRecorder = 0;

    //pageHeader.recordersPerPage = (PAGE_SIZE - sizeof(recorderHeader))/sizeof(schema->);
    TableHeader.numSchemaAttr = schema->numAttr;
    TableHeader.key = *(schema->keyAttrs);

    // 1st phase copy the content of Tableheader to frame
    memcpy(page->data, &TableHeader, sizeof(RM_TableHeader));
   
    // copy attr datatype to frame
    offset = (unsigned int)sizeof(RM_TableHeader); 
    size = schema->numAttr * sizeof(int);

    memcpy((char *)page->data + offset, schema->dataTypes, size);
    
    // copy attr size to frame
    offset += size; 
    size = schema->numAttr * sizeof(int);

    memcpy((char *)page->data + offset, schema->typeLength, size);

    //copy attr name to frame
    offset += size; 
    size = schema->numAttr * sizeof(char);
    
    memcpy((char *)page->data + offset, *(schema->attrNames), size);

    ret = markDirty(BM, page);
    if (ret != RC_OK){
        printf("%s Mark Dirty Page fail\n", __func__);
        return ret;
    }

    unpinPage(BM, page);

    return ret;
}

RC createTable (char *name, Schema *schema) {
	int ret = 0;

    printf("Enter %s\n", __func__);
    
    if(schema == NULL) {
        printf("schema is NULL\n");
        return RC_REC_SCHEMA_NOT_FOUND;
    }
    
    if(name == NULL) {
        printf("File name is NULL\n");
        return RC_REC_FILE_NOT_FOUND;
    }
   
    //Create Page File 
    ret = createPageFile(name);
    
    if (ret != RC_OK) {
        printf("Create Table Page File %s Fail\n", name);
        return RC_REC_TABLE_CREATE_FAILED;
    }
    
    //Initialize Buffer Pool
    ret = initBufferPool(BM, name, 3, RS_FIFO, NULL);
    if (ret != RC_OK) {
        printf("Init Buffer POOL Fail\n");
        return RC_BM_BP_INIT_BUFFER_POOL_FAILED;
    }
    
    //Initialize the Block Header
    ret = initTableHeader(name, schema);
    if (ret != RC_OK) {
        printf("Init Page Header Fail\n");
        return RC_BM_BP_INIT_BUFFER_POOL_FAILED;
    }

    ret = shutdownBufferPool(BM);
    if (ret != RC_OK) {
        printf("Shutdown Buffer Pool Fail\n");
        return RC_BM_BP_SHUTDOWN_BUFFER_POOL_FAILED;
    }

	return ret;
}

RC openTable (RM_TableData *rel, char *name) {
	int ret = 0;
    
    //Initialize Buffer Pool
    ret = initBufferPool(BM, name, 3, RS_FIFO, NULL);
    if (ret != RC_OK) {
        printf("Init Buffer POOL Fail\n");
        return RC_BM_BP_INIT_BUFFER_POOL_FAILED;
    }

    BM->pageFile = name;
    rel->name = name;
    //rel->schema;
    //rel->mgmtData;

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
    Schema *schema;

    schema = malloc(sizeof(Schema));
    if (schema == NULL) {
        printf("malloc schema size fail\n");
        return NULL;
    }
    
    schema->numAttr = numAttr; 
    schema->attrNames = attrNames;
    schema->dataTypes = dataTypes;
    schema->typeLength = typeLength;
    schema->keyAttrs = keys; 
    schema->keySize = keySize;

	return schema;
}

RC freeSchema (Schema *schema){
	int ret = 0, i;

    for (i=0; i<schema->numAttr; i++)
        free(schema->attrNames[i]);
    
    free(schema);

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
