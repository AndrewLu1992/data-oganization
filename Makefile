OBJS=dberror.o storage_mgr.o test_assign3_1.o buffer_mgr.o buffer_mgr_stat.o buffer_mgr_algorithm.o expr.o record_mgr.o rm_serializer.o

all: $(OBJS)
	cc -o test_assign3_1 $(OBJS)

dberror.o: dberror.c dberror.h
	cc -c dberror.c

storage_mgr.o: storage_mgr.c storage_mgr.h dberror.h
	cc -c storage_mgr.c

test_assign3_1.o: test_assign3_1.c test_helper.h storage_mgr.h dberror.h
	cc -c test_assign3_1.c rm_serializer.c

rm_serializer.o: rm_serializer.c tables.h
	cc -c rm_serializer.c

buffer_mgr.o: buffer_mgr.c buffer_mgr.h
	cc -c buffer_mgr.c

buffer_mgr_algorithm.o: buffer_mgr_algorithm.c buffer_mgr_algorithm.h
	cc -c buffer_mgr_algorithm.c

buffer_mgr_stat.o: buffer_mgr_stat.c  buffer_mgr_stat.h buffer_mgr_algorithm.h
	cc -c buffer_mgr_stat.c 

expr.o: expr.c expr.h
	cc -c expr.c

record_mgr.o: record_mgr.c record_mgr.h
	cc -c record_mgr.c

.PHONY: clean
clean:
	-rm *.o test_assign3_1  
