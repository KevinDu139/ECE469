default:
	cd makeprocs; make
	cd consumer; make
	cd producer; make

clean:
	cd makeprocs; make clean
	cd consumer; make clean
	cd producer; make clean

run:
	dlxsim -x ../../bin/os.dlx.obj -a -u bin/makeprocs.dlx.obj 3; ee469_fixterminal
