WORK_ROOT = ../../..
THIRD64 = $(WORK_ROOT)/lib

CC = gcc
AR = ar

SRC = $(wildcard *.c)
OBJS = $(SRC:%.c=%.o)
LIB = libmurl.a

INCLUDE = -I../include \
		  -I$(THIRD64)/curl/include 

CFLAG = -g -Wall -fPIC

# remember to add curl and cares and so many others libs 
# when using murl

#LDFLAG = -L$(THIRD64)/curl/lib -lcurl -lcares\
		 -lgssapi_krb5 -lkrb5 -lk5crypto -lcom_err \
		 -lidn -lssl -lcrypto -lkrb5support -lkeyutils \
		 -lselinux -lsepol -lldap -lrt \

all: $(LIB)
	cp $(LIB) ../lib
	# ATTENTION: Your Makefile must include LDFLAG = -L$(THIRD64)/curl/lib -lcurl -lcares -lgssapi_krb5 -lkrb5 -lk5crypto -lcom_err -lidn -lssl -lcrypto -lkrb5support -lkeyutils -lselinux -lsepol -lldap -lrt 

$(LIB) : $(OBJS)
	$(AR) -r $@ $^

.c.o:
	$(CC) $(CFLAG) $(INCLUDE) -c $^

clean:
	rm -f $(LIB) $(OBJS)

rebuild: clean all
