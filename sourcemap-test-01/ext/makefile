CPP = emcc

libs/libmean.a : mean.o
	$(AR) rcs libs/libmean.a mean.o

mean.o : mean.c
	$(CPP) -c mean.c -o mean.o -g4

clean:
	rm mean.o libs/libmean.a
