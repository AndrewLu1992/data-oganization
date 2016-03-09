#include "record_mgr.h"

// table and manager
RC initRecordManager (void *mgmtData){
	int ret;

	return ret;
}

RC shutdownRecordManager (){
	int ret;

	return ret;
}

RC createTable (char *name, Schema *schema) {
	int ret;

	return ret;
}

RC openTable (RM_TableData *rel, char *name) {
	int ret;

	return ret;
}

RC closeTable (RM_TableData *rel){
	int ret;

	return ret;
}

RC deleteTable (char *name){
	int ret;

	return ret;
}

int getNumTuples (RM_TableData *rel) {
	int ret;

	return ret;
}

// handling records in a table
RC insertRecord (RM_TableData *rel, Record *record) {
	int ret;

	return ret;
}

RC deleteRecord (RM_TableData *rel, RID id) {
	int ret;

	return ret;
}

RC updateRecord (RM_TableData *rel, Record *record) {
	int ret;

	return ret;
}

RC getRecord (RM_TableData *rel, RID id, Record *record) {
	int ret;

	return ret;
}

// scans
RC startScan (RM_TableData *rel, RM_ScanHandle *scan, Expr *cond) {
	int ret;

	return ret;
}

RC next (RM_ScanHandle *scan, Record *record) {
	int ret;

	return ret;
}

RC closeScan (RM_ScanHandle *scan) {
	int ret;

	return ret;
}

// dealing with schemas
int getRecordSize (Schema *schema) {
	int ret;

	return ret;
}

Schema *createSchema (int numAttr, char **attrNames, DataType *dataTypes, int *typeLength, int keySize, int *keys) {
	int ret;

	return ret;
}

RC freeSchema (Schema *schema){
	int ret;

	return ret;
}

// dealing with records and attribute values
RC createRecord (Record **record, Schema *schema) {
	int ret;

	return ret;
}

RC freeRecord (Record *record) {
	int ret;

	return ret;
}

RC getAttr (Record *record, Schema *schema, int attrNum, Value **value) {
	int ret;

	return ret;
}

RC setAttr (Record *record, Schema *schema, int attrNum, Value *value) {
	int ret;

	return ret;
}
