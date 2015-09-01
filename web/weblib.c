#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/socket.h>  
#include <arpa/inet.h>
#include <netdb.h>  
#include <string.h>  
#include "cJSON.h"
#include <errno.h>
#include "weblib.h"
#if 0
//http get 
#define HOST "www.baidu.com"  
#define PAGE "/"  
#define PORT 8080
#define USERAGENT "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/30.0.1599.114 Safari/537.36"  
#define ACCEPTLANGUAGE "zh-CN,zh;q=0.8,en;q=0.6,en-US;q=0.4,en-GB;q=0.2"  
#define ACCEPTENCODING "gzip,deflate,sdch"  
char *build_get_query(char *host,char *page){  
    char *query;  
    char *getpage=page;  
    char *tpl="GET %s HTTP/1.1\r\nHost:%s\r\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\nUser-Agent:%s\r\nAccept-Language:%s\r\n\r\n";//Accept-Encoding:%s\r\n  
    query=(char *)malloc(strlen(host)+strlen(getpage)+strlen(USERAGENT)+strlen(tpl)+strlen(ACCEPTLANGUAGE)-5);//+strlen(ACCEPTENCODING)  
    sprintf(query,tpl,getpage,host,USERAGENT,ACCEPTLANGUAGE);//ACCEPTENCODING  
    return query;  
}  
char *get_ip(char *host){  
    struct hostent *hent;  
    int iplen=15;  
    char *ip=(char *)malloc(iplen+1);  
    memset(ip,0,iplen+1);  
    if((hent=gethostbyname(host))==NULL){  
        perror("Can't get ip");  
        exit(1);  
    }  
    if(inet_ntop(AF_INET,(void *)hent->h_addr_list[0],ip,iplen)==NULL){  
        perror("Can't resolve host!\n");  
        exit(1);  
    }  
    return ip;  
}  
void usage(){  
    fprintf(LOG_PREFXstderr,"USAGE:htmlget host [page]\n\thost:the website hostname. ex:www.baidu.com\n\tpage:the page to retrieve. ex:index.html,default:/\n");  
}  
int create_tcp_socket(){  
    int sock;  
    if((sock=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP))<0){  
        perror("Can't create TCP socket!\n");  
        exit(1);  
    }  
    return sock;  
} 
int htpp_get(int argc,char *argv[])
{
	struct sockaddr_in *remote;  
    int sock;  
    int tmpres;  
    char *ip;  
    char *get;  
    char buf[BUFSIZ+1];  
    char *host;  
    char *page;  
  
  
    if(argc==1){  
        usage();  
        exit(2);  
    }  
    host=argv[1];  
    if(argc>2){  
        page=argv[2];  
    }else{  
        page=PAGE;  
    }  
    fprintf(LOG_PREFXstdout,"page:%s,hostName:%s\n",page,host);  
    sock=create_tcp_socket();  
    ip=argv[1];//get_ip(host);  
    fprintf(LOG_PREFXstderr,"IP is %s\n",ip);  
    remote=(struct sockaddr_in *)malloc(sizeof(struct sockaddr_in*));  
    remote->sin_family=AF_INET;  
    tmpres=inet_pton(AF_INET,ip,(void *)(&(remote->sin_addr.s_addr)));  
    if(tmpres<0){  
        perror("Can't set remote->sin_addr.s_addr");  
        exit(1);  
    }else if(tmpres==0){  
        fprintf(LOG_PREFXstderr,"%s is not a valid IP address\n",ip);  
        exit(1);  
    }  
    remote->sin_port=htons(PORT);  
    if(connect(sock,(struct sockaddr *)remote,sizeof(struct sockaddr))<0){  
        perror("Could not connect!\n");  
        exit(1);  
    }  
    get =build_get_query(host,page);  
    fprintf(LOG_PREFXstdout,"<start>\n%s\n<end>\n",get);  
    int sent=0;  
    while(sent<strlen(get)){  
        tmpres=send(sock,get+sent,strlen(get)-sent,0);  
        if(tmpres==-1){  
            perror("Can't send query!");  
            exit(1);  
        }  
        sent+=tmpres;  
    }  
    memset(buf,0,sizeof(buf));  
    int htmlstart=0;  
    char *htmlcontent;  
    while((tmpres=recv(sock,buf,BUFSIZ,0))>0){  
        if(htmlstart==0){  
            htmlcontent=strstr(buf,"\r\n\r\n");  
            if(htmlcontent!=NULL){  
                htmlstart=1;  
                htmlcontent+=4;  
            }  
        }else{  
            htmlcontent=buf;  
        }  
        if(htmlstart){  
            fprintf(LOG_PREFXstdout,"%s",htmlcontent);  
        }  
        memset(buf,0,tmpres);  
       // fprintf(LOG_PREFXstdout,"\n\n\ntmpres Value:%d\n",tmpres);  
    }  
    fprintf(LOG_PREFXstdout,"\nreceive data over!\n");  
    if(tmpres<0){  
        perror("Error receiving data!\n");  
    }  
    free(get);  
    free(remote);  
    //free(ip);  
    close(sock);  
    return 0;  
}
#else
#define BUFFER_SIZE 1024  
#define HTTP_POST "POST /%s HTTP/1.1\r\nHOST: %s:%d\r\nAccept: */*\r\n"\
    "Content-Type:application/x-www-form-urlencoded\r\nContent-Length: %d\r\n\r\n%s"  
