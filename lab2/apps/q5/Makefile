default:
	cd makeprocs; make
	cd injectN2; make
	cd injectH2O; make
	cd react1; make
	cd react2; make
	cd react3; make

clean:
	cd makeprocs; make clean
	cd injectN2; make clean
	cd injectH2O; make clean
	cd react1; make clean
	cd react2; make clean
	cd react3; make clean

run:
	dlxsim -x ../../bin/os.dlx.obj -a -u bin/makeprocs.dlx.obj 6 12; ee469_fixterminal
