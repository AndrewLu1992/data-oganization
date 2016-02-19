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
        NewPageFrame->flags = Frame_EmpPage;
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

    return RC_OK;
}

//Destroy the Frame in the memory
RC DestroyPageFrameList(struct BM_PageFrame *FrameList, int numFrames) {
    int i, ret = 0;

    struct BM_PageHandle *pageHandler;
    struct BM_PageFrame *nextFrame, *curFrame = FrameList;

    for (i=0; i < numFrames; i++) {
        free(curFrame->pageHandle.data);   //free page data in memory 
        nextFrame = curFrame->next;

        free(curFrame);                    //free PageFrame 
        curFrame = nextFrame;
    }
    
    return RC_OK;
}

// Rmove the pool
RC shutdownBufferPool(BM_BufferPool *const bm) {
    int ret = 0;
 
    ret = DestroyPageFrameList(bm->mgmtData, bm->numPages);

    return ret;
}

RC forceFlushpages(BM_BufferPool *bm, struct BM_PageFrame *Frame) {
    int i, ret;
    SM_FileHandle fh;
    BM_PageHandle page = Frame->pageHandle;
    
    printf("Frame PageFrame No.:%d, flags %d, fixCount %d , pageNum %d\n", Frame->PFN, Frame->flags, Frame->fixCount, Frame->pageHandle.pageNum);
    ret = openPageFile(bm->pageFile, &fh);
    if (RC_OK != ret)
        return RC_FILE_HANDLE_NOT_INIT;

    ret = writeBlock(page.pageNum, &fh, page.data);
    if (RC_OK != ret) {
        closePageFile(&fh);
        printf("%s Write Block Fail\n", __func__);
        return RC_WRITE_FAILED;
    }

    closePageFile(&fh);

   //  forceFlushpages(bm, curFrame);            

    return ret;
}

RC forceFlushPool(BM_BufferPool *const bm) {
    int i, ret = 0;
    unsigned int numFrames = bm->numPages;
    struct BM_PageFrame *curFrame = bm->mgmtData;
    
    for (i = 0; i < numFrames; i++) {
        if ((curFrame->fixCount == 0) && (curFrame->flags & Frame_dirty))
            forceFlushpages(bm, curFrame);            
    }

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
    int i;
    int *PageNumArray = NULL;
    BM_PageFrame *frame = bm->mgmtData;

    PageNumArray = (int *)malloc(bm->numPages * sizeof(int));

    for (i = 0; i < bm->numPages; i++) {
        if(Frame_EmpPage == frame->flags)
            PageNumArray[i] = NO_PAGE;
        else
            PageNumArray[i] = frame->pageHandle.pageNum;
        frame = frame->next;
    }

    return PageNumArray;
}

bool *getDirtyFlags (BM_BufferPool *const bm) {
    int i;
    bool *DirtyFlagArray = NULL;
    BM_PageFrame *frame = bm->mgmtData;

    DirtyFlagArray = (bool *)malloc(bm->numPages * sizeof(bool));

    for (i = 0; i < bm->numPages; i++) {
        if(Frame_EmpPage == frame->flags)
            DirtyFlagArray[i] = DIRTY_CLEAN;
        else
            DirtyFlagArray[i] = frame->flags & Frame_dirty;
        frame = frame->next;
    }

    return DirtyFlagArray;
}

int *getFixCounts (BM_BufferPool *const bm) {
    int i;
    int *FixCountsArray = NULL;
    BM_PageFrame *frame = bm->mgmtData;

    FixCountsArray= (int *)malloc(bm->numPages * sizeof(int));

    for (i = 0; i < bm->numPages; i++) {
        if(Frame_EmpPage == frame->flags)
            FixCountsArray[i] = 0;
        else
            FixCountsArray[i] = frame->fixCount;
        frame = frame->next;
    }

    return FixCountsArray;
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