#define HTTP_GET "GET /%s HTTP/1.1\r\nHOST: %s:%d\r\nAccept: */*\r\n\r\n"  
#define MY_HTTP_DEFAULT_PORT 8080  
  
static int http_tcpclient_create(const char *host, int port,int timeout){  
    struct hostent *he;  
    struct sockaddr_in server_addr;   
    int socket_fd; 
	socklen_t lon; 
	int error;
	fd_set rset, wset;
	unsigned int len;
	struct timeval tv; 
	int valopt,ret; 
  	int imode=1,i=0;
    if((he = gethostbyname(host))==NULL){  
		printf(LOG_PREFX"gethostbyname failed\n");
        return -1;  
    }  
  
   server_addr.sin_family = AF_INET;  
   server_addr.sin_port = htons(port);  
   server_addr.sin_addr = *((struct in_addr *)he->h_addr);  
  
    if((socket_fd = socket(AF_INET,SOCK_STREAM,0))==-1){  
        return -1;  
    }  
	tv.tv_usec=0;
	tv.tv_sec = timeout;

   	fcntl(socket_fd, F_SETFL, O_NONBLOCK);
	if(connect(socket_fd, (struct sockaddr *)&server_addr,sizeof(struct sockaddr)) <0)
	{
		if(errno!=EINPROGRESS)
		{
			printf(LOG_PREFX"connect failed\n");
			close(socket_fd);
			return -1;
		}
	}
	else
	{
		fcntl(socket_fd, F_SETFL, 0);
		return socket_fd;
	}

	
	/* in progress */
	FD_ZERO(&rset);
	FD_SET(socket_fd, &rset);
	wset = rset;

	ret = select(socket_fd + 1, &rset, &wset, NULL, &tv);
	if (ret == 0) {
	   /* no readable or writable socket, timeout */
	   close(socket_fd);
	   errno = ETIMEDOUT;
	   printf(LOG_PREFX"Error no readable or writable socket, timeout\n");
	   return -1;
	}
	else if (ret < 0) {
	   /* select error */
	   close(socket_fd);
	   printf(LOG_PREFX"Error select error\n");
	   return -1;
	}

	if (FD_ISSET(socket_fd, &rset) || FD_ISSET(socket_fd, &wset)) {
	   /* check is there any error */
	   len = sizeof(error);
	   getsockopt(socket_fd, SOL_SOCKET, SO_ERROR, &error, &len);
	}

	if (error) {
	   /* connect error */
	   close(socket_fd);
	   errno = error;
	   printf(LOG_PREFX"Error %s\n",strerror(errno));
	   return -1;
	}
	/*
	ioctl(socket_fd, FIONBIO, &imode);
	while(i<timeout)
	{
		FD_ZERO(&myset);
		FD_SET(socket_fd, &myset);
		if(select(socket_fd+1,  0,&myset, 0, &tv) > 0 )
		{
		    if(connect(socket_fd, (struct sockaddr *)&server_addr,sizeof(struct sockaddr)) == -1)
			{
				if(errno==EINPROGRESS)
				{

					FD_ZERO(&myset); 
					FD_SET(socket_fd, &myset);
					if(select(socket_fd+1, NULL, &myset, NULL, &tv) > 0) 
					{ 
						if(FD_ISSET(socket_fd,&myset))
						{
							lon = sizeof(int); 
							getsockopt(socket_fd, SOL_SOCKET, SO_ERROR, (void*)(&valopt), &lon); 
							if(!valopt) 
							{ 
								printf(LOG_PREFX"OK Connection is done\n");
								break;
							}
							else
							printf(LOG_PREFX"3\n");
						}
						else
						printf(LOG_PREFX"2\n");
					}
					else
						printf(LOG_PREFX"1\n");
				}
				else
				{
					printf(LOG_PREFX"connect failed %d\n",i++);
					sleep(1);
				}
		    }
			else
				break;
		}
		else
			printf(LOG_PREFX"4\n");
	}
	imode=0;
	ioctl(socket_fd, FIONBIO, &imode); 
	if(i==timeout)
	{
		close(socket_fd);  
		return -1;
	}*/
	//fcntl(socket_fd, F_SETFL, 0);
    return socket_fd;  
}  
  
