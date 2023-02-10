#PA2 cpp compiler
.PHONY: all run run_all execute checker check_all clean
all: 107501567_PA2.o
	@g++ -std=c++11 107501567_PA2.cpp -o 107501567_PA2.exe
run:
	@./107501567_PA2.exe $(input) $(output)
run_all:
	@./107501567_PA2.exe testcase1 testcase1.out
	@./107501567_PA2.exe testcase2 testcase2.out
	@./107501567_PA2.exe testcase3 testcase3.out
execute:
	@./107501567_PA2.exe $(input) $(input).out
	@./checker $(input) $(input).out
checker:
	@./checker $(input) $(input).out
check_all:
	@./checker testcase1 testcase1.out
	@./checker testcase2 testcase2.out
	@./checker testcase3 testcase3.out

clean:
	@rm *.out
	@rm *.exe
	@rm *.o
	
	
