#include <stdio.h>
#include <stdlib.h>
#include "dberror.h"
#include "storage_mgr.h"
#include "buffer_mgr.h"
#include "buffer_mgr_algorithm.h"

int NumReadIO;
int NumWriteIO;

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
        NewPageFrame->freq = 0;
        NewPageFrame->flags = Frame_EmpPage;
        NewPageFrame->pageHandle.pageNum = i; // alloca pagesize frame
        NewPageFrame->pageHandle.data = (BM_FrameAddress) calloc(1, PAGE_SIZE); // alloca pagesize frame
        NewPageFrame->next = NULL;
        NewPageFrame->prev = NULL;
        
        if (0 == i)
            HEAD = Tail= NewPageFrame;
        else{
            HEAD->prev = NewPageFrame;
            Tail->next = NewPageFrame;
            Tail->next->prev = Tail;
            Tail->next->next = NULL;
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
    
    NumReadIO = 0;
    NumWriteIO = 0;
     
    bm->pageFile = (char*) pageFileName;
    bm->numPages = numPages;
    bm->mgmtData = PageFrameList;
    bm->strategy = strategy;

    return RC_OK;
}

// Rmove the pool
RC shutdownBufferPool(BM_BufferPool *const bm) {
    int ret = 0;
    int i;
    struct BM_PageFrame *curFrame, *nextFrame;

    curFrame= nextFrame = bm->mgmtData;

    while (curFrame != NULL) {
        if ((curFrame->fixCount == 0) && (curFrame->flags & Frame_dirty)) {
            ret = forcePage(bm, &(curFrame->pageHandle));
            if (ret != RC_OK) {
                printf("%s Flush Page %d faiil\n", __func__, curFrame->PFN);
                return RC_BM_BP_FLUSH_PAGE_FAILED;
            }
        }    
    
        free(curFrame->pageHandle.data);   //free page data in memory 
        nextFrame = curFrame->next;

        free(curFrame);                    //free PageFrame 
        curFrame = nextFrame;
    }

    return ret;
}

RC forceFlushPool(BM_BufferPool *const bm) {
    int i, ret = 0;
    unsigned int numFrames = bm->numPages;
    struct BM_PageFrame *curFrame = bm->mgmtData;
   
    while (curFrame != NULL) {
        if ((curFrame->fixCount == 0) && (curFrame->flags & Frame_dirty)) {   // page can not be flushed till no other thread used this frame and frame is dirty
            forcePage(bm, &(curFrame->pageHandle));
            curFrame->flags |= Frame_swapbacked;   // Frame is swapbacked
            curFrame->flags &= ~Frame_dirty;        // Fame is not dirty
        }
        curFrame = curFrame->next;
    }

    return RC_OK;
}

// Buffer Manager Interface Access Pages
RC markDirty (BM_BufferPool *const bm, BM_PageHandle *const page) {
    struct BM_PageFrame *curFrame = bm->mgmtData;
   
    while(curFrame != NULL) {
        if (curFrame->pageHandle.pageNum ==  page->pageNum) {
            curFrame->flags = curFrame->flags | Frame_dirty;
            break;
        }
        else 
            curFrame = curFrame->next;
    }

    return RC_OK;
}

RC unpinPage (BM_BufferPool *const bm, BM_PageHandle *const page) {
    struct BM_PageFrame *curFrame = bm->mgmtData;
 
    while(curFrame != NULL) {
        if (curFrame->pageHandle.pageNum ==  page->pageNum) {
            curFrame->fixCount -=1;
            return RC_OK; 
        }
        else 
            curFrame = curFrame->next;
    }

    return RC_OK;
}

RC forcePage (BM_BufferPool *const bm, BM_PageHandle *const page) {

    int i, ret;
    SM_FileHandle fh;
    
    ret = openPageFile(bm->pageFile, &fh);
    if (RC_OK != ret)
        return RC_FILE_HANDLE_NOT_INIT;

    ret = writeBlock(page->pageNum, &fh, page->data);
    if (RC_OK != ret) {
        closePageFile(&fh);
        printf("%s Write Page %d Fail\n", __func__, page->pageNum);
        return RC_WRITE_FAILED;
    }
    
    NumWriteIO++;
    closePageFile(&fh);

    return RC_OK;
}

