#include <stdio.h>
#include <json-c/json.h>
#include <string.h>
#include <curl/curl.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<unistd.h>
#include<stdlib.h>
#include<netdb.h>

#define SERVER_PORT 9000
#define BUF_SIZE 2048

void process_client(int clien_fd);
void erro(char *msg);
void front_page(int client_fd, char ID[BUF_SIZE]);
//void login(int client_fd, char ID[BUF_SIZE]);
void my_data(int client_fd, char ID[BUF_SIZE]);
void general_data(int client_fd, char ID[BUF_SIZE]);
void xor_encrytp_decrypt(char *key, char *string, int n);

/*
This example project use the Json-C library to decode the objects to C char arrays and 
use the C libcurl library to request the data to the API.
*/

char key_ID[] = "4A404E635266556A576E5A7234753778214125442A472D4B6150645367566B59";
char key[] = "67566B59703373367638792F423F4528482B4D6251655468576D5A7134743777217A24432646294A404E635266556A586E3272357538782F413F442A472D4B6150645367566B59703373367639792442264529482B4D6251655468576D5A7134743777217A25432A462D4A614E635266556A586E3272357538782F413F442847";
struct string {
	char *ptr;
	size_t len;
};

//Write function to write the payload response in the string structure
size_t writefunc(void *ptr, size_t size, size_t nmemb, struct string *s)
{
	size_t new_len = s->len + size*nmemb;
	s->ptr = realloc(s->ptr, new_len + 1);
	if (s->ptr == NULL) {
		fprintf(stderr, "realloc() failed\n");
		exit(EXIT_FAILURE);
	}
	memcpy(s->ptr + s->len, ptr, size*nmemb);
	s->ptr[new_len] = '\0';
	s->len = new_len;

	return size*nmemb;
}

//Initilize the payload string
void init_string(struct string *s) {
	s->len = 0;
	s->ptr = malloc(s->len + 1);
	if (s->ptr == NULL) {
		fprintf(stderr, "malloc() failed\n");
		exit(EXIT_FAILURE);
	}
	s->ptr[0] = '\0';
}

//Get the Data from the API and return a JSON Object
struct json_object *get_student_data()
{
	struct string s;
	struct json_object *jobj;

	//Intialize the CURL request
	CURL *hnd = curl_easy_init();

	//Initilize the char array (string)
	init_string(&s);

	curl_easy_setopt(hnd, CURLOPT_CUSTOMREQUEST, "GET");
	//To run on department network uncomment this request and comment the other
	//curl_easy_setopt(hnd, CURLOPT_URL, "http://10.3.4.75:9014/v2/entities?options=keyValues&type=student&attrs=activity,calls_duration,calls_made,calls_missed,calls_received,department,location,sms_received,sms_sent&limit=1000");
        //To run from outside
	curl_easy_setopt(hnd, CURLOPT_URL, "http://socialiteorion2.dei.uc.pt:9014/v2/entities?options=keyValues&type=student&limit=1000");

	//Add headers
	struct curl_slist *headers = NULL;
	headers = curl_slist_append(headers, "cache-control: no-cache");
	headers = curl_slist_append(headers, "fiware-servicepath: /");
	headers = curl_slist_append(headers, "fiware-service: socialite");

	//Set some options
	curl_easy_setopt(hnd, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, writefunc); //Give the write function here
	curl_easy_setopt(hnd, CURLOPT_WRITEDATA, &s); //Give the char array address here

	//Perform the request
	CURLcode ret = curl_easy_perform(hnd);
	if (ret != CURLE_OK){
		fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(ret));

		/*jobj will return empty object*/
		jobj = json_tokener_parse(s.ptr);

		/* always cleanup */
		curl_easy_cleanup(hnd);
		return jobj;

	}
	else if (CURLE_OK == ret) {
		jobj = json_tokener_parse(s.ptr);
		free(s.ptr);

		/* always cleanup */
		curl_easy_cleanup(hnd);
		return jobj;
	}

}

