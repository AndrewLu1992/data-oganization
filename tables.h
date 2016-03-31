#ifndef TABLES_H
#define TABLES_H

#include "dt.h"


// Data Types, Records, and Schemas
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

// information of a table schema: its attributes, datatypes, 
typedef struct Schema
{
  int numAttr;
  char **attrNames;
  DataType *dataTypes;
  int *typeLength;
  int *keyAttrs;
  int keySize;
} Schema;

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

#define MAKE_STRING_VALUE(result, value)				\
  do {									\
    (result) = (Value *) malloc(sizeof(Value));				\
    (result)->dt = DT_STRING;						\
    (result)->v.stringV = (char *) malloc(strlen(value) + 1);		\
    strcpy((result)->v.stringV, value);					\
  } while(0)


#define MAKE_VALUE(result, datatype, value)				\
  do {									\
    (result) = (Value *) malloc(sizeof(Value));				\
    (result)->dt = datatype;						\
    switch(datatype)							\
      {									\
      case DT_INT:							\
	(result)->v.intV = value;					\
	break;								\
      case DT_FLOAT:							\
	(result)->v.floatV = value;					\
	break;								\
      case DT_BOOL:							\
	(result)->v.boolV = value;					\
	break;								\
      }									\
  } while(0)


// debug and read methods
extern Value *stringToValue (char *value);
extern char *serializeTableInfo(RM_TableData *rel);
extern char *serializeTableContent(RM_TableData *rel);
extern char *serializeSchema(Schema *schema);
extern char *serializeRecord(Record *record, Schema *schema);
extern char *serializeAttr(Record *record, Schema *schema, int attrNum);
extern char *serializeValue(Value *val);

#endif
