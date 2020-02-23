WORKING_DIR:=$(shell pwd)
TARGET=tb_PE.cpp

all: debug

debug:
	g++ -std=c++11 -g -I. -I./systemc/include -L. -Lsystemc/lib-linux64 -lsystemc -lm -Wl,-rpath,./systemc/lib-linux64 $(TARGET) -o sim
release:
	g++ -I. -I./systemc/include -L. -Lsystemc/lib-linux64 -lsystemc -lm -Wl,-rpath,./systemc/lib-linux64 $(TARGET) -o sim
clean:
	rm -rf ./sim
run:
	./sim
	
gtkwave:
	ssh gateway -X gtkwave $(WORKING_DIR)/$(WAVE) && echo "exit"