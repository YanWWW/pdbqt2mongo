# Usage: make pdbqt2mongo
# Before executing program, remeber run "module load mongodb-api/c-1.4.2"
MONGO_ROOT=/WORK/app/mongo-api/c/1.4.2
LIBS := -L$(MONGO_ROOT)/lib -lbson-1.0 -lmongoc-1.0 -lsasl2
CFLAGS := -I$(MONGO_ROOT)/include/libbson-1.0 -I$(MONGO_ROOT)/include/libmongoc-1.0 -std=c99
#DEBUG := -ggdb
OPTS := -O2
CC := gcc
object=pdbqt2mongo

$(object):$(object).c
        $(CC) -o $(object) $(object).c $(CFLAGS) $(LIBS)
clean:
        rm -rf $(object) $(object).o
