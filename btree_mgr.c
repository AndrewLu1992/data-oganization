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
    btInfo.totalNodes = 0;
    btInfo.totalPages = 1;
    btInfo.keyType = keyType
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
    int offset = 0, ret=0, *pArr, pArrSize=0, tmp,i;
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
    printf("%s, %d , PageNum is %d, NodeType is %d, NumOfKeys is %d, Parent is %d, sibling is %d\n", 
                __func__, __LINE__,
                node.PageNum,
                node.NodeType,
                node.NumOfKeys,
                node.parent,
                node.sibling);

    // Find the Leaf Node and return the node structure
    if(node.NodeType == NT_LEAF) {
        printf("Find the leaf\n");
        unpinPage(BM, page);
        return node;
    }

    //Malloc the array to store pointer array
    pArrSize = sizeof(int) * (node.NumOfKeys+1);
    node.pointers.pArr = (int *)malloc(pArrSize);

    //FullFill pointer Array
    offset = sizeof(struct Node);
    memcpy(node.pointers.pArr, page->data+offset, pArrSize);


    //Fetch key from memory
    offset+= sizeof(int) * (btree_info->N+1);

    switch (tree->keyType) {
        case DT_INT:
            node.key.intV = (void *)malloc(sizeof(int) * node.NumOfKeys);
            memcpy(node.key.intV, page->data+offset, sizeof(int) * node.NumOfKeys);

            //Find the Leaf node
            for(i=0;i<node.NumOfKeys;i++) {
                printf("keyArr i %d is %d, key is %d \n",i, node.key.intV[i], key->v.intV);
                if(key->v.intV <= node.key.intV[i]) {
                    tmp =  node.pointers.pArr[i];
                    free(node.key.intV);
                    free(node.pointers.pArr);
                    unpinPage(BM, page);
                    return findLeafNode(tree, node.pointers.pArr[i], key);
                }
            }
            if(key->v.intV > node.key.intV[i-1]){
                printf("keyArr i %d is %d  key is %d \n",i, node.key.intV[i], key->v.intV);
                tmp =  node.pointers.pArr[i];
                free(node.key.intV);
                free(node.pointers.pArr);
                unpinPage(BM, page);
                return findLeafNode(tree, node.pointers.pArr[i], key);
            }
 
            free(node.key.intV);
            
            break;
        case DT_BOOL:
            node.key.boolV = (bool *) malloc(sizeof(bool) * node.NumOfKeys);
            memcpy(node.key.boolV, page->data+offset, sizeof(bool) * node.NumOfKeys);
            
            //Find the Leaf node
            for(i=0;i<node.NumOfKeys;i++) {
                printf("keyArr i %d is %d, key is %d \n",i, node.key.boolV[i], key->v.boolV);
                if(key->v.boolV <= node.key.boolV[i]) {
                    tmp =  node.pointers.pArr[i];
                    free(node.key.boolV);
                    free(node.pointers.pArr);
                    unpinPage(BM, page);
                    return findLeafNode(tree, node.pointers.pArr[i], key);
                }
            }
            if(key->v.boolV > node.key.boolV[i-1]){
                printf("keyArr i %d is %d  key is %d \n",i, node.key.boolV[i], key->v.boolV);
                tmp =  node.pointers.pArr[i];
                free(node.key.boolV);
                free(node.pointers.pArr);
                unpinPage(BM, page);
                return findLeafNode(tree, node.pointers.pArr[i], key);
            }
            
            break;
        case DT_FLOAT:
            node.key.floatV = (float *) malloc(sizeof(float) * node.NumOfKeys);
            memcpy(node.key.floatV, page->data+offset, sizeof(float) * node.NumOfKeys);
            
            //Find the Leaf node
            for(i=0;i<node.NumOfKeys;i++) {
                printf("keyArr i %d is %f greater than  key %f \n",i, node.key.floatV[i], key->v.floatV);
                if(key->v.floatV <= node.key.floatV[i]) {
                    tmp =  node.pointers.pArr[i];
                    free(node.key.floatV);
                    free(node.pointers.pArr);
                    unpinPage(BM, page);
                    return findLeafNode(tree, tmp, key);
                }
            }
            if(key->v.floatV > node.key.floatV[i-1]){
                printf("keyArr i %d is %f is less than  key %f \n",i, node.key.floatV[i], key->v.floatV);
                
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
        
    ret = pinPage(BM, page, node.PageNum);
    if(ret != RC_OK) {
        printf("%s pin page fail\n", __func__);
        return ret;
    }
    //Malloc the array to store pointer array
    RIDArrSize = sizeof(struct RID) * node.NumOfKeys;
    node.pointers.RIDArr = (struct RID *)malloc(RIDArrSize);

    offset = sizeof(struct Node);
    memcpy(node.pointers.RIDArr, page->data+offset, RIDArrSize);

    offset += sizeof(struct RID) * btreeInfo->N;

    switch (tree->keyType) {
        case DT_INT:
            node.key.intV = (void *)malloc(sizeof(int) * node.NumOfKeys);
            memcpy(node.key.intV, page->data+offset, sizeof(int) * node.NumOfKeys);

            //Find the Leaf node
            for(i=0;i<node.NumOfKeys;i++) {
                printf("%s, %d keyArr i %d is %d, key is %d \n",__func__,__LINE__,i, node.key.intV[i], key->v.intV);
                if(key->v.intV == node.key.intV[i]) {
                    memcpy(result, &(node.pointers.RIDArr[i]), sizeof(struct RID));
                    free(node.key.intV);
                    free(node.pointers.RIDArr);
                    unpinPage(BM, page);
                    return RC_OK;
                }
            }
 
            break;
        case DT_BOOL:
            node.key.boolV = (bool *) malloc(sizeof(bool) * node.NumOfKeys);
            memcpy(node.key.boolV, page->data+offset, sizeof(bool) * node.NumOfKeys);
            
            //Find the Leaf node
            for(i=0;i<node.NumOfKeys;i++) {
                printf("keyArr i %d is %d, key is %d \n",i, node.key.boolV[i], key->v.boolV);
                if(key->v.boolV == node.key.boolV[i]) {
                    memcpy(result, &(node.pointers.RIDArr[i]), sizeof(struct RID));
                    free(node.key.boolV);
                    free(node.pointers.RIDArr);
                    unpinPage(BM, page);
                    return RC_OK;
                }
            }
            
            break;
        case DT_FLOAT:
            node.key.floatV = (float *) malloc(sizeof(float) * node.NumOfKeys);
            memcpy(node.key.floatV, page->data+offset, sizeof(float) * node.NumOfKeys);
            printf("keyArr i %d is %d, key is %f \n",i, node.key.boolV[i], key->v.floatV);
            
            //Find the Leaf node
            for(i=0;i<node.NumOfKeys;i++) {
                if(key->v.floatV == node.key.boolV[i]) {
                    memcpy(result, &(node.pointers.RIDArr[i]), sizeof(struct RID));
                    free(node.key.floatV);
                    free(node.pointers.RIDArr);
                    unpinPage(BM, page);
                    return RC_OK;
                }
            }

            break;

        case DT_STRING:
            break;
    }
    
    printf("Cannot Find key %d in the tree, return -1\n", key->v.intV);

    unpinPage(BM, page);
    return ret;
}

struct Node * creatNode(BTreeHandle *tree, Value *key, NodeType nodeType) {
    struct BT_Info *btreeInfo;
    struct Node *node;
    DataType keyType;
    int MaxNumKeys;

    btreeInfo = (struct BT_Info *)tree->mgmtData;
    keyType = tree->keyType;
    MaxNumKeys = btreeInfo->N;

    node = (struct Node *)malloc(sizeof(Node));

    switch (keyType) {
        case DT_INT:
            node->key.intV = (int *)malloc(sizeof(int)* MaxNumKeys);
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

    switch (nodeType) {
        case NT_ROOT:
//            node->pointers.RIDArr = 
            break;
        case NT_LEAF:
            node->pointers.RIDArr =  (struct RID *)malloc(sizeof(struct RID) * MaxNumKeys);
            break; 
        case NT_NON_LEAF:
            node->pointers.pArr = (int *)malloc(sizeof(int) * (MaxNumKeys+1));
            break;
    }

    node->NumOfKeys = 1;
    node->parent = 0;
    node->sibling= 0;
    node->PageNum = tree->totalPages;

    return node;
}

RC saveNode(struct Node * node, BTreeHandle *tree) {
        int offset = 0, availPage, ret=0;
        struct BT_Info *btree_info;
        BM_PageHandle *page = MAKE_PAGE_HANDLE();
    
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

        //  pointer to page data space
        offset = sizeof(Node);
    
        switch (node->NodeType) {
            case NT_ROOT:
//            node->pointers.RIDArr = 
                break;
            case NT_LEAF:
                memcpy(page->data + offset, node->pointers.RIDArr, sizeof(struct RID) * (node->NumOfKeys));
                free(node->pointers.RIDArr);
                break; 
            case NT_NON_LEAF:
                memcpy(page->data + offset, node->pointers.pArr, sizeof(int) * (node->NumOfKeys));
                free(node->pointers.pArr);
                break;
        }

        // copy key to page data space
        offset += sizeof(int) * (btree_info->N);
        switch (tree->keyType) {
            case DT_INT:
                memcpy(page->data + offset, node->key.intV, sizeof(int) * node->NumOfKeys);
                free(node->key.intV);
                break;
            case DT_BOOL:
                memcpy(page->data + offset, node->key.boolV, sizeof(bool) * node->NumOfKeys);
                free(node->key.boolV);
                break;
            case DT_FLOAT:
                memcpy(page->data + offset, node->key.floatV, sizeof(float) * node->NumOfKeys);
                free(node->key.floatV);
                break;
        } 
        unpinPage(BM, page);
    
        free(node);

        return ret;
}

RC insertKeyIntoLeaf(BTreeHandle *tree, struct Node *node, Value *key) {
    int ret = 0;

    switch(tree->keyType) {
        case DT_INT:
            printf("%s, %d, key is %d\n", __func__,__LINE__, key->v.intV);
            node->key.intV[node.NumOfKeys] = key->v.intV;
            break;
        case DT_BOOL:
            node->key.boolV[node.NumOfKeys] = key->v.boolV;
            break;
        case DT_FLOAT:
            printf("%s, %d, key is %f\n", __func__,__LINE__, key->v.floatV);
            node->key.floatV[node.NumOfKeys] = key->v.floatV;
            break;
    }

    return ret;
}

RC insertKey (BTreeHandle *tree, Value *key, RID rid) {
    int ret = 0, availPage, rootPage;
    struct BT_Info *btree_Info;
    struct Node *newNode, node;
    struct RID result;

    btree_Info = (struct BT_Info *)tree->mgmtData;
/*    
    //Creat Root
    if (btree_Info->totalNodes == 0){
        root = creatNode(tree, key, NT_ROOT);
        if (root == NULL) {
            printf("Create Node Fail\n");
            return RC_CREATE_NODE_FAILED;
        }

        root->NodeType = NT_ROOT;
        root->NumOfKeys = 1;

        //update Btree info
        btree_Info->height =1;
        btree_Info->totalPages = 1;
        btree_Info->totalNodes = 1;
        btree_Info->totalKeys = 1;
        btree_Info->numNodes = 1;
        btree_Info->numEntry = btree_Info->N + 1;
            
        ret = saveNode(root, tree);
        if (ret != RC_OK) {
            printf("%s save Node fail\n", __func__);
            return -1;
        }
    }
*/
    // Get the node Position for the inserted Key
    ret = findKey (tree, key, &result);
    if (ret == 0 and result == rid) {
        printf("Key already exist");
        return RC_OK;
    }
    
    printf("Key do not exist in the tree\n");

    //Key is not exist in the tree
    rootPage = btreeInfo->rootPageNum;

    //Empty Tree
    if(rootPage = 0) {
        creatNode(tree, key, NT_LEAF);  //creat a leaf node
    }

    node = findLeafNode(tree, rootPage, key);
   
    if(Node.NumOfKeys < btree_Info.N)
        //simple case
        insertKeyIntoLeaf(tree, &node, key);
    else {
        //(b) leaf overflow
(b) leaf overflow
(c) non-leaf overflow
(d) new root

        newNode = creatNode(tree, key, NT_NON_LEAF);  // create a new node as node is full 
    }

    ret  = updateTree();
    
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

