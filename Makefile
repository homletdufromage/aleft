all:
	cd receiver; make
	cd sender; make

clean:
	cd receiver; make clean
	cd sender; make clean
