CC=g++ -std=c++11 -Wall -g


httpd : main.o ser.o
	$(CC) $^ -o $@ -lpthread

%.o : %.cpp
	$(CC) -c $< -o $@ -lpthread

clean:
	@rm *.o
	@rm httpd
