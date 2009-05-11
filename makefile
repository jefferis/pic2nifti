NIFTI_DIR = /Users/huiz/unix/tools/nifticlib-1.1.0
NIFTI_INCS = -I$(NIFTI_DIR)/include
NIFTI_LIBS = -L$(NIFTI_DIR)/lib -lniftiio -lznz -lz
LIBS= -L. -lpic2nifti

TARGET = libpic2nifti.o
LIB=libpic2nifti.a
BIN=pic2nifti

#CFLAGS = -g -O3 -arch i386 -arch ppc
CFLAGS = -O3 -arch i386 -arch ppc
FLAGS = -arch i386 -arch ppc

all : $(BIN)

$(BIN) : lib pic2nifti.o
	cc $(FLAGS) pic2nifti.o -o pic2nifti $(NIFTI_LIBS) $(LIBS)

lib : $(TARGET)
	rm -fr $(LIB)
	ar rc $(LIB) $(TARGET)
	ranlib $(LIB)

clean :
	rm -fr *.o *.a pic2nifti

% : %.o
	$(CC) $(FLAGS) $< -o $@ $(NIFTI_LIBS) $(LIBS)

%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@ $(NIFTI_INCS)

