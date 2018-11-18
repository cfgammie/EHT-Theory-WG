# Makefile for ...
# v0.1 DP 29 Sep 2017

#For the GNU C compiler
CC=gcc 
CFLAGS=-w

#Libraries
LDIR =/opt/local/lib 
LIBSGEN=-lm
LIBSFFT=-lfftw3 
LIBSFIT=-lcfitsio

#Header files
LHEAD=/opt/local/include

#Binaries
BINDIR=../bin

#Executables
EXEC=fitscopy imarith imcopy imlist imstat listhead liststruc\
     modhead tabcalc tablist tabmerge tabselect image2uv synthimage

#all rule
all: $(EXEC)

#fitstools
FITSDIR=Fits

fitscopy: $(FITSDIR)/fitscopy.c
	$(CC) $(CFLAGS) $(FITSDIR)/fitscopy.c -o $(BINDIR)/fitscopy -L$(LDIR) $(LIBSGEN) $(LIBSFIT)

imarith: $(FITSDIR)/imarith.c
	$(CC) $(CFLAGS) $(FITSDIR)/imarith.c -o $(BINDIR)/imarith -L$(LDIR) $(LIBSGEN) $(LIBSFIT)

imcopy: $(FITSDIR)/imcopy.c
	$(CC) $(CFLAGS) $(FITSDIR)/imcopy.c -o $(BINDIR)/imcopy -L$(LDIR) $(LIBSGEN) $(LIBSFIT)

imlist: $(FITSDIR)/imlist.c
	$(CC) $(CFLAGS) $(FITSDIR)/imlist.c -o $(BINDIR)/imlist -L$(LDIR) $(LIBSGEN) $(LIBSFIT)

imstat: $(FITSDIR)/imstat.c
	$(CC) $(CFLAGS) $(FITSDIR)/imstat.c -o $(BINDIR)/imstat -L$(LDIR) $(LIBSGEN) $(LIBSFIT)

listhead: $(FITSDIR)/listhead.c
	$(CC) $(CFLAGS) $(FITSDIR)/listhead.c -o $(BINDIR)/listhead -L$(LDIR) $(LIBSGEN) $(LIBSFIT)

liststruc: $(FITSDIR)/liststruc.c
	$(CC) $(CFLAGS) $(FITSDIR)/liststruc.c -o $(BINDIR)/liststruc -L$(LDIR) $(LIBSGEN) $(LIBSFIT)

modhead: $(FITSDIR)/modhead.c
	$(CC) $(CFLAGS) $(FITSDIR)/modhead.c -o $(BINDIR)/modhead -L$(LDIR) $(LIBSGEN) $(LIBSFIT)

tabcalc: $(FITSDIR)/tabcalc.c
	$(CC) $(CFLAGS) $(FITSDIR)/tabcalc.c -o $(BINDIR)/tabcalc -L$(LDIR) $(LIBSGEN) $(LIBSFIT)

tablist: $(FITSDIR)/tablist.c
	$(CC) $(CFLAGS) $(FITSDIR)/tablist.c -o $(BINDIR)/tablist -L$(LDIR) $(LIBSGEN) $(LIBSFIT)

tabmerge: $(FITSDIR)/tabmerge.c
	$(CC) $(CFLAGS) $(FITSDIR)/tabmerge.c -o $(BINDIR)/tabmerge -L$(LDIR) $(LIBSGEN) $(LIBSFIT)

tabselect: $(FITSDIR)/tabselect.c
	$(CC) $(CFLAGS) $(FITSDIR)/tabselect.c -o $(BINDIR)/tabselect -L$(LDIR) $(LIBSGEN) $(LIBSFIT)

# other commands
image2uv: image2uv.c io.o
	$(CC) $(CFLAGS) image2uv.c io.o -o $(BINDIR)/image2uv $(LIBSGEN) -L$(LDIR) -I$(LHEAD) $(LIBSFIT) $(LIBSFFT)	

synthimage: synthimage.c io.o modelsImage.o
	$(CC) $(CFLAGS) synthimage.c io.o modelsImage.o -o $(BINDIR)/synthimage $(LIBSGEN) -L$(LDIR) -I$(LHEAD) $(LIBSFIT) 

io.o: io.c
	$(CC) $(CFLAGS) -c io.c $(LIBSGEN) -L$(LDIR)  $(LIBSFIT)	

modelsImage.o: modelsImage.c 
	$(CC) $(CFLAGS) -c modelsImage.c $(LIBSGEN)

clean:
	rm -f *.o *.trace *~ $(FITSDIR)/*.o $(FITSDIR)/*.trace $(FITSDIR)/*~

.PHONY : all clean

