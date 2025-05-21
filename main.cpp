// Made by Nenboard

#include <winsock.h>
#include <time.h>
#include <stdio.h>
#include <threads.h>
#include <conio.h>

char response[1024 * 1024] = "HTTP/1.1 200 OK\n\n";
char site[1024 * 1024]{};
bool start;
char message[1024]{};

static int time()
{
	time_t since;
	tm time_message{};

	time(&since);
	localtime_s(&time_message, &since);
	printf("%02i:%02i:%02i ", time_message.tm_hour, time_message.tm_min, time_message.tm_sec);

	FILE* log;
	bool result = fopen_s(&log, "log.txt", "a");
	if (!result)
	{
		fprintf(log, "%02i:%02i:%02i ", time_message.tm_hour, time_message.tm_min, time_message.tm_sec);
		fclose(log);
	}

	return 0;
}

static int clients(SOCKET client)
{
	recv(client, message, 1024, NULL);
	send(client, response, (int)strlen(response), NULL);
	closesocket(client);

	return 0;
}

static int accepts(SOCKET server)
{
	SOCKET client;
	struct sockaddr_in ip{};
	int socklen = sizeof(ip);
	thrd_t threads[1]{};

	while (start)
	{
		client = accept(server, (sockaddr*)&ip, &socklen);
		if (client != INVALID_SOCKET)
		{
			time();
			printf("New connection: %s\n", inet_ntoa(ip.sin_addr));

			FILE* log;
			bool result = fopen_s(&log, "log.txt", "a");
			if (!result)
			{
				fprintf(log, "New connection: %s\n", inet_ntoa(ip.sin_addr));
				fclose(log);
			}

			thrd_create(threads, (thrd_start_t)clients, (SOCKET*)(intptr_t)client);
		}
	}

	return 0;
}

int main(int count, char** arguments)
{
	WSADATA data;
	bool result = WSAStartup(0x101, &data);
	if (result)
	{
		return 1;
	}

	int port;
	if (count >= 2)
	{
		port = atoi(arguments[1]);
	}
	else
	{
		port = 80;
	}

	SOCKET server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	sockaddr_in addres_server{};
	addres_server.sin_family = AF_INET;
	addres_server.sin_addr.s_addr = htonl(INADDR_ANY);
	addres_server.sin_port = htons(port);
	bind(server, (sockaddr*)&addres_server, sizeof(addres_server));
	listen(server, SOMAXCONN);

	FILE* cleanlog;
	result = fopen_s(&cleanlog, "log.txt", "w");
	if (!result)
	{
		fprintf(cleanlog, "");
		fclose(cleanlog);
	}

	time();
	printf("The server port is: %i\n", port);

	FILE* log;
	result = fopen_s(&log, "log.txt", "a");
	if (!result)
	{
		fprintf(log, "The server port is: %i\n", port);
		fclose(log);
	}

	FILE* opensite;
	result = fopen_s(&opensite, "site.html", "r");
	if (!result)
	{
		result = true;
		char line[1024]{};
		while (result)
		{
			strcat_s(site, line);
			result = fgets(line, 1024, opensite);
		}
		fclose(opensite);
		strcat_s(response, site);
	}

	start = true;
	thrd_t threads[1]{};
	thrd_create(threads, (thrd_start_t)accepts, (SOCKET*)(intptr_t)server);

	int key;
	do
	{
		key = _getch();
	} while (key != 27);

	start = false;
	closesocket(server);
	WSACleanup();
	return 0;
}
