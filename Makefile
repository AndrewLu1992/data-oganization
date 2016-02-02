OBJS=dberror.o storage_mgr.o test_assign1_1.o

all: $(OBJS)
	cc -o test_assign1_1 $(OBJS)

dberror.o: dberror.c dberror.h
	cc -c dberror.c

storage_mgr.o: storage_mgr.c storage_mgr.h dberror.h
	cc -c storage_mgr.c

test_assign1_1.o: test_assign1_1.c test_helper.h storage_mgr.h dberror.h
	cc -c test_assign1_1.c

.PHONY: clean
clean:
	-rm *.o test_assign1_1 

