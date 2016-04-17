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

    btInfo.rootPageNum = 0;
    btInfo.totalPages = 1;
    btInfo.keyType = keyType;
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

struct Node findLeafNode(BTreeHandle *tree, int PageNum, Value * key ) {
    int offset = 0, ret=0, *pArr, pArrSize=0, tmp,i, RIDArrSize = 0, KeyArrSize=0;
    struct BT_Info *btree_info;
    struct Node node;
    BM_PageHandle *page = MAKE_PAGE_HANDLE();
    
    btree_info = (struct BT_Info *)tree->mgmtData;

    //Fetch Node information
    ret = pinPage(BM, page, PageNum);
    if(ret != RC_OK) {
        printf("%s pin page fail\n", __func__);
        return node;
    }
    
    memcpy(&node, page->data, sizeof(struct Node));

    // Debug Node from memory
    printf("%s, %d , PageNum is %d, NodeType is %d, NumEntry is %d, Parent is %d, child is %d\n", 
                __func__, __LINE__,
                node.PageNum,
                node.NodeType,
                node.NumEntry,
                node.parent,
                node.next);
    //Caculate Key Array Size
    KeyArrSize = sizeof(struct Value) * (btree_info->N);
    node.KeyArr = (struct Value *) malloc(KeyArrSize);
    
    //Copy data from DISK
    offset = sizeof(struct Node);
    memcpy(node.KeyArr, page->data+offset, KeyArrSize);

    offset += KeyArrSize; 
    // Find the Leaf Node and return the node structure
    if(node.NodeType == NT_LEAF) {
        printf("Find the leaf\n");

        //Malloc the array to store pointer array and Entry Array
        RIDArrSize = sizeof(struct RID) * (btree_info->N);

        node.pointers.RIDArr = (struct RID *)malloc(RIDArrSize);

        memcpy(node.pointers.RIDArr, page->data+offset, RIDArrSize);

        unpinPage(BM, page);
        return node;
    }

    //Malloc the array to store pointer array
    pArrSize = sizeof(int) * (btree_info->N+1);
    node.pointers.pArr = (int *)malloc(pArrSize);

    //FullFill pointer Array
    memcpy(node.pointers.pArr, page->data+offset, pArrSize);

    // Find the next node to access
    for (i =0;i<node.NumEntry;i++) {
        ret = memcmp(key, &(node.KeyArr[i]), sizeof(struct Value));
        // Key <= KeyArr[i]
        if (ret <=0) {
                    tmp =  node.pointers.pArr[i];
                    free(node.KeyArr);
                    free(node.pointers.pArr);
                    unpinPage(BM, page);
                    return findLeafNode(tree, node.pointers.pArr[i], key);
        }
    }

    printf("Key is greater than the last value in Key Arr");
    tmp =  node.pointers.pArr[i];
    free(node.KeyArr);
    free(node.pointers.pArr);
    unpinPage(BM, page);
    return findLeafNode(tree, node.pointers.pArr[i], key);
    
    /*
        switch (tree->keyType) {
        case DT_INT:
                if(key->v.intV <= node.KeyArr[i].v.intV) {
                }
            break;
        case DT_BOOL:
                if(key->v.boolV <= node.KeyArr[i].boolV[i]) {
                    tmp =  node.pointers.pArr[i];
                    free(node.key.boolV);
                    free(node.pointers.pArr);
                    unpinPage(BM, page);
                    return findLeafNode(tree, node.pointers.pArr[i], key);
                }
            }
            if(key->v.boolV > node.key.boolV[i-1]){
                printf("KeyArr i %d is %d  key is %d \n",i, node.key.boolV[i], key->v.boolV);
                tmp =  node.pointers.pArr[i];
                free(node.key.boolV);
                free(node.pointers.pArr);
                unpinPage(BM, page);
                return findLeafNode(tree, node.pointers.pArr[i], key);
            }
            
            break;
        case DT_FLOAT:
            node.key.floatV = (float *) malloc(sizeof(float) * node.NumEntry);
            memcpy(node.key.floatV, page->data+offset, sizeof(float) * node.NumEntry);
            
            //Find the Leaf node
            for(i=0;i<node.NumEntry;i++) {
                printf("KeyArr i %d is %f greater than  key %f \n",i, node.key.floatV[i], key->v.floatV);
                if(key->v.floatV <= node.key.floatV[i]) {
                    tmp =  node.pointers.pArr[i];
                    free(node.key.floatV);
                    free(node.pointers.pArr);
                    unpinPage(BM, page);
                    return findLeafNode(tree, tmp, key);
                }
            }
            if(key->v.floatV > node.key.floatV[i-1]){
                printf("KeyArr i %d is %f is less than  key %f \n",i, node.key.floatV[i], key->v.floatV);
                
                tmp =  node.pointers.pArr[i];
                free(node.key.floatV);
                free(node.pointers.pArr);
                unpinPage(BM, page);
                return findLeafNode(tree, tmp, key);
            }

            break;

        case DT_STRING:
            break;
    }
        */
    return node;
}

