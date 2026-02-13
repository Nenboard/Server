// Made by Nenboard

#include <winsock.h>
#include <time.h>
#include <stdio.h>
#include <threads.h>
#include <wininet.h>
#include <conio.h>

char status[1024] = "HTTP/1.1 200 OK\n\n";
char response[1024 * 1024]{};
char index[1024 * 1024]{};
bool loop;
sockaddr_in ip{};
char ips[16][16]{};
int banneds;
char excs[1024][1024]{};
int chars;
char buffer[1024]{};
char line[1024]{};
char url[1024]{};
char message[1024]{};
char take[1024]{};

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

static int messages()
{
	int step = 0;
	while (buffer[step] != '\r')
	{
		line[step] = buffer[step];
		step++;
	}
	int save = step;

	while (line[step] != ' ')
	{
		line[step] = '\0';
		if (step > 0)
		{
			step--;
		}
	}
	line[step] = '\0';

	step = 0;
	while (line[step] != '/')
	{
		step++;
	}
	step++;

	int replace = 0;
	while (step < save)
	{
		url[replace] = line[step];
		replace++;
		step++;
	}

	int len = 1024;
	InternetCanonicalizeUrlA(url, message, (DWORD*)&len, ICU_DECODE | ICU_NO_ENCODE);

	bool handle = true;
	step = 0;
	while (step < chars)
	{
		if (!strcmp(message, excs[step]))
		{
			handle = false;
		}
		step++;
	}

	if (strlen(message) == 0)
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
	}

	if (strlen(message) > 0 and handle)
	{
		time();
		printf("Message: %s\n", message);

		FILE* log;
		bool result = fopen_s(&log, "log.txt", "a");
		if (!result)
		{
			fprintf(log, "Message: %s\n", message);
			fclose(log);
		}

		step = 0;
		while (step < strlen(message))
		{
			take[step] = message[step];
			step++;
		}
		take[step] = '\0';

		FILE* chat;
		result = fopen_s(&chat, "messages.txt", "a");
		if (!result)
		{
			if (take[0] != '\0')
			{
				strcat_s(take, "\n");
			}
			fprintf(chat, take);
			fclose(chat);
		}
	}

	return 0;
}

static int clients(SOCKET client)
{
	recv(client, buffer, 1024, NULL);
	messages();
	send(client, response, (int)strlen(response), NULL);
	closesocket(client);

	return 0;
}

static int accepts(SOCKET server)
{
	SOCKET client;
	int len = 16;
	thrd_t threads[1]{};

	while (loop)
	{
		client = accept(server, NULL, NULL);
		if (client != INVALID_SOCKET)
		{
			if (inet_ntoa(ip.sin_addr) != NULL)
			{
				getpeername(client, (sockaddr*)&ip, &len);
			}
			int step = 0;
			bool handle = true;
			while (step < banneds)
			{
				if (!strcmp(inet_ntoa(ip.sin_addr), ips[step]))
				{
					handle = false;
				}
				step++;
			}

			if (handle)
			{
				thrd_create(threads, (thrd_start_t)clients, (SOCKET*)(intptr_t)client);
			}
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
		port = 8000;
	}

	SOCKET server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	sockaddr_in addres_server{};
	addres_server.sin_family = AF_INET;
	addres_server.sin_addr.s_addr = htonl(INADDR_ANY);
	addres_server.sin_port = htons(port);
	bind(server, (sockaddr*)&addres_server, sizeof(addres_server));
	listen(server, SOMAXCONN);

	FILE* log;
	result = fopen_s(&log, "log.txt", "w");
	if (!result)
	{
		fprintf(log, "");
		fclose(log);
	}

	time();
	printf("The server port is: %i\n", port);

	result = fopen_s(&log, "log.txt", "a");
	if (!result)
	{
		fprintf(log, "The server port is: %i\n", port);
		fclose(log);
	}

	FILE* site;
	result = fopen_s(&site, "index.html", "r");
	if (!result)
	{
		result = true;
		char line[1024]{};
		while (result)
		{
			strcat_s(index, line);
			result = fgets(line, 1024, site);
		}
		fclose(site);

		int step = 0;
		while (step < strlen(status))
		{
			response[step] = status[step];
			step++;
		}
		strcat_s(response, index);
	}

	FILE* ip;
	result = fopen_s(&ip, "ips.txt", "r");
	if (!result)
	{
		int step = 0;
		result = true;
		while (result)
		{
			result = fgets(ips[step], 16, ip);
			if (ips[step][strlen(ips[step]) - 1] == '\n')
			{
				ips[step][strlen(ips[step]) - 1] = '\0';
			}
			step++;
		}
		fclose(ip);
		banneds = step;
	}

	FILE* exc;
	result = fopen_s(&exc, "exceptions.txt", "r");
	if (!result)
	{
		int step = 0;
		result = true;
		while (result)
		{
			result = fgets(excs[step], 16, exc);
			if (excs[step][strlen(excs[step]) - 1] == '\n')
			{
				excs[step][strlen(excs[step]) - 1] = '\0';
			}
			step++;
		}
		fclose(exc);
		chars = step;
	}

	loop = true;
	thrd_t threads[1]{};
	thrd_create(threads, (thrd_start_t)accepts, (SOCKET*)(intptr_t)server);

	int key;
	do
	{
		key = _getch();
	} while (key != 27);

	loop = false;
	closesocket(server);
	WSACleanup();
	return 0;
}
