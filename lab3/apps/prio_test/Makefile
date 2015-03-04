default:
	cd makeprocs; make
	cd spawn_me; make

clean:
	cd makeprocs; make clean
	cd spawn_me; make clean

run:
	dlxsim -D p -x ../../bin/os.dlx.obj -a -u bin/makeprocs.dlx.obj 3; ee469_fixterminal
