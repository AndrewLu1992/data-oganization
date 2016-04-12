#include "btree_mgr.h"

BM_BufferPool *BM = NULL;
struct BTreeHandle * BTreeHandler = NULL;

#define INDEX_HEADER_PAGE 0

RC initIndexManager (void *mgmtData){
    int ret = 0;
    BM = MAKE_POOL();
    
    ret = initBufferPool(BM, "assignment4", 3, RS_FIFO, NULL);
    if (ret != RC_OK) {
        printf("Init Buffer POOL Fail\n");
        return RC_BM_BP_INIT_BUFFER_POOL_FAILED;
    }

    BTreeHandler = (BTreeHandle *)malloc(sizeof(BTreeHandle));
    return ret;
}

RC shutdownIndexManager (){
    int ret = 0;

    ret = shutdownBufferPool(BM);
    if(ret != RC_OK) {
        printf("Shutdown Buffer Pool fail\n");
        return RC_BM_BP_SHUTDOWN_BUFFER_POOL_FAILED;
    }

    free(BM);
    free(BTreeHandler);    

    return ret;
}

// create, destroy, open, and close an btree index
RC createBtree (char *idxId, DataType keyType, int n){
    int ret = 0;
    BM_PageHandle *page = MAKE_PAGE_HANDLE();
    BT_Info btInfo;

    if(idxId == NULL) {
        printf("IDXID File name is NULL\n");
        return RC_REC_FILE_NOT_FOUND;
    }

    //Create Page File
    ret = createPageFile(idxId);
    
    if (ret != RC_OK) {
        printf("Create Table Page File %s Fail\n", idxId);
        return RC_REC_TABLE_CREATE_FAILED;
    }

    ret = pinPage(BM, page, INDEX_HEADER_PAGE);
    if (ret != RC_OK){
        printf("%s pin page fail\n", __func__);
        return ret;
    } 

    btInfo.rootPageNum = 1;
    btInfo.totalNodes = 0;
    btInfo.totalPages = 1;
    btInfo.keyType;
    btInfo.height;
    btInfo.numNodes=0;
    btInfo.numEntry=0;
    btInfo.N = n;// Num of search key in a node

    memcpy(page->data, &btInfo, sizeof(BT_Info));

    ret = markDirty(BM, page);
    if (ret != RC_OK){
        printf("%s Mark Dirty Page fail\n", __func__);
        return ret;
    }
    
    unpinPage(BM, page);
    free(page);
    
    return ret;
}

RC openBtree (BTreeHandle **tree, char *idxId) {
    int ret = 0;
    BM_PageHandle *page = MAKE_PAGE_HANDLE();
    BT_Info *btInfo;
    BTreeHandle *handle = NULL;

    handle = (BTreeHandle *)malloc(sizeof(BTreeHandle));
    handle->idxId = (char *)malloc(strlen(idxId)+1);
    btInfo = (BT_Info *) malloc(sizeof(BT_Info));    

    ret = pinPage(BM, page, INDEX_HEADER_PAGE);
    if (ret != RC_OK){
        printf("%s pin page fail\n", __func__);
        return ret;
    } 

    memcpy(btInfo, page->data, sizeof(BT_Info));

    //Fullfill the structure BTreeHandl
    strcpy(handle->idxId, idxId);
    handle->mgmtData = btInfo;
    handle->keyType = btInfo->keyType;

    *tree = handle;

    return ret;
}

RC closeBtree (BTreeHandle *tree) {
    int ret = 0;
    BM_PageHandle *page = MAKE_PAGE_HANDLE();
    BT_Info *btInfo = tree->mgmtData;
    
    ret = pinPage(BM, page, INDEX_HEADER_PAGE);
    if (ret != RC_OK){
        printf("%s pin page fail\n", __func__);
        return ret;
    } 

    memcpy(page->data, btInfo, sizeof(BT_Info));
    
    ret = markDirty(BM, page);
    if (ret != RC_OK){
        printf("%s Mark Dirty Page fail\n", __func__);
        return ret;
    }
    
    ret = forceFlushPool(BM);
    if (ret != RC_OK) {
        printf("%s Flush Page faiil\n", __func__);
        return RC_BM_BP_FLUSH_PAGE_FAILED;
    }    
    
    free(tree->mgmtData);
    unpinPage(BM, page);
    
    free(page);

    return ret;
}

RC deleteBtree (char *idxId){
    int ret = 0;

    ret = destroyPageFile(idxId);
    if(ret != RC_OK) {
        printf("Detele Table Fail\n");
        return ret;
    }

    return ret;
}

