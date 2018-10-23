all:
	@echo 'nothing to do'

.PHONY: test
test:
	./test

.PHONY: check
check:
	awk 'length($$0) > 80 { exit(1); }' < Makefile
	awk 'length($$0) > 80 { exit(1); }' < path.bash
	awk 'length($$0) > 80 { exit(1); }' < test
	shellcheck path.bash
	shellcheck -e SC1091,SC2034 test
