
proxy: proxy.c
	gcc -Wformat -Wformat-signedness -std=gnu17 -Wall -pedantic proxy.c ./lib/inih/ini.c ./lib/errproc/errproc.c -o proxy

clean:
	rm -f proxy