RC checkCachedPage(BM_BufferPool *const bm, BM_PageHandle *const page, const PageNumber pageNum) {
    int ret;
    struct BM_PageFrame *curFrame =(BM_PageFrame *) bm->mgmtData;    

    //Fetch the frame with ordered pageNum 
    //ret = fetchFrame(FrameList, page, pageNum);

    while(curFrame != NULL) {
        if (curFrame->flags == Frame_EmpPage) {
            curFrame = curFrame->next;
            continue;
        }
        else if(pageNum == curFrame->pageHandle.pageNum) {  //Page matched
            page->pageNum = pageNum;
            curFrame->fixCount += 1; 
            page->data = curFrame->pageHandle.data;

            if(bm->strategy == RS_LRU)
                maintainSortedFrameList(bm, curFrame);
            if(bm->strategy == RS_LFU) {
                curFrame->freq +=1;
                maintainLFUFrameList(bm, curFrame);                
            }
            return RC_OK;
        }
        else
            curFrame = curFrame->next;
    }

    if ((NULL == curFrame) || (NULL == page)) {
        ret = RC_BM_BP_NOT_FOUND; 
    }
    
    return RC_BM_BP_EMP_POOL;
}

RC pinPage (BM_BufferPool *const bm, BM_PageHandle *const page, const PageNumber pageNum) {
    int ret;

    //check page is in the memory or not
    ret = checkCachedPage(bm, page, pageNum);
   
    if (RC_OK == ret)
        return RC_OK;
   
    // Load page from disk to memory with ordered strategy
    switch(bm->strategy) {
        case RS_LRU:
        case RS_FIFO:
            ret = FIFO(bm, page, pageNum);
            break;
        case RS_CLOCK:
            ret = CLOCK(bm, page, pageNum);
            break;
        case RS_LFU:
            ret = LFU(bm, page, pageNum);
            break;
        case RS_LRU_K:
            ret = LRU_K(bm, page, pageNum);
            break;
    }

    return RC_OK;
}

// Statistics Interface
PageNumber *getFrameContents (BM_BufferPool *const bm) {
    int i, seq;
    int *PageNumArray = NULL;
    BM_PageFrame *frame = bm->mgmtData;
    
    PageNumArray = (int *)malloc(bm->numPages * sizeof(int));

    while(frame!=NULL) {
        seq = frame->PFN;
        
        if(frame->flags == Frame_EmpPage){
            PageNumArray[seq] = NO_PAGE;
        }
        else{
            PageNumArray[seq] = frame->pageHandle.pageNum;
        }
        frame = frame->next;
    }

    return PageNumArray;
}

bool *getDirtyFlags (BM_BufferPool *const bm) {
    int i,seq;
    bool *DirtyFlagArray = NULL;
    BM_PageFrame *frame = bm->mgmtData;

    DirtyFlagArray = (bool *)malloc(bm->numPages * sizeof(bool));

    for (i = 0; i < bm->numPages; i++) {
        seq = frame->PFN;
        //if(frame->pageHandle.pageNum == -1)
        if(frame->flags == Frame_EmpPage)
            DirtyFlagArray[seq] = DIRTY_CLEAN;
        else
            DirtyFlagArray[seq] = frame->flags & Frame_dirty;
        frame = frame->next;
    }

    return DirtyFlagArray;
}

int *getFixCounts (BM_BufferPool *const bm) {
    int i,seq;
    int *FixCountsArray = NULL;
    BM_PageFrame *frame = bm->mgmtData;
    
    FixCountsArray= (int *)malloc(bm->numPages * sizeof(int));

    for (i = 0; i < bm->numPages; i++) {
        seq = frame->PFN;
        if(Frame_EmpPage == frame->flags)
            FixCountsArray[seq] = 0;
        else
            FixCountsArray[seq] = frame->fixCount;
        frame = frame->next;
    }

    return FixCountsArray;
}

int getNumReadIO (BM_BufferPool *const bm) {

    return NumReadIO;
}

int getNumWriteIO (BM_BufferPool *const bm) {

    return NumWriteIO;
}
