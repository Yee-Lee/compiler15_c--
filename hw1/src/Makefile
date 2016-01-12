compile:
	gcc AcDc.c -o AcDc
clean:
	-rm AcDc *.dc
	-rm -rf ../output
	

TARGET = \
        sample1 \
	sample2 \
	sample33 \
	sample4 \
	sample5

OUTPUTS = $(addsuffix .dc, $(TARGET))

OUTPUTDIR:
	-mkdir ../output

%.dc : ../test/%.ac OUTPUTDIR
	./AcDc ../test/$*.ac ../output/$@

run: $(OUTPUTS)

All: clean compile run

test: compile run
	dc ../output/sample1.dc
	dc ../output/sample2.dc
	dc ../output/sample33.dc
	dc ../output/sample4.dc
	dc ../output/sample5.dc

#
#./AcDc 1.ac 1.dc
#./AcDc 2.ac 2.dc
#./AcDc 3.ac 3.dc
#...