static void http_tcpclient_close(int socket){  
    close(socket);  
}  
  
static int http_tcpclient_recv(int socket,char *lpbuff,int timeout){	
  int recvnum = 0;	
  fd_set rset;
  struct timeval tv; 
  tv.tv_usec=0;
  tv.tv_sec = timeout;
  FD_ZERO(&rset); 
  FD_SET(socket, &rset);
  if(select(socket+1, &rset, NULL, NULL, &tv) > 0) 
  {
  	recvnum = recv(socket, lpbuff,BUFFER_SIZE*4,0);  
  }
  else
  {
	printf(LOG_PREFX"recv time out\n");
	return -1;
  }
  return recvnum;  
}

static int http_tcpclient_send(int socket,char *buff,int size,int timeout){  
  int sent=0,tmpres=0;	
  fd_set wset;
  struct timeval tv; 
  tv.tv_usec=0;
  tv.tv_sec = timeout;

	while(sent < size)
	{  
		FD_ZERO(&wset);
		FD_SET(socket, &wset);
		if(select(socket+1,NULL, &wset,  NULL, &tv) > 0) 
		{ 
			tmpres = send(socket,buff+sent,size-sent,0);	
			if(tmpres == -1)
			{  
				return -1;  
			}  
			sent += tmpres; 
		}
		else
		{
			printf(LOG_PREFX"sent time out\n");
			return -1;
		}
	}  
  return sent;	
}  
static int http_parse_url(const char *url,char *host,char *file,int *port)  
{  
    char *ptr1,*ptr2;  
    int len = 0;  
    if(!url || !host || !file || !port){  
        return -1;  
    }  
  
    ptr1 = (char *)url;  
  
    if(!strncmp(ptr1,"http://",strlen("http://"))){  
        ptr1 += strlen("http://");  
    }else{  
        return -1;  
    }  
  
    ptr2 = strchr(ptr1,'/');  
    if(ptr2){  
        len = strlen(ptr1) - strlen(ptr2);  
        memcpy(host,ptr1,len);  
        host[len] = '\0';  
        if(*(ptr2 + 1)){  
            memcpy(file,ptr2 + 1,strlen(ptr2) - 1 );  
            file[strlen(ptr2) - 1] = '\0';  
        }  
    }else{  
        memcpy(host,ptr1,strlen(ptr1));  
        host[strlen(ptr1)] = '\0';  
    }  
    //get host and ip  
    ptr1 = strchr(host,':');  
    if(ptr1){  
        *ptr1++ = '\0';  
        *port = atoi(ptr1);  
    }else{  
        *port = MY_HTTP_DEFAULT_PORT;  
    }  
  
    return 0;  
}  
  
  
  
static char *http_parse_result(const char*lpbuf)  
{  
    char *ptmp = NULL;   
    char *response = NULL;  
    ptmp = (char*)strstr(lpbuf,"HTTP/1.1");  
    if(!ptmp){  
        printf(LOG_PREFX"http/1.1 not faind\n");  
        return NULL;  
    }  
    if(atoi(ptmp + 9)!=200){  
        printf(LOG_PREFX"result:\n%s\n",lpbuf);  
        return NULL;  
    }  
  
    ptmp = (char*)strstr(lpbuf,"\r\n\r\n");  
    if(!ptmp){  
        printf(LOG_PREFX"ptmp is NULL\n");  
        return NULL;  
    }  
    response = (char *)malloc(strlen(ptmp)+1);  
    if(!response){  
        printf(LOG_PREFX"malloc failed \n");  
        return NULL;  
    }  
    strcpy(response,ptmp+4);  
    return response;  
}  
  
char * http_post(const char *url,const char *post_str,int timeout){  
  
    char post[BUFFER_SIZE] = {'\0'};  
    int socket_fd = -1;  
    char lpbuf[BUFFER_SIZE*4] = {'\0'};  
    char *ptmp;  
    char host_addr[BUFFER_SIZE] = {'\0'};  
    char file[BUFFER_SIZE] = {'\0'};  
    int port = 0;  
    int len=0;  
    char *response = NULL;  
  
    if(!url || !post_str){  
        printf(LOG_PREFX"      failed!\n");  
        return NULL;  
    }  
  
    if(http_parse_url(url,host_addr,file,&port)){  
        printf(LOG_PREFX"http_parse_url failed!\n");  
        return NULL;  
    }  
    //printf(LOG_PREFX"host_addr : %s\tfile:%s\t,%d\n",host_addr,file,port);  
  
    socket_fd = http_tcpclient_create(host_addr,port,timeout);  
    if(socket_fd < 0){  
        printf(LOG_PREFX"http_tcpclient_create failed\n");  
        return NULL;  
    }  
       
    sprintf(lpbuf,HTTP_POST,file,host_addr,port,strlen(post_str),post_str);  
  
    if(http_tcpclient_send(socket_fd,lpbuf,strlen(lpbuf),timeout) < 0){  
        printf(LOG_PREFX"http_tcpclient_send failed..\n");  
        return NULL;  
    }  
    printf(LOG_PREFX"POST Sent:\n%s\n",lpbuf);  
  
    /*it's time to recv from server*/  
    if(http_tcpclient_recv(socket_fd,lpbuf,timeout) <= 0){  
        printf(LOG_PREFX"http_tcpclient_recv failed\n");  
        return NULL;  
    }  
  
    http_tcpclient_close(socket_fd);  
  
    return http_parse_result(lpbuf);  
}  
  