// index access
RC findKey (BTreeHandle *tree, Value *key, RID *result) {
    int ret = -1, rootPage=0, offset,i, RIDArrSize;
    struct Node node;
    struct BT_Info *btreeInfo;
    BM_PageHandle *page = MAKE_PAGE_HANDLE();

    btreeInfo = (struct BT_Info *)tree->mgmtData;
     
    rootPage = btreeInfo->rootPageNum;

    node = findLeafNode(tree, rootPage, key);

    for(i=0; i < node.NumEntry;i++) {
        ret = memcmp(&(node.KeyArr[i]), key, sizeof(Value));
        if(ret == 0) {
            printf("Key had find out\n");
            memcpy(result, &(node.pointers.RIDArr[i]), sizeof(struct RID));

            free(node.KeyArr);
            free(node.pointers.RIDArr);
            unpinPage(BM, page);
            return RC_OK;
        }
    }

    printf("Cannot Find key %d in the tree, return -1\n", key->v.intV);

    unpinPage(BM, page);
    return ret;
}

struct Node * creatNode(BTreeHandle *tree, Value *key, NodeType nodeType) {
    struct BT_Info *btreeInfo;
    struct Node *node;
    int MaxNumKeys;

    btreeInfo = (struct BT_Info *)tree->mgmtData;
    MaxNumKeys = btreeInfo->N;

    node = (struct Node *)malloc(sizeof(Node));

    node->KeyArr = (struct Value *)malloc(sizeof(struct Value)* MaxNumKeys);

    memcpy(&(node->KeyArr[0]), key, sizeof(struct Value));
    /* 
    switch (tree->keyType) {
        case DT_INT:
            node->key.intV[0] = key->v.intV;
            //memcpy(node->key.intV, &(key->v.intV), sizeof(int));
            break;
        case DT_BOOL:
            node->key.boolV = (bool *) malloc(sizeof(bool)* MaxNumKeys);
            node->key.boolV[0] = key->v.boolV;
            //memcpy(node->key.boolV, &(key->v.boolV), sizeof(bool));
            break;
        case DT_FLOAT:
            node->key.floatV = (float *)malloc(sizeof(float)* MaxNumKeys);
            node->key.floatV[0] = key->v.floatV;
            //memcpy(node->key.floatV, &(key->v.floatV), sizeof(float));
            break;
        case DT_STRING:
            break;
    }
    */
    switch (nodeType) {
        case NT_ROOT:
            node->pointers.pArr = (int *)malloc(sizeof(int) * (MaxNumKeys+1));
            node->NodeType = NT_ROOT;
            break;
        case NT_LEAF:
            node->pointers.RIDArr =  (struct RID *)malloc(sizeof(struct RID) * MaxNumKeys);
            node->NodeType = NT_LEAF;
            break; 
        case NT_NON_LEAF:
            node->pointers.pArr = (int *)malloc(sizeof(int) * (MaxNumKeys+1));
            node->NodeType = NT_NON_LEAF;
            break;
    }

    node->NumEntry = 1;
    node->PageNum = btreeInfo->totalPages;
    node->NodeID = btreeInfo->numNodes;

    btreeInfo->totalPages++;
    btreeInfo->numNodes++;
    btreeInfo->numEntry++; 
    return node;
}

