NAME:=test.exe
SRC:=leptjson.c leptjson.h test.c
OBJ:=leptjson.o test.o

$(NAME): $(OBJ)
	gcc $^ -o $@
	del *.o *.gch

$(OBJ): $(SRC)
	gcc $^ -c

clean:
	del $(NAME)
	del *.o
run:
	$(NAME)