char * http_get(const char *url,int timeout)  
{  
  
    char post[BUFFER_SIZE] = {'\0'};  
    int socket_fd = -1;  
    char lpbuf[BUFFER_SIZE*4] = {'\0'};  
    char *ptmp;  
    char host_addr[BUFFER_SIZE] = {'\0'};  
    char file[BUFFER_SIZE] = {'\0'};  
    int port = 0;  
    int len=0;  
  
    if(!url){  
        printf(LOG_PREFX"      failed!\n");  
        return NULL;  
    }  
  
    if(http_parse_url(url,host_addr,file,&port)){  
        printf(LOG_PREFX"http_parse_url failed!\n");  
        return NULL;  
    }  
    printf(LOG_PREFX"host_addr : %s\tfile:%s\t,%d\n",host_addr,file,port);  
  
    socket_fd =  http_tcpclient_create(host_addr,port,timeout);  
    if(socket_fd < 0){  
        printf(LOG_PREFX"http_tcpclient_create failed\n");  
        return NULL;  
    }  
  
    sprintf(lpbuf,HTTP_GET,file,host_addr,port);  
  
    if(http_tcpclient_send(socket_fd,lpbuf,strlen(lpbuf),timeout) < 0){  
        printf(LOG_PREFX"http_tcpclient_send failed..\n");  
        return NULL;  
    }  
  	//printf(LOG_PREFX"GET Sent:\n%s\n",lpbuf);  
  
    if(http_tcpclient_recv(socket_fd,lpbuf,timeout) <= 0){  
        printf(LOG_PREFX"http_tcpclient_recv failed\n");  
        return NULL;  
    }  
    http_tcpclient_close(socket_fd);  
  
    return http_parse_result(lpbuf);  
}  
#endif
char doit_ack(char *text,const char *item_str)
{
	char result=0;cJSON *json,*item_json;
		
		json=cJSON_Parse(text);
		if (!json) {printf(LOG_PREFX"Error before: [%s]\n",cJSON_GetErrorPtr());}
		else
		{
			item_json=cJSON_GetObjectItem(json,item_str);
			if((item_json->type & 255) ==cJSON_True)
			{
				result=1;
			}
			cJSON_Delete(json);
		}
		return result;
}
char *doit(char *text,const char *item_str)
{
	char *out=NULL;cJSON *json,*item_json;
	
	json=cJSON_Parse(text);
	if (!json) {printf(LOG_PREFX"Error before: [%s]\n",cJSON_GetErrorPtr());}
	else
	{
		//out=cJSON_Print(json);
		item_json = cJSON_GetObjectItem(json, "data");
		if (item_json)
		{
			cJSON *data;
			data=cJSON_GetObjectItem(item_json,item_str);
			if(data)
			{
			    int nLen = strlen(data->valuestring);
				out=(char *)malloc(nLen+1);
				memset(out,'\0',nLen+1);
				memcpy(out,data->valuestring,nLen);
			}			
		    //printf(LOG_PREFX"%s ,%d %s\n",item_str,nLen,item_json->valuestring);
		}
		//else
		//	printf(LOG_PREFX"get %s failed\n",item_str);
		cJSON_Delete(json);
		//printf(LOG_PREFX"%s\n",out);
		//free(out);
	}
	return out;
}
char *doit_data(char *text,const char *item_str)
{
	char *out=NULL;cJSON *json,*item_json;
	
	item_json=cJSON_Parse(text);
	if (!item_json) {printf(LOG_PREFX"Error before: [%s]\n",cJSON_GetErrorPtr());}
	else
	{
		if (item_json)
 		{	 		    
			int nLen = strlen(item_json->valuestring);
		    printf("%s ,%d %s\n",item_str,nLen,item_json->valuestring);					
			out=(char *)malloc(nLen+1);
			memset(out,'\0',nLen+1);
			memcpy(out,item_json->valuestring,nLen);
		}
 		else
			printf(LOG_PREFX"get %s failed\n",item_str);
 		cJSON_Delete(json);	
	}
	return out;
}