RC saveNode(struct Node *node, BTreeHandle *tree) {
        int offset = 0, availPage, ret=0, lenArr;
        struct BT_Info *btree_info;
        BM_PageHandle *page = MAKE_PAGE_HANDLE();
    
        lenArr = btree_info->N;
        btree_info = (struct BT_Info *)tree->mgmtData;
        //availPage = btree_info->totalPages; not true

        //Write node to Page
        ret = pinPage(BM, page, availPage);
        if(ret != RC_OK) {
            printf("%s pin page fail\n", __func__);
            return ret;
        }
        // copy root to page data space
        memcpy(page->data, node, sizeof(Node));

        //save  key array to page
        offset = sizeof(Node);
        memcpy(page->data + offset, node->KeyArr, sizeof(struct Value) * lenArr);
        
        // save Pointer array to page 
        offset += sizeof(struct Value) * lenArr;
        switch (node->NodeType) {
            case NT_ROOT:
                memcpy(page->data + offset, node->pointers.pArr, sizeof(int) * (lenArr + 1));
                free(node->pointers.pArr);
                break;
            case NT_LEAF:
                memcpy(page->data + offset, node->pointers.RIDArr, sizeof(struct RID) * lenArr);
                free(node->pointers.RIDArr);
                break; 
            case NT_NON_LEAF:
                memcpy(page->data + offset, node->pointers.pArr, sizeof(int) * (lenArr + 1));
                free(node->pointers.pArr);
                break;
        }
/*
        offset += sizeof(int) * (btree_info->N);
        switch (tree->keyType) {
            case DT_INT:
                memcpy(page->data + offset, node->key.intV, sizeof(int) * node->NumEntry);
                free(node->key.intV);
                break;
            case DT_BOOL:
                memcpy(page->data + offset, node->key.boolV, sizeof(bool) * node->NumEntry);
                free(node->key.boolV);
                break;
            case DT_FLOAT:
                memcpy(page->data + offset, node->key.floatV, sizeof(float) * node->NumEntry);
                free(node->key.floatV);
                break;
        } 
*/    
        ret = markDirty(BM, page);

        if (ret != RC_OK){
            printf("%s Mark Dirty Page fail\n", __func__);
            return ret;
        }

        free(node->KeyArr);        
        free(node);
        unpinPage(BM, page);

        return ret;
}

RC insertKeyIntoLeaf(BTreeHandle *tree, struct Node *node, Value *key) {
    int ret = 0;
    
    memcpy(&(node->KeyArr[node->NumEntry]), key, sizeof(struct Value));
/*
    switch(tree->keyType) {
        case DT_INT:
            printf("%s, %d, key is %d\n", __func__,__LINE__, key->v.intV);
            node->key.intV[node->NumEntry] = key->v.intV;
            break;
        case DT_BOOL:
            node->key.boolV[node->NumEntry] = key->v.boolV;
            break;
        case DT_FLOAT:
            printf("%s, %d, key is %f\n", __func__,__LINE__, key->v.floatV);
            node->key.floatV[node->NumEntry] = key->v.floatV;
            break;
    }
*/
    return ret;
}

