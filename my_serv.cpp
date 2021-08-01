//
// Created by Phylicia Melinda on 7/30/21.
//

//подключить либы из сабдж + доп
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>

int id[64*1024];
int max = 0, next_id = 0;
char bufRead[42*4096], bufStr[42*4096], bufWrite[42*4096];
fd_set active, readyWrite, readyRead;

// разослать если засечен текущий фд и не себе
void fatal_error(){
	write(2, "Fatal error\n", 12);
	exit(1);
}

void send_all(int except){
	for(int i = 0; i <= max; i++)
		if(FD_ISSET(i, &readyWrite) && i != except)
			send(i, bufWrite, strlen(bufWrite), 0 );
}


int main(int ac, char *av[]){

	if(ac != 2){
		write(2, "Wrong num\n", 10);
		exit(1);
	}

	int port = atoi(av[1]);

	bzero(&id, sizeof(id));
	FD_ZERO(&active);

	int servSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (servSocket < 0)
		fatal_error();
	// взять сокет

	//взять сокет текущему серверу + проверка

	max = servSocket;
	FD_SET(servSocket, &active);

	// перезапись макс фд
	// засетить этот сокет в активные

	struct sockaddr_in servaddr;
	socklen_t len;
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = (1 << 24) | 127;
	servaddr.sin_port = (port >> 8) | (port << 8);

	//адрес сервера, битовая маска

	// bind
	if ((bind(servSocket, (const struct sockaddr *)&servaddr, sizeof(servaddr))) < 0) //?
		fatal_error();
	// listen 128
	if (listen(servSocket, 128) < 0)
		fatal_error();

	//прировнять все фд сеты к активным

	while(1){
		readyRead = readyWrite = active;
		// селект максим число открытых сейчас + 1с проверкой

		if(select(max + 1, &readyRead, &readyWrite, NULL, NULL) < 0)
			continue;

		for(int s = 0; s <= max; s++){

			if(FD_ISSET(s, &readyRead) && s == servSocket){

				int clientSock = accept(servSocket, (struct sockaddr *)&servaddr, &len);
				if (clientSock < 0)
					continue;

				max = (clientSock > max) ? clientSock : max;
				id[clientSock] = next_id++;
				FD_SET(clientSock, &active);

				sprintf(bufWrite, "server: client %d arrived\n", id[clientSock]);
				send_all(clientSock);
				break;
			}

			if(FD_ISSET(s, &readyRead) && s != servSocket){

				int res = recv(s, &bufRead, 42*4096, 0);

				if(res <= 0){
					sprintf(bufWrite, "client just left: %d\n", id[s]);
					send_all(s);
					FD_CLR(s, &active);
					close(s);
					break;

				} else {
					for(int i = 0, j = 0; i < res; i++, j++ ){
						bufStr[j] = bufRead[i];
						if(bufStr[j] == '\n'){
							bufStr[j] = '\0';
							sprintf(bufWrite, "client : %d %s\n", id[s], bufStr);
							send_all(s);
							j = -1;
						}
					}

				}
			}
		}

		//засетить текущий фд и если этот s наш слушающий сервер
		// принять на наш север и выдать фд новому клиенту с проверкой


		//перезаписать макс в зависимости от клиента
		// записать нового в массив, увеличиваем
		// засетить нового клиента в сет активных

		// собрать в строку, разослать и брейкнуться


		//второй иф если фд засечен на чтение и не наш сервер

		// принять сообщение через recv


		// если ничего не пришло собираем строку, отправляем, чистим фд, закрываем соединение, брейкаемся

		// в ином случаем в цикле до момента сколько байт было прочитано
		// перезаписываем в строку буфера
		// если \n то зануляем ее в конце, собираем строку, отправляем всем и j
	}
}