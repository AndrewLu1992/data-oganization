#include "record_mgr.h"
#include "buffer_mgr_stat.h"
#include "dberror.h"
#include "buffer_mgr.h"
#include "tables.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TABLE_HEADER_PAGE   0

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
    int ret, HeaderPageNum = 0, i, RecordSize;
    RM_TableHeader TableHeader;
    unsigned int offset=0 , size = 0;

    BM_PageHandle *page = MAKE_PAGE_HANDLE();

    ret = pinPage(BM, page, HeaderPageNum);
    if (ret != RC_OK){
        printf("%s pin page fail\n", __func__);
        return ret;
    }

    //fullfile the pageHeader structure
    strcpy(TableHeader.tableName, name);
    TableHeader.numRecorders = 0;
    TableHeader.totalPages = 1;
    TableHeader.totalRecorder = 0;
    TableHeader.recordersPerPage = 10;

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

    RecordSize = getRecordSize(schema);
    memcpy((char *)page->data + offset, schema->typeLength, size);
    
    //copy attr name to frame
    for (i = 0; i < schema->numAttr; i ++) {    
        offset += size; 
        size = sizeof(char);
        memcpy((char *)page->data + offset, *(schema->attrNames + i), size);
    }

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
    
struct RM_TableHeader * parsePageHeader(char * page, Schema *schema) {
    int ret = 0;
    int numAttr = 0;
    int *cpSizes, *cpKeys,* keyAttrs; 
    unsigned int offset;
    struct RM_TableHeader * TableHeader;
    DataType *cpDt;
    char * attrNames;
    
    TableHeader = (struct RM_TableHeader *)malloc(sizeof(RM_TableHeader));

    memcpy(TableHeader, page, sizeof(RM_TableHeader));

    cpDt = (DataType *) malloc(sizeof(DataType) * TableHeader->numSchemaAttr);
    cpSizes = (int *) malloc(sizeof(int) * TableHeader->numSchemaAttr);
    attrNames = (char *) malloc(sizeof(char) * TableHeader->numSchemaAttr);
    keyAttrs = (int *) malloc(sizeof(int));

    numAttr = TableHeader->numSchemaAttr;

    // parse datatype
    offset =  sizeof(RM_TableHeader);
    memcpy(cpDt, page + offset, numAttr * sizeof(int));

    memcpy(keyAttrs, &(TableHeader->key), sizeof(int));

    //parse data type size
    offset += numAttr * sizeof(int);
    memcpy(cpSizes, page + offset, numAttr * sizeof(int)); 

    offset += numAttr * sizeof(int);
    strncpy(attrNames, page + offset, numAttr * sizeof(char));
    
    //full fill schema structure    
    schema->dataTypes = cpDt;
    schema->attrNames = &attrNames;
    schema->typeLength = cpSizes;
    schema->numAttr = numAttr;
    schema->keyAttrs = keyAttrs;
 
    return TableHeader;
}

RC openTable (RM_TableData *rel, char *name) {
	int ret = 0;
    Schema *schema;
    struct RM_TableHeader *TableHeader;


    BM_PageHandle *page = MAKE_PAGE_HANDLE();

    schema = malloc(sizeof(Schema));
    if (schema == NULL) {
        printf("malloc schema size fail\n");
        return -1;
    }
    
    //Initialize Buffer Pool
    ret = initBufferPool(BM, name, 3, RS_FIFO, NULL);
    if (ret != RC_OK) {
        printf("Init Buffer POOL Fail\n");
        return RC_BM_BP_INIT_BUFFER_POOL_FAILED;
    }


    ret = pinPage(BM, page, TABLE_HEADER_PAGE);

    TableHeader = parsePageHeader(page->data, schema);

    BM->pageFile = name;

    rel->name = name;
    rel->schema = schema;
    rel->mgmtData = TableHeader;

	return ret;
}

RC closeTable (RM_TableData *rel){
	int ret = 0;

    

	return ret;
}

RC deleteTable (char *name){
	int ret = 0;

    //ret = DestroyPageFile(name);

    return ret;
}

int getNumTuples (RM_TableData *rel) {
	int ret = 0;
    BM_PageHandle *page = (BM_PageHandle *) malloc(sizeof(BM_PageHandle));

    ret = pinPage(BM, page, TABLE_HEADER_PAGE);
    if (ret != RC_OK) {
        printf("%s, %d, pin page 0 fail\n", __func__,__LINE__);
        return ret;
    }


    
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
    int recordSize = 0, i;
    
    for (i = 0; i < schema->numAttr; i++) {
        switch(schema->dataTypes[i]){
            case DT_INT:
                schema->typeLength[i] = sizeof(int);                
                break;
            case DT_FLOAT:
                schema->typeLength[i] = sizeof(float);
                break;
            case DT_BOOL:
                schema->typeLength[i] = sizeof(bool);
                break;
        }

        recordSize += schema->typeLength[i];
    }
   return recordSize;
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
    int RecordSize;

    *record = malloc(sizeof(Record));

    RecordSize = getRecordSize(schema);
   
    (*record)->data = (char *) calloc(RecordSize, sizeof(char)); 

    return ret;
}

RC freeRecord (Record *record) {
	int ret = 0;
    
    free(record->data);
    free(record);

	return ret;
}

RC getAttr (Record *record, Schema *schema, int attrNum, Value **value) {
	int ret = 0, offset = 0, i;
    Value *Val;

    Val = (Value *) malloc(sizeof(Value));
    
    Val->dt = schema->dataTypes[attrNum];
    
    for (i = 0; i < attrNum; i++) {
        offset += schema->typeLength[i];
    }

    switch(schema->dataTypes[attrNum]) {
        case DT_STRING:
            Val->v.stringV = (char *) malloc(schema->typeLength[attrNum] + 1);
            memcpy(Val->v.stringV, record->data + offset,schema->typeLength[attrNum]);
            Val->v.stringV[schema->typeLength[attrNum]] = '\0';
            break;
        case DT_INT:
            memcpy(&(Val->v.intV), record->data + offset,schema->typeLength[attrNum]);
            break;
        case DT_FLOAT:
            memcpy(&(Val->v.floatV), record->data + offset,schema->typeLength[attrNum]);
            break;
        case DT_BOOL:
            memcpy(&(Val->v.boolV), record->data + offset,schema->typeLength[attrNum]);
            break;
    }

    value = &Val;    

	return ret;
}

RC setAttr (Record *record, Schema *schema, int attrNum, Value *value) {
	int ret = 0,  offset = 0, i;

    for (i = 0; i < attrNum; i++) {
        offset += schema->typeLength[i];
    }

    switch(value->dt) {
        case DT_STRING:
            memcpy(record->data + offset, value->v.stringV,schema->typeLength[attrNum]);
            break;
        case DT_BOOL:
            memcpy(record->data + offset, &(value->v.boolV),schema->typeLength[attrNum]);
            break;
        case DT_INT:
            memcpy(record->data + offset, &(value->v.intV),schema->typeLength[attrNum]);
            break;
        case DT_FLOAT:
            memcpy(record->data + offset, &(value->v.floatV),schema->typeLength[attrNum]);
            break;
    }

	return ret;
}
