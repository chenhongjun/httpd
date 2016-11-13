CC=g++ -std=c++11 -Wall -g


http : main.o ser.o
	$(CC) $^ -o $@ -lpthread

%.o : %.cpp
	$(CC) -c $< -o $@ -lpthread

clean:
	@rm *.o
