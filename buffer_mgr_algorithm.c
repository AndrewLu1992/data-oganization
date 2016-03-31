#include <stdio.h>
#include <stdlib.h>
#include "storage_mgr.h"
#include "buffer_mgr_algorithm.h"

BM_PageFrame* findEmpPage(BM_PageFrame * FrameListHead) {
    struct BM_PageFrame *curFrame = FrameListHead;

    while(curFrame != NULL) {
        if(curFrame->flags == Frame_EmpPage){
            return curFrame;
        }
        else
            curFrame = curFrame->next;
    }

    // Do not find the empty page in the list
    return NULL;
}

BM_PageFrame* pinPosition(BM_PageFrame * FrameListHead) {
    struct BM_PageFrame *curFrame = FrameListHead;

    while(curFrame != NULL) {
        if(curFrame->fixCount == 0) {
            return curFrame;
        }
        else
            curFrame = curFrame->next;
    }
    
    // Do not find the empty page in the list
    return NULL;
}

RC InsertAfterFrame(BM_BufferPool *const bm, struct BM_PageFrame *insertPos, struct BM_PageFrame *selectedFrame) {
    int ret = 0;

    BM_PageFrame *Head = bm->mgmtData;
    BM_PageFrame *nextFrame;
    
    nextFrame = insertPos->next;
    
    selectedFrame->prev = insertPos;
    selectedFrame->next = nextFrame;
    
    if (nextFrame != NULL)
        nextFrame->prev = selectedFrame;
    else
        Head->prev = selectedFrame; 
    insertPos->next = selectedFrame;

    return ret;
}

RC maintainLFUFrameList(BM_BufferPool *const bm, struct BM_PageFrame *selectedFrame) {
    int ret;
    BM_PageFrame *insertPos,  *curFrame = selectedFrame->next, Tail;

    BM_PageFrame *Head = bm->mgmtData;

   
    // If the selectedFrame is in the tail of list, do not need to maintain
    if (selectedFrame == Head->prev){
            printf("%s, %d selected Frame is the tail do not need to maintain it\n", __func__,__LINE__);
            return ret;
    }
     
    // Do not need to maintain the list 
    if (selectedFrame->freq <= selectedFrame->next->freq){
        printf("%s, %d donot need maintain select frame%d freq %d is <= next frame%d freq %d\n", __func__, __LINE__, selectedFrame->PFN, selectedFrame->freq, selectedFrame->next->PFN, selectedFrame->next->freq);
        return ret;
    }

    // Remove SelectedFrame from the list
    if (selectedFrame == bm->mgmtData) {
        bm->mgmtData = selectedFrame->next;
        selectedFrame->next->prev = selectedFrame->prev;
        
    }
    else {
        selectedFrame->next->prev = selectedFrame->prev;
        selectedFrame->prev->next = selectedFrame->next;
    }
   
    // find the position that to insert the selected frame 
    while(1) {
        if (selectedFrame->freq > curFrame->freq) {
            if(curFrame->next != NULL)
                curFrame = curFrame->next;
            else { 
                insertPos = curFrame;
                break;
            }
        }
        else if (selectedFrame->freq <= curFrame->freq) {
                insertPos = curFrame->prev; // Insert the frame before curFrame
                break;
        }
    }
    ret = InsertAfterFrame(bm, insertPos, selectedFrame);

    curFrame = bm->mgmtData;
    return ret;
}

/*update frame list head, this could be used in FIFO, LRU*/
RC maintainSortedFrameList(BM_BufferPool *const bm, struct BM_PageFrame *selectedFrame) {
    int ret;
    BM_PageFrame *Head = bm->mgmtData;

    if(selectedFrame == Head->prev)
        return ret; 
    if(selectedFrame == Head) {
        bm->mgmtData = Head->next;
        Head->next = NULL;
        Head->prev->next = Head;
    }
    else {
        // Removed the frame from the list
        selectedFrame->prev->next = selectedFrame->next;
        selectedFrame->next->prev = selectedFrame->prev;

        Head->prev->next = selectedFrame;
        selectedFrame->prev = Head->prev;
        selectedFrame->next = NULL;
        Head->prev = selectedFrame;
    }
    return ret;
}


