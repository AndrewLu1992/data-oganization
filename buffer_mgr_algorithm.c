#include <stdio.h>
#include <stdlib.h>
#include "storage_mgr.h"
#include "buffer_mgr_algorithm.h"

BM_PageFrame* findEmpPage(BM_PageFrame * FrameListHead) {
    struct BM_PageFrame *curFrame = FrameListHead;

    while(curFrame != NULL) {
        if(curFrame->flags == Frame_EmpPage)
            return curFrame;
        else
            curFrame = curFrame->next;
    }

    // Do not find the empty page in the list
    return NULL;
}

/*update frame list head, this could be used in FIFO, LRU*/
RC maintainSortedFrameList(BM_BufferPool *const bm, struct BM_PageFrame *selectedFrame) {
    int ret;
    BM_PageFrame * Head = bm->mgmtData;
    
    if(selectedFrame == Head->prev)
        return ret; 
    if(selectedFrame == Head) {
        bm->mgmtData = Head->next;
        Head->next = NULL;
        Head->prev->next = Head;
    }
    else {
        selectedFrame->prev->prev = selectedFrame;
        selectedFrame->prev->next = selectedFrame->next;
        selectedFrame->next->prev = selectedFrame->prev;
        Head->prev->next = selectedFrame;        
        selectedFrame->prev = Head->prev;
        selectedFrame->next = NULL;
    }
    return ret;
}


RC FIFO(BM_BufferPool *const bm, BM_PageHandle *const page, const PageNumber pageNum) {
    int ret, i;
    struct BM_PageFrame *frameListHead, *curFrame, *selectedFrame;
    SM_FileHandle fh;

    ret = openPageFile(bm->pageFile, &fh);
    if (RC_OK != ret)
        return RC_FILE_HANDLE_NOT_INIT;

    frameListHead = (BM_PageFrame *)bm->mgmtData;
        
    selectedFrame = findEmpPage(frameListHead);
    
    // Implement FIFO algorithem to find the oldest frame
    if (NULL == selectedFrame) {
        curFrame = frameListHead; // check from the tail of list
        for(i = 0; i < bm->numPages; i++) {
            if(curFrame->fixCount == 0)
                selectedFrame = curFrame;
            else
                curFrame = curFrame->next;
        }
    }

    // Do not have usable frame in memory 
    if (NULL == selectedFrame) {
        printf("%s All frame cannot be used\n", __func__);
        return RC_BM_BP_NO_FRAME_TO_REP;
    }
   
    //Prepare to read the page from disk to memory
    ret = ensureCapacity(pageNum + 1, &fh);
    if (RC_OK != ret) {
        closePageFile(&fh);
        return RC_READ_NON_EXISTING_PAGE;
    }

    //Write framepage to disk if page is dirty
    if(selectedFrame->flags & Frame_dirty)
       writeBlock (selectedFrame->pageHandle.pageNum,  &fh, selectedFrame->pageHandle.data); 

    ret  = readBlock(pageNum, &fh, selectedFrame->pageHandle.data);
   
    selectedFrame->fixCount += 1; 
    maintainSortedFrameList(bm, selectedFrame);

    return ret;    
}

RC CLOCK(BM_BufferPool *const bm, BM_PageHandle *const page, const PageNumber pageNum) {
    int ret;

    return ret;    
}

RC LFU(BM_BufferPool *const bm, BM_PageHandle *const page, const PageNumber pageNum) {
    int ret;

    return ret;    
}

RC LRU_K(BM_BufferPool *const bm, BM_PageHandle *const page, const PageNumber pageNum) {
    int ret;

    return ret;    
}