RC splitLeaf(BTreeHandle *tree, struct Node *curLeaf, Value *key, RID rid) {
    struct Node *newLeafNode;
    struct BT_Info *btreeInfo;
    int i, j, InsertPos=0, ret, N, keyOffset, RIDOffset;
    struct  Value * tmpKeyArr;
    struct RID * tmpRIDArr;
    
    N = btreeInfo->N;

    btreeInfo = (struct BT_Info *)tree->mgmtData;
    tmpKeyArr = (struct Value *) malloc(sizeof(struct Value) * (N + 1));
    tmpRIDArr = (struct RID *) malloc(sizeof(struct RID) * (N + 1));
   
    //maintain the link 
    newLeafNode = creatNode(tree, key, NT_LEAF);  //creat a leaf node
    newLeafNode->next = curLeaf->PageNum; 

    // index of original key array
    j=0;

    // Sort the key and save in temp array
    for(i=0;i<= N; i++){
        ret = memcmp(key, &(curLeaf->KeyArr[j]), sizeof(struct Value));
        if(ret <0 || j == N){
            memcpy(&tmpKeyArr[i], key, sizeof(Value));
            memcpy(&tmpRIDArr[i], &rid, sizeof(RID));

            //copy leaf key and RID to tmp array
            memcpy(&tmpKeyArr[i+1], &curLeaf->KeyArr[j], sizeof(Value) * (N-j));
            memcpy(&tmpRIDArr[i+1], &curLeaf->pointers.RIDArr[j], sizeof(RID) * (N-j));
            break;
        }
        else {
            memcpy(&tmpKeyArr[i], &curLeaf->KeyArr[j], sizeof(Value));
            memcpy(&tmpRIDArr[i], &curLeaf->pointers.RIDArr[j], sizeof(RID));
            j++;
        }
    }
    
    //caculate new num entry for both node
    if((btreeInfo->N) % 2 ==0) {
        newLeafNode->NumEntry = btreeInfo->N /2 + 1;
        curLeaf->NumEntry = (btreeInfo->N)/2;
    }
    else{
        newLeafNode->NumEntry = (btreeInfo->N) /2;
        curLeaf->NumEntry = (btreeInfo->N)/2;
    }
    
    //clean the array of key and rid for both leaf node
    memset(curLeaf->KeyArr, 0, sizeof(struct Value) * N);
    memset(curLeaf->pointers.RIDArr, 0, sizeof(struct RID) * N);

    //fullfill the key array for each node

    memcpy(newLeafNode->KeyArr, tmpKeyArr, newLeafNode->NumEntry * sizeof(struct Value));
    memcpy(newLeafNode->pointers.RIDArr, tmpRIDArr, newLeafNode->NumEntry * sizeof(struct RID));
        
    //fullfill the RID array for each node
    keyOffset = newLeafNode->NumEntry * sizeof(struct Value);
    RIDOffset = newLeafNode->NumEntry * sizeof(struct RID);

    memcpy(curLeaf->KeyArr, tmpKeyArr + keyOffset , curLeaf->NumEntry * sizeof(struct Value));
    memcpy(curLeaf->pointers.RIDArr, tmpRIDArr + RIDOffset, curLeaf->NumEntry * sizeof(struct RID));
    
}

RC insertKey (BTreeHandle *tree, Value *key, RID rid) {
    int ret = 0, availPage, rootPage;
    struct BT_Info *btreeInfo;
    struct Node leaf;
    struct Node *rootNode, *leafNode;
    struct RID result;

    btreeInfo = (struct BT_Info *)tree->mgmtData;

    // Get the node Position for the inserted Key
    ret = findKey(tree, key, &result);
    if (ret == 0) {
        printf("Key already exist");
        return RC_OK;
    }
    
    printf("Key do not exist in the tree\n");

    //Key is not exist in the tree
    rootPage = btreeInfo->rootPageNum;

    //Empty Tree
    if(rootPage = 0) {
        leafNode = creatNode(tree, key, NT_LEAF);  //creat a leaf node
        rootNode = creatNode(tree, key, NT_ROOT);  //creat a leaf node

        rootNode->pointers.pArr[0]= leafNode->PageNum;
       
        leafNode->parent = rootNode->PageNum; 
        leafNode->pointers.RIDArr[0] = rid;
        
        //Save Root Node        
        ret = saveNode(rootNode, tree);
        if (ret != RC_OK) {
            printf("%s save Node fail\n", __func__);
            return -1;
        }
        
        //Save Leaf node
        ret = saveNode(leafNode, tree);
        if (ret != RC_OK) {
            printf("%s save Node fail\n", __func__);
            return -1;
        }
    }

    leaf = findLeafNode(tree, rootPage, key);
  
    // Leaf is not full 
    if(leaf.NumEntry < btreeInfo->N)
        insertKeyIntoLeaf(tree, &leaf, key); //simple case
    else {
        // Leaf is Full
        ret = splitLeaf(tree, &leaf, key, rid);
    }
/* 
        NonLeafFull
        RootFull


        SplitLeaf()
        InsertNewLeaf`sSmallest key into parent
        // leafOverFlow        

    ret  = updateTree();
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