// access information about a b-tree
RC getNumNodes (BTreeHandle *tree, int *result) {
    int ret = 0;

    result = &((BT_Info *)tree->mgmtData)->numNodes;
    return ret;
}

RC getNumEntries (BTreeHandle *tree, int *result) {
    int ret = 0;

    result = &((BT_Info *)tree->mgmtData)->numEntry;
    return ret;
}

RC getKeyType (BTreeHandle *tree, DataType *result) {
    int ret = 0;

    result = &(tree->keyType);
    return ret;
}

// index access
RC findKey (BTreeHandle *tree, Value *key, RID *result) {
    int ret = 0;

    return ret;
}

struct Node * creatNode(BTreeHandle *tree, Value *key) {
    struct BT_Info *btreeInfo;
    struct Node *node;
    DataType keyType;
    int MaxNumKeys, *pointerArray;
    void *keyArray;

    btreeInfo = (struct BT_Info *)tree->mgmtData;
    keyType = tree->keyType;
    MaxNumKeys = btreeInfo->N;

    node = (struct Node *)malloc(sizeof(Node));

    switch (keyType) {
        case DT_INT:
            keyArray = (void *)malloc(sizeof(int)* MaxNumKeys);
            memcpy(keyArray, &(key->v.intV), sizeof(int));
            node->index.intV = (int *)keyArray;
            break;
        case DT_BOOL:
            keyArray = (void *)malloc(sizeof(bool)* MaxNumKeys);
            memcpy(keyArray, &(key->v.boolV), sizeof(bool));
            node->index.boolV = (bool *)keyArray;
            break;
        case DT_FLOAT:
            keyArray = (void *)malloc(sizeof(float)* MaxNumKeys);
            memcpy(keyArray, &(key->v.floatV), sizeof(float));
            node->index.floatV = (float *)keyArray;
            break;
        case DT_STRING:
            break;
    }
    pointerArray = (int *)malloc(sizeof(int) * (MaxNumKeys+1));

    node->NumOfKeys = 0;
    node->parent = 0;
    node->sibling= 0;
    node->pointerArray = pointerArray;

    return node;
}

RC insertKey (BTreeHandle *tree, Value *key, RID rid) {
    int ret = 0, availPage, offset = 0;
    struct BT_Info *btree_info;
    struct Node *root;
    BM_PageHandle *page = MAKE_PAGE_HANDLE();

    btree_info = (struct BT_Info *)tree->mgmtData;
    availPage = btree_info->totalPages;

    //Creat Root
    if (btree_info->totalNodes == 0){
        root = creatNode(tree, key);
        if (root == NULL) {
            printf("Create Node Fail\n");
            return RC_CREATE_NODE_FAILED;
        }

        root->NodeType = NT_ROOT;
        root->NumOfKeys = 1;

        //update Btree info
        btree_info->height =1;
        btree_info->totalPages = 1;
        btree_info->totalNodes = 1;
        btree_info->totalKeys = 1;
        btree_info->numNodes = 1;
        btree_info->numEntry = btree_info->N + 1;
            
        //Write node to Page
        ret = pinPage(BM, page, availPage);
        if(ret != RC_OK) {
            printf("%s pin page fail\n", __func__);
            return ret;
        }
        // copy root to page data space
        memcpy(page->data, root, sizeof(Node));

        // copy pointer to page data space
        offset = sizeof(Node);
        memcpy(page->data + offset, root->pointerArray, sizeof(int) * (btree_info->N));

        // copy key to page data space
        offset += sizeof(int) * (btree_info->N);
        switch (tree->keyType) {
            case DT_INT:
                memcpy(page->data + offset, root->index.intV, sizeof(int) * btree_info->N);
                break;
            case DT_BOOL:
                memcpy(page->data + offset, root->index.boolV, sizeof(bool) * btree_info->N);
                break;
            case DT_FLOAT:
                memcpy(page->data + offset, root->index.floatV, sizeof(float) * btree_info->N);
                break;
        } 
        free(tree->mgmtData);
        unpinPage(BM, page);
    }
/*
    // Get the node Position for the inserted Key
    findKeyNode() 
    
    
    
    switch(){
        case NS_SIMPLE_CASE:   //space available in leaf
        break;
        case NS_LEAF_OVERFLOW:
        break;
        case NS_NON_LEAF_OVERFLOW:
        break;
        case NS_ROOT_OVERFLOW:
        break;
    }
*/
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

