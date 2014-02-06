# valgrind executed on remote linux machine

VFLAGS = --track-origins=yes --tool=memcheck --leak-check=yes --error-exitcode=1

rsync:
	ssh udesktop 'rm -rf tmp/ee'
	rsync -ra -e ssh --exclude '/.git' --exclude '/bin' . udesktop:tmp/ee

grind: $(TESTS) 
	set -e; for file in $^; do echo "\n\033[00;32m+++ $$file +++\033[00m\n" && valgrind $(VFLAGS) ./$$file; done

rgrind: rsync
	ssh udesktop 'cd tmp/ee && make clean && make grind'

grind-report: $(TESTS) 
	for file in $^; do \
		echo "\n \033[00;34m+++ $$file +++ \033[00m\n" && \
		G_SLICE=always-malloc G_DEBUG=gc-friendly \
		valgrind $(VFLAGS) -v --num-callers=40 --log-file=valgrind.log ./$$file; \
	done

rgrind-report: rsync
	ssh udesktop 'cd tmp/ee && make clean && make grind-report'
	scp udesktop:tmp/ee/valgrind.log .
	cat valgrind.log
