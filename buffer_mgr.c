#include <stdio.h>
#include <stdlib.h>
#include "dberror.h"
#include "storage_mgr.h"
#include "buffer_mgr.h"


// Init the PageFrameList that contines with page frame informaiton
struct BM_PageFrame * initPageFrameList(int numPages) {
    int i;
    struct BM_PageFrame *HEAD , *Tail;
    struct BM_PageFrame *NewPageFrame;

    for (i = 0; i < numPages; i++) {
        // Fullfil the datastructure of BM_PageFrame        
        NewPageFrame = (BM_PageFrame *) malloc(sizeof(BM_PageFrame));
        NewPageFrame->PFN = i;
        NewPageFrame->fixCount = 0;
        NewPageFrame->flags = Frame_active;
        NewPageFrame->pageHandle.data = (BM_FrameAddress) calloc(1, PAGE_SIZE); // alloca pagesize frame
        NewPageFrame->next = NULL;         
        if (0 == i)
            HEAD = Tail= NewPageFrame;
        else{
            Tail->next = NewPageFrame;
            Tail = Tail->next;    
        }
    }
    return HEAD;
}

//Create a buffer pool for an existing page file
RC initBufferPool(BM_BufferPool* const bm, const char* const pageFileName, const int numPages, ReplacementStrategy strategy, void *stratData){  
    struct BM_PageFrame *PageFrameList = NULL;

    printf("Enter %s\n", __func__);

    if (NULL == bm)
        return RC_BM_BP_NOT_FOUND;
    if (NULL == pageFileName)
        return RC_FILE_NOT_FOUND;
    if (numPages < 0)
        return RC_BM_BP_REQUEST_PAGE_NUMBER_ILLEGAL;

    PageFrameList = initPageFrameList(numPages);
    if (PageFrameList == NULL)
        return RC_BM_BP_PAGEFRAME_INIT_FAILED;
 
    bm->pageFile = (char*) pageFileName;
    bm->numPages = numPages;
    bm->mgmtData = PageFrameList;
    bm->strategy = strategy;

    printf("exit %s\n", __func__);
    return RC_OK;
}

//Destroy the Frame in the memory
RC DestroyPageFrameList(struct BM_PageFrame *FrameList, int numFrames) {
    int i, ret = 0;

    struct BM_PageHandle *pageHandler;
    struct BM_PageFrame *curFrame, *nextFrame = FrameList;

    printf("Enter %s\n", __func__);
    for (i=0; i < numFrames; i++) {
        free(curFrame->pageHandle.data);   //free page data in memory 
        nextFrame = curFrame->next;

        free(curFrame);                    //free PageFrame 
        curFrame = nextFrame;
    }
    
    printf("exit %s\n", __func__);
    return RC_OK;
}


// Rmove the pool
RC shutdownBufferPool(BM_BufferPool *const bm) {
    int ret = 0;
    printf("Enter %s\n", __func__);
 
    ret = DestroyPageFrameList(bm->mgmtData, bm->numPages);

    printf("exit %s\n", __func__);
    return ret;
}

RC forceFlushPool(BM_BufferPool *const bm) {
    printf("Enter %s\n", __func__);

    printf("exit %s\n", __func__);
    return RC_OK;
}

// Buffer Manager Interface Access Pages
RC markDirty (BM_BufferPool *const bm, BM_PageHandle *const page) {
    printf("Enter %s\n", __func__);

    printf("exit %s\n", __func__);
    return RC_OK;
}

RC unpinPage (BM_BufferPool *const bm, BM_PageHandle *const page) {

    printf("Enter %s\n", __func__);
    printf("exit %s\n", __func__);
    return RC_OK;
}

RC forcePage (BM_BufferPool *const bm, BM_PageHandle *const page) {

    printf("Enter %s\n", __func__);
    printf("exit %s\n", __func__);
    return RC_OK;
}

RC pinPage (BM_BufferPool *const bm, BM_PageHandle *const page, 
        const PageNumber pageNum) {
    printf("Enter %s\n", __func__);

    printf("exit %s\n", __func__);
    return RC_OK;
}

// Statistics Interface
PageNumber *getFrameContents (BM_BufferPool *const bm) {
    printf("Enter %s\n", __func__);

    printf("exit %s\n", __func__);
    return RC_OK;
}

bool *getDirtyFlags (BM_BufferPool *const bm) {
    printf("Enter %s\n", __func__);

    printf("exit %s\n", __func__);
    return RC_OK;
}

int *getFixCounts (BM_BufferPool *const bm) {

    printf("Enter %s\n", __func__);
    printf("exit %s\n", __func__);
    return RC_OK;
}

int getNumReadIO (BM_BufferPool *const bm) {

    printf("Enter %s\n", __func__);
    printf("exit %s\n", __func__);
    return RC_OK;
}

int getNumWriteIO (BM_BufferPool *const bm) {

    printf("Enter %s\n", __func__);
    printf("exit %s\n", __func__);
    return RC_OK;
}
