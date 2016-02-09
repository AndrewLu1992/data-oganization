Personnel information
Ting Ma  <tma11@hawk.iit.edu>
Shuo Yan<syan9@hawk.iit.edu >
Congwei She<cshe@hawk.iit.edu >
Chen Wang<Cwang90@hawk.iit.edu >

     File list
            
      Makefile
       README.txt
       dberror.c
       dberror.h
       storage_mgr.c
       storage_mgr.h
       test_assign1_1.c
       test_helper.h

      
      Milestone

      02/02/2015  Coding and unit test
      02/08/2015  Integration test
      02/09/2015  Deliver code to Blackboard

Installation instruction
1. make
cc -c dberror.c
cc -c storage_mgr.c
cc -c test_assign1_1.c
cc -o test_assign1_1 dberror.o storage_mgr.o test_assign1_1.o

2. ./test_assign1_1

[test_assign1_1.c-test single page content-L99-11:13:14] OK: finished test

3. Make Clean
rm *.o test_assign1_1 test_pagefile.bin 
rm: cannot remove ‘test_pagefile.bin’: No such file or directory


Function descriptions: of all additional functions


Additional error codes: of all additional error codes (leave if blank if there isn't any)

#define RC_FILE_CLOSE_FAILED 5
#define RC_FILE_DELETE_FAILED 6
#define INV_PARAMETER  7

Extra credit: of all extra credits (leave if blank if there isn't any)


Additional files: of all additional files


Test cases: of all additional test cases added (leave if blank if there isn't any)

Problems solved (leave if blank if there isn't any)

Problems to be solved (leave if blank if there isn't any)

