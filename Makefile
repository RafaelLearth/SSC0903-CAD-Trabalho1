CC = gcc
CFLAGS = -O2 -Wall -Wextra -std=c11
OMPFLAGS = -fopenmp
LDLIBS = -lm

TARGET_SEQ = studentsseq
TARGET_PAR = studentspar
TARGET_AVAL = avaliacao_cod

SRC_SEQ = studentsseq.c
SRC_PAR = studentspar2.c
SRC_AVAL = avaliacao_cod.c

.PHONY: all runseq runpar runaval run clean

all: $(TARGET_SEQ) $(TARGET_PAR) $(TARGET_AVAL)

$(TARGET_SEQ): $(SRC_SEQ)
	$(CC) $(CFLAGS) $(OMPFLAGS) -o $(TARGET_SEQ) $(SRC_SEQ) $(LDLIBS)

$(TARGET_PAR): $(SRC_PAR)
	$(CC) $(CFLAGS) $(OMPFLAGS) -o $(TARGET_PAR) $(SRC_PAR) $(LDLIBS)

$(TARGET_AVAL): $(SRC_AVAL)
	$(CC) $(CFLAGS) -o $(TARGET_AVAL) $(SRC_AVAL) $(LDLIBS)

runseq: $(TARGET_SEQ)
	./$(TARGET_SEQ) < entrada.txt

runpar: $(TARGET_PAR)
	./$(TARGET_PAR) < entrada.txt

runaval: $(TARGET_AVAL)
	./$(TARGET_AVAL)

run: runseq runpar runaval

clean:
	rm -f $(TARGET_SEQ) $(TARGET_PAR) $(TARGET_AVAL)

#make compila os dois
#make runseq executa o sequencial
#make runpar executa o paralelo
#make run executa os dois em sequência