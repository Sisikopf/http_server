#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

void headers(int client, int size, int httpcode, char *contentType) {
	char buf[1024];
	char strsize[20];
	sprintf(strsize, "%d", size);
	if (httpcode == 200) {
		strcpy(buf, "HTTP/1.0 200 OK\r\n");
	}
	else if (httpcode == 404) {
		strcpy(buf, "HTTP/1.0 404 Not Found\r\n");
	}
	else {
		strcpy(buf, "HTTP/1.0 500 Internal Server Error\r\n");
	}
	send(client, buf, strlen(buf), 0);
	strcpy(buf, "Connection: keep-alive\r\n");
	send(client, buf, strlen(buf), 0);
	strcpy(buf, "Content-length: ");
	send(client, buf, strlen(buf), 0);
	strcpy(buf, strsize);
	send(client, buf, strlen(buf), 0);
	strcpy(buf, "\r\n");
	send(client, buf, strlen(buf), 0);
	strcpy(buf, "simple-server\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, contentType);
	send(client, buf, strlen(buf), 0);
	strcpy(buf, "\r\n");
	send(client, buf, strlen(buf), 0);
}


void parseFileName(char *line, char **filepath, size_t *len) {
	char *start = NULL;
	while ((*line) != '/') line++;
	start = line + 1;
	while ((*line) != ' ') line++;
	(*len) = line - start;
	*filepath = (char*)malloc(*len + 1);
	*filepath = strncpy(*filepath, start, *len);
	(*filepath)[*len] = '\0';
	printf("%s \n", *filepath);
}
int setContentType(char *filepath, char **contentType) {
    if(strstr(filepath, ".jpg")!=NULL) {
        *contentType = "Content-Type: image/jpeg\r\n";
        return 1;
    }
    else if(strstr(filepath, ".txt")!=NULL) {
        *contentType = "Content-Type: text/plain\r\n";
        return 2;
    }
    else if(strstr(filepath, ".html")!=NULL) {
        *contentType = "Content-Type: text/html\r\n";
        return 3;
    }
    else if(strstr(filepath, ".css")!=NULL) {
        *contentType = "Content-Type: text/css\r\n";
        return 4;
    }
    else if(strstr(filepath, ".png")!=NULL) {
        *contentType = "Content-Type: image/png\r\n";
        return 5;
    }
    else if(strstr(filepath, ".pdf")!=NULL) {
        *contentType = "Content-Type: application/pdf\r\n";
        return 6;
    }
    else if(strstr(filepath, ".ico")!=NULL) {
        *contentType = "Content-Type: image/x-icon\r\n";
        return 7;
    }
    else return NULL;
}
int main() {
	int ld = 0;
	int res = 0;
	int cd = 0;
	int filesize = 0;
	const int backlog = 10;
	struct sockaddr_in saddr;
	struct sockaddr_in caddr;
	char *line = NULL;
	size_t len = 0;
	char *filepath = NULL;
	size_t filepath_len = 0;
	int empty_str_count = 0;
	socklen_t size_saddr;
	socklen_t size_caddr;
	FILE *fd;
	FILE *file;
    char *contentType = (char*) malloc(255*sizeof(char));
	ld = socket(AF_INET, SOCK_STREAM, 0);
	if (ld == -1) {
		printf("listener create error \n");
	}
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(8085);
	saddr.sin_addr.s_addr = INADDR_ANY;
	res = bind(ld, (struct sockaddr *)&saddr, sizeof(saddr));
	if (res == -1) {
		printf("bind error \n");
	}
	res = listen(ld, backlog);
	if (res == -1) {
		printf("listen error \n");
	}
	while (1) {
		cd = accept(ld, (struct sockaddr *)&caddr, &size_caddr);
		if (cd == -1) {
			printf("accept error \n");
		}
		printf("client in %d descriptor. Client addr is %d \n", cd, caddr.sin_addr.s_addr);
		fd = fdopen(cd, "r");
		if (fd == NULL) {
			printf("error open client descriptor as file \n");
		}
		while ((res = getline(&line, &len, fd)) != -1) {
			if (strstr(line, "GET")) {
				parseFileName(line, &filepath, &filepath_len);
			}
			if (strcmp(line, "\r\n") == 0) {
				empty_str_count++;
			}
			else {
				empty_str_count = 0;
			}
			if (empty_str_count == 1) {
				break;
			}
			printf("%s", line);
		}
		printf("open %s \n", filepath);

		file = fopen(filepath, "rb");
		if (file == NULL) {
			printf("404 File Not Found \n");
			headers(cd, 0, 404, contentType);
		}
		else if (file!=NULL && setContentType(filepath, &contentType)==NULL)
		{
			printf("500 Internal Server Error \n");
			headers(cd, 0, 500, contentType);
		}
		else if (file!=NULL && setContentType(filepath, &contentType)!=NULL){
			fseek(file, 0L, SEEK_END);
			filesize = ftell(file);
			fseek(file, 0L, SEEK_SET);
			printf("%s", contentType);
			headers(cd, filesize, 200, contentType);
			unsigned char buf[filesize];
			int n = fread(buf,filesize,1,file);
			if(n==0)
				printf("Read file Error");
			res = send(cd, buf, filesize, 0);
			if (res == -1) {
				printf("send error \n");
			}
		}
		close(cd);
	}
	return 0;
}

