clean:
	rm -rf *.o *.gv *.out
	rm -rf output.s


init:
	./run.sh pattern/init1.c
par1:
	./run.sh pattern/par1.c
par2:
	./run.sh pattern/par2.c
for:
	./run.sh pattern/for1.c
sc0:
	./run.sh pattern/sc0.c
sc1:
	./run.sh pattern/sc1.c
tc:
	./run.sh pattern/tc1.c
arr1:
	./run.sh pattern/arr1.c
arr2:
	./run.sh pattern/arr2.c
arr3:
	./run.sh pattern/arr3.c
while:
	./run.sh pattern/while.c

programs = \
	test1 \
	test2 \
	test3 \
	test4 \
	test5 \
	test6 \
	test7 \
	test8 \
	test9 \
	test10

output:
	mkdir -p $@

$(programs):
	./run.sh pattern/$@.c


all: output $(programs)
	
vmclean:
	-rm ../hw5/vm_share/*.out
clean: vmclean
	-rm pattern/*.out