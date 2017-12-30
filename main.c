#include <stdint.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>

//structure to hold the split url
typedef struct url {
    char server[128];
    char path[256];
} URL;

//gets the address info related to the url supplied
struct addrinfo* getAddrInfo(const char *host);

//builds an http request header
struct url httprequest(const char *host, int len);

//main method
int main(int argc, char* args[])
{
    //sample url
    char *theurl = "www.example.com";
    struct addrinfo *data;
    
    //resolve the host address
    if (!(data = getAddrInfo(theurl)))
    {
        printf("unable to resolve host address\n");
        return -1;
    }
    
    //the socket file descriptor
    int sfd;
    
    if ((sfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("Unable to create socket\n");
        return -1;
    }
    
    //attempt to connect to the server
    if (connect(sfd, data->ai_addr, data->ai_addrlen))
    {
        printf("Unable to connect\n");
        return -1;
    }
    
    //parse the url
    URL stuff = httprequest(theurl, (int)strlen(theurl));
    
    
    //data buffer
    char buffer[1024];
    
    //build the http request
    int reqlen = sprintf(buffer, "GET /%s HTTP/1.1\r\nHost: %s\n\r\n\r", stuff.path, stuff.server);
    
    printf("===Sent===\n%s\n=========\n\n", buffer);
    
    //send request to the server
    send(sfd, buffer, reqlen, 0);
    
    //buffer to hold the repsonse
    char response[4096];
    
    
    //now for the response
    int s = (int)recv(sfd, response, sizeof(response), 0);
    
    //more error handling
    if (s <= 0)
    {
        return -1;
    }
    
    //get rid of garabage
    response[s] = 0;
    
    printf("%d bytes received\n", s);
    
    printf("%s\n", response);
    
    //close socket
    close(sfd);
    
    //free the pointer
    freeaddrinfo(data);
    return 0;
}

//get the linked-list of address info
struct addrinfo* getAddrInfo(const char *host)
{
    struct addrinfo hints, *results;
    memset(&hints, 0, sizeof(hints));
    //set the hints
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    int stat;
    
    //resolve hostname
    if ((stat = getaddrinfo(host, "80", &hints, &results)) != 0)
    {
        printf("%s\n", gai_strerror(stat));
        return NULL;
    }
    
    //free me
    return results;
}

struct url httprequest(const char *host, int len)
{
    //buffers for the server and path
    char server[128]    = {0};
    char path[256]      = {0};
    
    int i;
    
    //the indexes
    int pathIndex = 0;
    int serverIndex = 0;
    
    int isPath = 0;
    
    
    //loop through and parse
    for (i = 0; i < len; i++)
    {
        if (!isPath)
        {
            server[serverIndex++] = host[i];
        }
        else
        {
            path[pathIndex++] = host[i];
        }
        if (host[i] == '/')
        {
            isPath = 1;
        }
    }
    
    struct url urldata;
    
    server[serverIndex] = 0;
    path[pathIndex] = 0;
    
    //setup the structure
    strcpy(urldata.server, server);
    strcpy(urldata.path, path);

    return urldata;
}
