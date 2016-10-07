/* fpont 12/99   */
/* pont.net      */
/* mcastClient.c */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h> /* close */

#define SERVER_PORT 1500
#define MAX_MSG 100

int datalen;
char databuf[1024];

int main(int argc, char *argv[]) {

  int sd, rc, i;
  unsigned char ttl = 1;
  struct sockaddr_in cliAddr, servAddr;
  struct hostent *h;

  if(argc<3) {
    printf("usage %s <mgroup> <data1> <data2> ... <dataN>\n",argv[0]);
    exit(1);
  }

  h = gethostbyname(argv[1]);
  if(h==NULL) {
    printf("%s : unknown host '%s'\n",argv[0],argv[2]);
    exit(1);
  }

  servAddr.sin_family = h->h_addrtype;
  memcpy((char *) &servAddr.sin_addr.s_addr, h->h_addr_list[0],h->h_length);
  servAddr.sin_port = htons(SERVER_PORT);
  
  /* check if dest address is multicast */
  if(!IN_MULTICAST(ntohl(servAddr.sin_addr.s_addr))) {
    printf("%s : given address '%s' is not multicast \n",argv[0],
	   inet_ntoa(servAddr.sin_addr));
    exit(1);
  }

  /* create socket */
  sd = socket(AF_INET,SOCK_DGRAM,0);
  if (sd<0) {
    printf("%s : cannot open socket\n",argv[0]);
    exit(1);
  }
  
  /* bind any port number */
  cliAddr.sin_family = AF_INET;
  cliAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  cliAddr.sin_port = htons(0);
  if(bind(sd,(struct sockaddr *) &cliAddr,sizeof(cliAddr))<0) {
    perror("bind");
    exit(1);
  }

  if(setsockopt(sd,IPPROTO_IP,IP_MULTICAST_TTL, &ttl,sizeof(ttl))<0) {
    printf("%s : cannot set ttl = %d \n",argv[0],ttl);
    exit(1);
  }

  printf("%s : sending data on multicast group '%s' (%s)\n",argv[0],
	 h->h_name,inet_ntoa(*(struct in_addr *) h->h_addr_list[0]));
  

  /* send data */
  for(i=2;i<argc;i++) {
    rc = sendto(sd,argv[i],strlen(argv[i])+1,0,
		(struct sockaddr *) &servAddr, sizeof(servAddr));
    
    if (rc<0) {
      printf("%s : cannot send data %d\n",argv[0],i-1);
      close(sd);
      exit(1);
    }
  }/* end for */

  /* close socket and exit */
  close(sd);
  exit(0);
}
