/*************************************************************
 * CLIENTE liga ao servidor (definido em argv[1]) no porto especificado  
 * (em argv[2]), escrevendo a palavra predefinida (em argv[3]).
 * USO: >cliente <enderecoServidor>  <porto>  <Palavra>
 *************************************************************/

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>

#define BUF_SIZE 2048

//definição do ID e das chaves
char id[] = "fc769b802ca3d871408d264a5da3f433";
char key[] = "67566B59703373367638792F423F4528482B4D6251655468576D5A7134743777217A24432646294A404E635266556A586E3272357538782F413F442A472D4B6150645367566B59703373367639792442264529482B4D6251655468576D5A7134743777217A25432A462D4A614E635266556A586E3272357538782F413F442847";
char key_ID[] = "4A404E635266556A576E5A7234753778214125442A472D4B6150645367566B59";

void erro(char *msg);
void process_server(int fd, char id[], char *key);
void xor_encrytp_decrypt(char *key, char *string, int n);



int main(int argc, char *argv[]) {
  char endServer[100], buffer[BUF_SIZE];
  int fd, client, nread;
  struct sockaddr_in addr, client_addr;
  int client_addr_size;
  struct hostent *hostPtr;
  

  if (argc != 3) {
    printf("cliente <host> <port>\n");
    exit(-1);
  }

  strcpy(endServer, argv[1]);
  if ((hostPtr = gethostbyname(endServer)) == 0) {
    printf("Couldn t get host address.\n");
    exit(-1);
  }

  bzero((void *) &addr, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = ((struct in_addr *)(hostPtr->h_addr))->s_addr;
  addr.sin_port = htons((short) atoi(argv[2]));

  if((fd = socket(AF_INET,SOCK_STREAM,0)) == -1)
	erro("socket");
  if( connect(fd,(struct sockaddr *)&addr,sizeof (addr)) < 0)
	erro("Connect");

  process_server(fd, id, key);
  
  close(fd);
}

void process_server(int fd, char id[], char *key){
    char buffer[BUF_SIZE];
    int nread;
    int opt = 0;
    

    //printf("%s", id);
    system("clear");
    memset(buffer, 0, strlen(buffer));
    strcat(buffer, id);
    //encripta ID
    xor_encrytp_decrypt(key_ID, buffer, strlen(buffer)); 
    //envia o ID para o servidor
    write(fd, buffer, BUF_SIZE-1);  
    memset(buffer, 0, strlen(buffer));
    //fica à espera de mensagens do servidor
    while (1){
      system("clear");
      memset(buffer, 0, BUF_SIZE);
      nread = read(fd, buffer, BUF_SIZE-1);
      buffer[nread] = '\0';
      //desencripta mensagem
      xor_encrytp_decrypt(key, buffer, strlen(buffer));
      //erifica se é a flag de Log Out
      if(strcmp(buffer, "out") == 0){
        //terminar ligação
        close(fd);
        break;
      }
      printf("%s", buffer);
      memset(buffer, 0, BUF_SIZE);
      scanf("%d", &opt);
      sprintf(buffer, "%d", opt);
      //encripta resposta para o servidor
      xor_encrytp_decrypt(key, buffer, strlen(buffer));
      //manda resposta para o servidor
      write(fd, buffer, BUF_SIZE-1);
      
    }
    
}

void erro(char *msg)
{
	printf("Erro: %s\n", msg);
	exit(-1);
}

//função XOR que encripta e desencripta a informação
void xor_encrytp_decrypt(char *key, char *string, int n){
  int i;
    int keylen = strlen(key);

    for(i = 0; i < n; i++){
        string[i] = string[i]^key[i%keylen];
    }
}