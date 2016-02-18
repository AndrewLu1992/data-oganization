#include <stdio.h>
#include "dberror.h"
#include "storage_mgr.h"
#include "buffer_mgr.h"


// Init the PageFrameList that contines with page frame informaiton
struct PageFrame * initPageFrameList(int numPages) {
    int i;
    struct BM_PageFrame *HEAD = NULL;
    struct BM_PageFrame *Tail = NULL;
    struct BM_PageFrame *PageFrame;

    for (i = 0; i < numPages, i++) {
        // Fullfil the datastructure of BM_PageFrame        
        PageFrame = (BM_PageFrame *)malloc (sizeof(BM_PageFrame));
        PageFrame->PFN = i;
        PageFrame->flages = Frame_active;
        PageFrame->vm_start = calloc(1, PAGE_SIZE); // alloca pagesize frame
        PageFrame->next = NULL;        

        if (0 == i)
            HEAD = Tail= PageFrame;
        else
            Tail->next = PageFrame;
            Tail = Tail->next;    
    }

    return HEAD;
}

//Create a buffer pool for an existing page file
RC initBufferPool(BM_BufferPool *const bm, const char *const pageFileName, 
          const int numPages, ReplacementStrategy strategy, 
          void *stratData) {

    struct BM_PageFrame *PageFrameList = NULL;

    if (NULL == bm)
        return RC_BM_BP_NOT_FOUND;
    if (NULL == pageFileName)
        return RC_FILE_NOT_FOUND;
    if (numberPages < 0)
        return RC_BM_BP_REQUEST_PAGE_NUMBER_ILLEGAL;

    PageFrameList = initPageFrameList(numPages);
    if (PageFrameList == NULL)
        return RC_BM_BP_PAGEFRAME_INIT_FAILED;
 
    bm->pageFile = pageFileName;
    bm->numberPages = numPages;
    bm->mgmtData = PageFrameList;
    bm->strategy = strategy;

    return RC_OK;
}

RC shutdownBufferPool(BM_BufferPool *const bm) {
    int ret = 0;
 
    ret = DestroyPageFrameList(bm->mgmtData);
        

    return RC_OK;
}

RC forceFlushPool(BM_BufferPool *const bm) {

    return RC_OK;
}

// Buffer Manager Interface Access Pages
RC markDirty (BM_BufferPool *const bm, BM_PageHandle *const page) {

    return RC_OK;
}

RC unpinPage (BM_BufferPool *const bm, BM_PageHandle *const page) {

    return RC_OK;
}

RC forcePage (BM_BufferPool *const bm, BM_PageHandle *const page) {

    return RC_OK;
}

RC pinPage (BM_BufferPool *const bm, BM_PageHandle *const page, 
        const PageNumber pageNum) {

    return RC_OK;
}

// Statistics Interface
PageNumber *getFrameContents (BM_BufferPool *const bm) {

    return RC_OK;
}

bool *getDirtyFlags (BM_BufferPool *const bm) {

    return RC_OK;
}

int *getFixCounts (BM_BufferPool *const bm) {

    return RC_OK;
}

int getNumReadIO (BM_BufferPool *const bm) {

    return RC_OK;
}

int getNumWriteIO (BM_BufferPool *const bm) {

    return RC_OK;
}
