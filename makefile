all:
	gcc zze_ser.c -o server -lpthread
	gcc zze_cli.c -o client -lpthread
clean:
	rm server_v1 client_v1