RC FIFO(BM_BufferPool *const bm, BM_PageHandle *const page, const PageNumber pageNum) {
    int ret, i;
    struct BM_PageFrame *frameListHead, *curFrame, *selectedFrame;
    SM_FileHandle fh;
    
    ret = openPageFile(bm->pageFile, &fh);
    if (RC_OK != ret){
        printf("%s, %d, Open Page File Fail\n", __func__,__LINE__);
        return RC_FILE_HANDLE_NOT_INIT;
    }
    frameListHead = (BM_PageFrame *)bm->mgmtData;
     
    selectedFrame = findEmpPage(frameListHead);
   
        // No Empty page, fetch suitable position to replace the page
    if (NULL == selectedFrame) { 
        selectedFrame = pinPosition(frameListHead);
    }

    // Do not have usable frame in memory 
    if (NULL == selectedFrame) { 
        printf("%s All frame cannot be used\n", __func__);
        return RC_BM_BP_NO_FRAME_TO_REP;
    }
  
    //Prepare to read the page from disk to memory
    if (fh.totalNumPages < pageNum +1) {
        ret = ensureCapacity(pageNum +1, &fh);
        NumWriteIO += pageNum +1 - fh.totalNumPages;
        if (RC_OK != ret) {
            closePageFile(&fh);
            printf("%s, %d ensurecapacity fail\n",__func__, __LINE__ );
            return RC_READ_NON_EXISTING_PAGE;
        }
    }

    //Write framepage to disk if page is dirty
    if(selectedFrame->flags & Frame_dirty) {
        writeBlock(selectedFrame->pageHandle.pageNum,  &fh, selectedFrame->pageHandle.data); 
        selectedFrame->flags &= ~Frame_dirty;
        NumWriteIO++;
    }

    ret  = readBlock(pageNum, &fh, selectedFrame->pageHandle.data);
    NumReadIO++;
     
    page->pageNum = pageNum;
    page->data = selectedFrame->pageHandle.data;
    
    selectedFrame->fixCount += 1; 
    selectedFrame->pageHandle.pageNum = pageNum;
    selectedFrame->flags |=Frame_cached;
    
    maintainSortedFrameList(bm, selectedFrame);

    closePageFile(&fh);

    return ret;    
}

RC CLOCK(BM_BufferPool *const bm, BM_PageHandle *const page, const PageNumber pageNum) {
    int ret;

    return ret;    
}

RC LFU(BM_BufferPool *const bm, BM_PageHandle *const page, const PageNumber pageNum) {
    int ret, i;
    struct BM_PageFrame *frameListHead, *curFrame, *selectedFrame;
    SM_FileHandle fh;

    ret = openPageFile(bm->pageFile, &fh);
    if (RC_OK != ret)
        return RC_FILE_HANDLE_NOT_INIT;

    frameListHead = (BM_PageFrame *)bm->mgmtData;
        
    selectedFrame = findEmpPage(frameListHead);
    // No Empty page, fetch suitable position to replace the page
    if (NULL == selectedFrame) 
        selectedFrame = pinPosition(frameListHead);
    printf("%s, %d selectedFrame is %d\n", __func__,__LINE__,selectedFrame->PFN); 

    // Do not have usable frame in memory 
    if (NULL == selectedFrame) {
        printf("%s All frame cannot be used\n", __func__);
        return RC_BM_BP_NO_FRAME_TO_REP;
    }
   
    //Prepare to read the page from disk to memory
    if (fh.totalNumPages < pageNum +1) {
        ret = ensureCapacity(pageNum+1, &fh);
        NumWriteIO += pageNum +1 - fh.totalNumPages; 
        if (RC_OK != ret) {
            closePageFile(&fh);
            return RC_READ_NON_EXISTING_PAGE;
        }
    }
    //Write framepage to disk if page is dirty
    if(selectedFrame->flags & Frame_dirty) {
       writeBlock(selectedFrame->pageHandle.pageNum,  &fh, selectedFrame->pageHandle.data); 
       selectedFrame->flags &= ~Frame_dirty;
       NumWriteIO++;
    }

    ret  = readBlock(pageNum, &fh, selectedFrame->pageHandle.data);
    NumReadIO++;

    page->pageNum = pageNum;
    page->data = selectedFrame->pageHandle.data;
    
    selectedFrame->fixCount += 1; 
    selectedFrame->freq =1;
    selectedFrame->pageHandle.pageNum = pageNum;
    selectedFrame->flags |=Frame_cached; 

    maintainLFUFrameList(bm, selectedFrame);
    
    closePageFile(&fh);
    
    return ret;
}

RC LRU_K(BM_BufferPool *const bm, BM_PageHandle *const page, const PageNumber pageNum) {
    int ret;

    return ret;    
}

