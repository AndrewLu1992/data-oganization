#include "storage_mgr.h"
#include "dberror.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

void initStorageManager (void){

}


RC createPageFile (char *fileName){
    FILE *fp = fopen(fileName, "a+");
    int NUMBER_PAGES = 1;
    int ret;

    if(NULL == fp)
        return RC_FILE_NOT_FOUND;
    
    /*Allock PAGE_SIZE contiguous memory and initializes it to zero*/
    SM_PageHandle *ptr = (SM_PageHandle *)calloc (NUMBER_PAGES, PAGE_SIZE);

    if(NULL == ptr)
        return RC_FILE_HANDLE_NOT_INIT;

    /*Write Number_pages`s page_size byte to file*/
    ret = fwrite(ptr, PAGE_SIZE, NUMBER_PAGES, fp);
    if(ret != NUMBER_PAGES)
        return RC_WRITE_FAILED;
     
    free(ptr);
    fclose(fp);

    return RC_OK;    
}

RC openPageFile (char *fileName, SM_FileHandle *fHandle){
    FILE *fp;
    struct stat status;

    fp = fopen(fileName, "r+");
    if(NULL == fp)
        return RC_FILE_NOT_FOUND;

    /*Fetch the status of the file*/
    stat(fileName, &status); 

    /*Init the file handler*/ 
    fHandle->fileName = fileName;
    fHandle->totalNumPages = status.st_size / PAGE_SIZE;
    fHandle->curPagePos = 0;
    fHandle->mgmtInfo = fp; //used by close function

    return RC_OK;    
}

RC closePageFile (SM_FileHandle *fHandle){
    int ret;
    FILE *fp;
    
    /*Check file handler status*/
    if(NULL == fHandle)
        return RC_FILE_HANDLE_NOT_INIT;

    fp = fHandle->mgmtInfo;

    ret = fclose(fp);
    if(0 != ret)
        return RC_FILE_CLOSE_FAILED;
    else
        return RC_OK;
}

RC destroyPageFile (char *fileName){
    int ret;

    if(NULL == fileName)
        return RC_FILE_NOT_FOUND;

    ret = remove(fileName);
    if(0 == ret)
        return RC_OK;
    else
        return RC_FILE_DELETE_FAILED;
}

/* reading blocks from disc */
RC readBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage){
    FILE *fp;
    long offset;
    int size;

    fp = fHandle->mgmtInfo;
    offset = pageNum * PAGE_SIZE;

    if (fHandle == NULL)
        return RC_FILE_HANDLE_NOT_INIT;

    if(pageNum < 0)
        return INV_PARAMETER;

    if(pageNum > fHandle->totalNumPages)
        return RC_READ_NON_EXISTING_PAGE;
    
    fseek(fp, offset, SEEK_SET);
    size = fread(memPage, 1, PAGE_SIZE, fp);
    fHandle->curPagePos = pageNum;

    return RC_OK;
}

int getBlockPos (SM_FileHandle *fHandle){
    if (fHandle == NULL)
        return RC_FILE_HANDLE_NOT_INIT;
    
    return fHandle->curPagePos;
}

RC readFirstBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
    int ret;

    if(NULL == fHandle)
        return RC_FILE_HANDLE_NOT_INIT;

    ret = readBlock(0, fHandle, memPage);
    
    return ret;
}

RC readPreviousBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
    int ret;

    if(NULL == fHandle)
        return RC_FILE_HANDLE_NOT_INIT;

    /*current page position is the first one, next page do not previous block*/
    if(0 == fHandle->curPagePos)
        return RC_READ_NON_EXISTING_PAGE;

    ret = readBlock(fHandle->curPagePos-1, fHandle, memPage);
    
    return ret;
}

RC readCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
    int ret;

    if(NULL == fHandle)
        return RC_FILE_HANDLE_NOT_INIT;

    ret = readBlock(fHandle->curPagePos-1, fHandle, memPage);
    
    return ret;
}

RC readNextBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
    int ret;

    if(NULL == fHandle)
        return RC_FILE_HANDLE_NOT_INIT;

    /*current page position is the last one, next page do not exist*/
    if(fHandle->totalNumPages == fHandle->curPagePos)
        return RC_READ_NON_EXISTING_PAGE;

    ret = readBlock(fHandle->curPagePos+1, fHandle, memPage);
    
    return ret;
}

RC readLastBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
    int ret;

    if(NULL == fHandle)
        return RC_FILE_HANDLE_NOT_INIT;

    ret = readBlock(fHandle->totalNumPages, fHandle, memPage);
    
    return ret;
}

/* writing blocks to a page file */
RC writeBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage){
    int ret;
    FILE *fp;
    long offset;  // pageNum is from 0 to n 
    int NUMBER_PAGES = 1;
    
    if (NULL == fHandle)
        return RC_FILE_HANDLE_NOT_INIT;

    /*Allow to append the page at the end of file*/
    if (pageNum > fHandle->totalNumPages)
        return RC_WRITE_FAILED;

    fp = fHandle->mgmtInfo;
    offset = pageNum * PAGE_SIZE;

    fseek(fp, offset, SEEK_SET);
    
    /*Write memPage to page*/
    ret = fwrite(memPage, PAGE_SIZE, NUMBER_PAGES, fp);
    if(ret != NUMBER_PAGES)
        return RC_WRITE_FAILED;

    fHandle->curPagePos = pageNum;

    /*If append the block to the file, increase the totalNumPage one*/
    if (pageNum = fHandle->totalNumPages)
        ++fHandle->totalNumPages;

    return ret; 
}

RC writeCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
    int ret;
    int cur_pagePos;

    if(NULL == fHandle)
        return RC_FILE_HANDLE_NOT_INIT;
    
    cur_pagePos = fHandle->curPagePos;    
    ret = writeBlock(cur_pagePos, fHandle, memPage);    
    
    return ret;
}

RC appendEmptyBlock (SM_FileHandle *fHandle){
    int ret;
    int last_pagePos;
    
    last_pagePos = fHandle->totalNumPages;
    SM_PageHandle EmptyPage = calloc(PAGE_SIZE, sizeof(char));
    
    ret = writeBlock(last_pagePos, fHandle, EmptyPage);    
 
    return ret; 
}

RC ensureCapacity (int numberOfPages, SM_FileHandle *fHandle){
    int ret;
    int i;
    int GapPages;

    GapPages = numberOfPages - fHandle->totalNumPages;

    if (GapPages > 0)
        while(GapPages) {
            ret = appendEmptyBlock(fHandle);
            if (ret != RC_OK)
                return ret;
            --GapPages;
        }
    return ret;
}