int main(void){
	system("clear");
  int fd, client;
  struct sockaddr_in addr, client_addr;
  int client_addr_size;

  //bzero((void *) &addr, sizeof(addr));
  addr.sin_family      = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_port        = htons(SERVER_PORT);

  if ( (fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
	erro("na funcao socket");
  if ( bind(fd,(struct sockaddr*)&addr,sizeof(addr)) < 0)
	erro("na funcao bind");
  if( listen(fd, 5) < 0) 
	erro("na funcao listen");
  
    int nclientes=0;
    
  while (1) {
    client_addr_size = sizeof(client_addr);
    client = accept(fd,(struct sockaddr *)&client_addr,&client_addr_size);
      nclientes++;
    if (client > 0) {
      if (fork() == 0) {
        close(fd);
        system("clear");
        process_client(client);
        exit(0);
      }
    close(client);
    }
  }
	return 0;
}


void process_client(int client_fd){
	int nread = 0;
	int opt = 0;
	char buffer[BUF_SIZE], ID[BUF_SIZE], aux[BUF_SIZE];
	memset(buffer, 0, strlen(buffer));
	memset(ID, 0, BUF_SIZE);
	memset(aux, 0, BUF_SIZE);
	nread = read(client_fd, buffer, BUF_SIZE-1);
	buffer[nread] = '\0';
	xor_encrytp_decrypt(key_ID, buffer, strlen(buffer));
	strcat(ID, buffer);
	memset(buffer, 0, strlen(buffer));

	front_page(client_fd, ID);


}

void erro(char *msg){
	printf("Erro: %s\n", msg);
	exit(-1);
}

void front_page(int client_fd, char ID[BUF_SIZE]){
	int nread = 0;
	int opt = 0;
	char buffer[BUF_SIZE];
	memset(buffer, 0, BUF_SIZE);
	strcat(buffer, "Welcome to ISABELA's stats APP!\nChoose wich stats do you want to see:\n1-My Own Stats\n2-General Stats\n0-Log Out\n-> ");
	xor_encrytp_decrypt(key, buffer, strlen(buffer));
	write(client_fd, buffer, BUF_SIZE-1);
	do{
		nread = read(client_fd, buffer, BUF_SIZE-1);
    	buffer[nread] = '\0';
		xor_encrytp_decrypt(key, buffer, strlen(buffer));
		//printf("\nbuffer -> %s", buffer);
		buffer[strcspn(buffer, "\n")] = 0;
		opt = atoi(buffer);
		//printf("\nopt-> %d", opt);
		if(opt < 0 || opt > 2){
			memset(buffer, 0, strlen(buffer));
			strcat(buffer, "\nWrong option choose again\n\nChoose wich stats do you want to see:\n1-My Own Stats\n2-General Stats\n0-Log Out\n-> ");
			xor_encrytp_decrypt(key, buffer, strlen(buffer));
			write(client_fd, buffer, BUF_SIZE-1);
		}
		if(opt == 1){
			my_data(client_fd, ID);
		}
		if(opt == 2){
			general_data(client_fd, ID);
		}
		if(opt == 0){
			memset(buffer, 0, BUF_SIZE);
			strcat(buffer, "out");
			xor_encrytp_decrypt(key, buffer, strlen(buffer));
			write(client_fd, buffer, BUF_SIZE-1);
			close(client_fd);
		}
	} while (opt != 0 && opt != 1 && opt != 2);
	
}

void my_data(int client_fd, char ID[BUF_SIZE]){
	int nread = 0;
	int opt = 0;
	char buffer[BUF_SIZE], aux[BUF_SIZE];
	memset(buffer, 0, BUF_SIZE);
	memset(aux, 0, BUF_SIZE);
	//JSON obect
	struct json_object *jobj_array, *jobj_obj;
	struct json_object *jobj_object_id, *jobj_object_type, *jobj_object_activity, *jobj_object_location, *jobj_object_latlong, *jobj_object_callsduration, 
	*jobj_object_callsmade, *jobj_object_callsmissed, *jobj_object_callsreceived, *jobj_object_department, *jobj_object_smsreceived, *jobj_object_smssent, *jobj_object_accelerometer,
	*jobj_object_ble, *jobj_object_bluetooth, *jobj_object_gyroscope, *jobj_object_wifi, *jobj_object_timestamp;
	enum json_type type = 0;
	int arraylen = 0;
	int i;

	//Get the student data
	jobj_array = get_student_data();

	//Get array length
	arraylen = json_object_array_length(jobj_array);

	//Example of howto retrieve the data
	for (i = 0; i < arraylen; i++) {
		//get the i-th object in jobj_array
		jobj_obj = json_object_array_get_idx(jobj_array, i);

		//get the name attribute in the i-th object
		jobj_object_id = json_object_object_get(jobj_obj, "id");
		jobj_object_type = json_object_object_get(jobj_obj, "type");
		jobj_object_activity = json_object_object_get(jobj_obj, "activity");
		jobj_object_location = json_object_object_get(jobj_obj, "location");
		jobj_object_callsduration = json_object_object_get(jobj_obj, "calls_duration");
		jobj_object_callsmade = json_object_object_get(jobj_obj, "calls_made");
		jobj_object_callsmissed = json_object_object_get(jobj_obj, "calls_missed");
		jobj_object_callsreceived= json_object_object_get(jobj_obj, "calls_received");
		jobj_object_department = json_object_object_get(jobj_obj, "department");
		jobj_object_smsreceived = json_object_object_get(jobj_obj, "sms_received");
		jobj_object_smssent = json_object_object_get(jobj_obj, "sms_sent");
		jobj_object_accelerometer = json_object_object_get(jobj_obj, "accelerometer");
		jobj_object_ble = json_object_object_get(jobj_obj, "ble");
		jobj_object_bluetooth = json_object_object_get(jobj_obj, "bluetooth");
		jobj_object_gyroscope = json_object_object_get(jobj_obj, "gyroscope");
		jobj_object_wifi = json_object_object_get(jobj_obj, "wifi");
		jobj_object_timestamp = json_object_object_get(jobj_obj, "timestamp");

		//printf("\nVerificar opcao -> %d\n", opt);
		if(strcmp(ID, json_object_get_string(jobj_object_id)) == 0){
			//printf("\nuser found");
			//memset(buffer, 0, strlen(buffer));
			strcat(aux, "Type = ");
			strcat(aux, json_object_get_string(jobj_object_type));
			strcat(aux, "\nActivity = ");
			strcat(aux, json_object_get_string(jobj_object_activity));
			strcat(aux, "\nLocation = ");
			strcat(aux, json_object_get_string(jobj_object_location));
			strcat(aux, "\nCalls duration = ");
			strcat(aux, json_object_get_string(jobj_object_callsduration));
			strcat(aux, "\nCalls made = ");
			strcat(aux, json_object_get_string(jobj_object_callsmade));
			strcat(aux, "\nCalls missed = ");
			strcat(aux, json_object_get_string(jobj_object_callsmissed));		
			strcat(aux, "\nCalls received = ");
			strcat(aux, json_object_get_string(jobj_object_callsreceived));
			strcat(aux, "\nDepartment = ");
			strcat(aux, json_object_get_string(jobj_object_department));
			strcat(aux, "\nSMS received = ");
			strcat(aux, json_object_get_string(jobj_object_smsreceived));
			strcat(aux, "\nSMS sent = ");
			strcat(aux, json_object_get_string(jobj_object_smssent));
			strcat(aux, "\n\nType any number to go back to main menu!\n-> ");
			//printf("\n%s", aux);
			strcat(buffer, aux);
			//printf("\n%s", buffer);
			xor_encrytp_decrypt(key, buffer, strlen(buffer));
			write(client_fd, buffer, BUF_SIZE-1);
			memset(buffer, 0, strlen(buffer));
			nread = read(client_fd, buffer, BUF_SIZE-1);
      		buffer[nread] = '\0';
			xor_encrytp_decrypt(key, buffer, strlen(buffer));
			if(strcmp(buffer, aux) != 0)
				front_page(client_fd, ID);

		}	
		/*else{
			printf("\nuser not found");
		}*/

	}
}


void general_data(int client_fd, char ID[BUF_SIZE]){
	int nread = 0;
	int opt = 0;
	int house = 0;
	int uni = 0;
	int other = 0;
	int callsmiss, callsmade, callsreceive, smssent, smsreceive;
	int exer, sleepp, classes, tilting, walk, veic, still, unkown;
	char buffer[BUF_SIZE], aux[BUF_SIZE], aux2[BUF_SIZE];
	memset(buffer, 0, BUF_SIZE);
	memset(aux, 0, BUF_SIZE);

	//JSON obect
	struct json_object *jobj_array, *jobj_obj;
	struct json_object *jobj_object_id, *jobj_object_type, *jobj_object_activity, *jobj_object_location, *jobj_object_latlong, *jobj_object_callsduration, 
	*jobj_object_callsmade, *jobj_object_callsmissed, *jobj_object_callsreceived, *jobj_object_department, *jobj_object_smsreceived, *jobj_object_smssent, *jobj_object_accelerometer,
	*jobj_object_ble, *jobj_object_bluetooth, *jobj_object_gyroscope, *jobj_object_wifi, *jobj_object_timestamp;
	enum json_type type = 0;
	int arraylen = 0;
	int i;

	//Get the student data
	jobj_array = get_student_data();

	//Get array length
	arraylen = json_object_array_length(jobj_array);

	//Example of howto retrieve the data
	strcat(buffer, "There are ->");
	sprintf(aux, "%d", arraylen);
	strcat(buffer, aux);
	strcat(buffer, "<- people in the ISABELA's server\n");


	memset(aux, 0, BUF_SIZE);
	strcat(buffer, "\nGeneral stats Menu\n");
	//write(client_fd, buffer, BUF_SIZE-1);
	callsmiss = 0; callsmade = 0; callsreceive = 0; smssent = 0; smsreceive = 0;
	exer = 0; sleepp = 0; classes = 0; tilting = 0; walk = 0; veic = 0; still = 0; unkown = 0;
	//printf("\n%s", buffer);
	memset(aux2, 0, strlen(aux2));
	for(i = 0; i < arraylen; i++){
		jobj_obj = json_object_array_get_idx(jobj_array, i);
		jobj_object_location = json_object_object_get(jobj_obj, "location");
		jobj_object_type = json_object_object_get(jobj_obj, "type");
		jobj_object_activity = json_object_object_get(jobj_obj, "activity");
		jobj_object_callsmade = json_object_object_get(jobj_obj, "calls_made");
		jobj_object_callsmissed = json_object_object_get(jobj_obj, "calls_missed");
		jobj_object_callsreceived = json_object_object_get(jobj_obj, "calls_received");
		jobj_object_smsreceived = json_object_object_get(jobj_obj, "sms_received");
		jobj_object_smssent = json_object_object_get(jobj_obj, "sms_sent");
		strcat(aux2, json_object_get_string(jobj_object_location));
		if(strcmp(aux2, "University")== 0){
			uni++;
		}
		if(strcmp(aux2, "House") == 0){
			house++;
		}
		if(strcmp(aux2, "Other") == 0){
			other++;
		}
		memset(aux2, 0, strlen(aux2));
		strcat(aux2, json_object_get_string(jobj_object_activity));
		if(strcmp(aux2, "Exercise") == 0){
			exer++;
		}
		if(strcmp(aux2, "Sleeping") == 0){
			sleepp++;
		}
		if(strcmp(aux2, "Classes") == 0){
			classes++;
		}
		if(strcmp(aux2, "Tilting") == 0){
			tilting++;
		}
		if(strcmp(aux2, "Walking") == 0){
			walk++;
		}
		if(strcmp(aux2, "In vehicle") == 0){
			veic++;
		}
		if(strcmp(aux2, "Unknown") == 0){
			unkown++;
		}
		if(strcmp(aux2, "Stil") == 0){
			still++;
		}
		memset(aux2, 0, strlen(aux2));
		strcat(aux2, json_object_get_string(jobj_object_callsmade));
		callsmade += atoi(aux2);
		strcat(aux2, json_object_get_string(jobj_object_callsmissed));	
		callsmiss += atoi(aux2);
		memset(aux2, 0, strlen(aux2));	
		strcat(aux2, json_object_get_string(jobj_object_callsreceived));
		callsreceive += atoi(aux2);
		memset(aux2, 0, strlen(aux2));
		strcat(aux2, json_object_get_string(jobj_object_smsreceived));
		smsreceive += atoi(aux);
		memset(aux2, 0, strlen(aux2));
		strcat(aux2, json_object_get_string(jobj_object_smssent));
		smssent += atoi(aux2);
		memset(aux2, 0, strlen(aux2));

	}
	//memset(buffer, 0, BUF_SIZE);
	sprintf(aux, "%d", uni);
	strcat(buffer, "\nLocation Stats\nThere are ->");
	strcat(buffer, aux);
	strcat(buffer, "<- people at the University.\nThere are ->");
	memset(aux, 0, BUF_SIZE);
	sprintf(aux, "%d", house);
	strcat(buffer, aux);
	strcat(buffer, "<- people at home.\nThere ->");
	memset(aux, 0, BUF_SIZE);
	sprintf(aux, "%d", other);
	strcat(buffer, aux);
	strcat(buffer, "<- people in other locations.\n");
	strcat(buffer, "\nActivity Stats\nThere ->");
	sprintf(aux, "%d", exer);
	strcat(buffer, aux);
	strcat(buffer, "<- people doing exercise.\nThere are ->");
	memset(aux, 0, strlen(aux));
	sprintf(aux, "%d", sleepp);
	strcat(buffer, aux);
	strcat(buffer, "<- people sleeping.\nThere are ->");
	memset(aux, 0, strlen(aux));
	sprintf(aux, "%d", classes);
	strcat(buffer, aux);
	strcat(buffer, "<- people in classes.\nThere are ->");
	memset(aux, 0, strlen(aux));
	sprintf(aux, "%d", tilting);
	strcat(buffer, aux);
	strcat(buffer, "<- people tilting.\nThere are ->");
	memset(aux, 0, strlen(aux));
	sprintf(aux, "%d", walk);
	strcat(buffer, aux);
	strcat(buffer, "<- people walking.\nThere are ->");
	memset(aux, 0, strlen(aux));
	sprintf(aux, "%d", veic);
	strcat(buffer, aux);
	strcat(buffer, "<- people in a vehicle.\nThere are ->");
	memset(aux, 0, strlen(aux));
	sprintf(aux, "%d", still);
	strcat(buffer, aux);
	strcat(buffer, "<- people stilling.\nThe ativity of ->");
	memset(aux, 0, strlen(aux));
	sprintf(aux, "%d", unkown);
	strcat(buffer, aux);
	strcat(buffer, "<- people is unknown\n");
	memset(aux, 0, strlen(aux));
	strcat(buffer, "\nCalls Stats\nThe users made a total of ->");
	sprintf(aux, "%d", callsmade);
	strcat(buffer, aux);
	strcat(buffer, "<- calls.\nThe users received a total of ->");
	memset(aux, 0, strlen(aux));
	sprintf(aux, "%d", callsreceive);
	strcat(buffer, aux);
	strcat(buffer, "<- calls, wich ->");
	memset(aux, 0, strlen(aux));
	sprintf(aux, "%d", callsmiss);
	strcat(buffer, aux);
	strcat(buffer, "<- were missed\n\nSMS Stats\nThe users sent a total of ->");
	memset(aux, 0, strlen(aux));
	sprintf(aux, "%d", smssent);
	strcat(buffer, aux);
	strcat(buffer, "<- SMSs and received a total of ->");
	memset(aux, 0, strlen(aux));
	sprintf(aux, "%d", smsreceive);
	strcat(buffer, aux);
	strcat(buffer, "<- SMSs\n\nType any number to go back -> ");
	memset(aux, 0, strlen(aux));

	xor_encrytp_decrypt(key, buffer, strlen(buffer));
	write(client_fd, buffer, BUF_SIZE-1);
	nread = read(client_fd, buffer, BUF_SIZE);
	buffer[nread] = '\0';
	xor_encrytp_decrypt(key, buffer, strlen(buffer));
	if(strcmp(buffer, aux) != 0){
		front_page(client_fd, ID);
	}
}

void xor_encrytp_decrypt(char *key, char *string, int n){
  int i;
    int keylen = strlen(key);

    for(i = 0; i < n; i++){
        string[i] = string[i]^key[i%keylen];
    }
}