default:
	cd makeprocs; make
	cd producer; make
	cd consumer; make

clean:
	cd makeprocs; make clean
	cd producer; make clean
	cd consumer; make clean

run:
	dlxsim -x ../../bin/os.dlx.obj -a -u bin/makeprocs.dlx.obj 3; ee469_fixterminal
