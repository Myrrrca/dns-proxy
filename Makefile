TARGET = proxy
CC = gcc

PREF_SRC = ./src/
PREF_OBJ = ./obj/
PREF_INIH = ./lib/inih/
PREF_ERRPROC = ./lib/errproc/

SRC = $(wildcard $(PREF_SRC)*.c) $(wildcard $(PREF_INIH)*.c) $(wildcard $(PREF_ERRPROC)*.c)
OBJ = $(patsubst $(PREF_SRC)%.c, $(PREF_OBJ)%.o, $(filter $(PREF_SRC)%.c, $(SRC))) \
      $(patsubst $(PREF_INIH)%.c, $(PREF_OBJ)%.o, $(filter $(PREF_INIH)%.c, $(SRC))) \
      $(patsubst $(PREF_ERRPROC)%.c, $(PREF_OBJ)%.o, $(filter $(PREF_ERRPROC)%.c, $(SRC)))

.PHONY : clean

$(TARGET) : $(OBJ) 
	$(CC) $(OBJ) -Wall -o $(TARGET) 

$(PREF_OBJ)%.o : $(PREF_SRC)%.c
	$(CC) -c -Wall $< -o $@

$(PREF_OBJ)%.o : $(PREF_INIH)%.c
	$(CC) -c -Wall $< -o $@

$(PREF_OBJ)%.o : $(PREF_ERRPROC)%.c
	$(CC) -c -Wall $< -o $@

clean:
	rm $(TARGET) $(PREF_OBJ)*.o 
