CC=gcc

OPT_DEBUG=-Wall -O # -Wredundant-decls -Wnested-externs -Wstrict-prototypes  -Wmissing-prototypes -Wpointer-arith -Winline -Wcast-qual  -fformat-extensions -ansi -mpreferred-stack-boundary=2
OPT_DBG=-Werror -Wno-unused -ffreestanding
OPT_DEF=-D_KERNEL -DKLD_MODULE -DFISTGEN
OPT_INC=-I. -I$(SYS_SRC)
CFLAGS=$(OPT_DEBUG) $(OPT_DEF) $(OPT_DBG) $(OPT_INC)
UCFLAGS=$(OPT_DEBUG) -DFISTGEN $(OPT_INC)

SYS_SRC=/sys
KLD=/modules

TARGET = snoopfs.ko
BINS = fist_ioctl mount_snoopfs 

SRC = vnode.c vfs.c print.c subr.c fist_snoopfs.c  
OBJ = ${SRC:.c=.o}


all: $(TARGET) $(BINS)

$(TARGET): $(OBJ)
	ld -Bshareable -o $(TARGET) $(OBJ)

vnode_if.h:
	awk -f $(SYS_SRC)/tools/vnode_if.awk  $(SYS_SRC)/kern/vnode_if.src -h



fist_ioctl: fist_ioctl.c
	${CC} -o $@ $? ${UCFLAGS}

mount_snoopfs: mount_snoopfs.c getmntopts.c
	${CC} -o $@ $? ${UCFLAGS}



$(OBJ): fist.h snoopfs.h vnode_if.h

install: $(TARGET)
	install -c -m 775 $(TARGET) $(KLD)

clean:
	rm -f *~ \#* core *.d .*.flags $(OBJ) $(OBJ0) $(OBJ1)
	rm -f $(TARGET) vnode_if.h
	rm -f setdefs.h setdef0.c setdef1.c $(BINS)

#-include *.d
