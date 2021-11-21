#include <stdio.h> // maloc..
#include <stdarg.h>
#include <stddef.h>
#include <stdbool.h> // boolean C
#include <stdlib.h>
#include <string.h> // string C
#include <time.h>
#include <math.h>
#include <enet/enet.h> // Lib ENET / UDP
#include <curl/curl.h> // Lib CURL / Request HTTP.
#include <iostream>
#include <vector> // tableau dynamique - C++
#include <ctime>
using namespace std;


struct memory {
   char *response;
   size_t size;
 };

 struct memory resultResponse;
 
 static size_t cb(void *data, size_t size, size_t nmemb, void *userp)
 {
   size_t realsize = size * nmemb;
   struct memory *mem = (struct memory *)userp;
 
   char *ptr = (char*)realloc(mem->response, mem->size + realsize + 1);

   if(ptr == NULL)
   {
	   return 0;  /* out of memory! */
   }

   mem->response = ptr;
   memcpy(&(mem->response[mem->size]), data, realsize);
   mem->size += realsize;
   mem->response[mem->size] = 0;
 
   return realsize;
}

void requestHTTP_POST()
{
	curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
	curl_easy_setopt(curl, CURLOPT_URL, "https://crz-gamestudio.com/jeu/keyAccess.php?keyAccessUser=grjidfnjikhf33203dsfhjWSSJEZN");
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(curl, CURLOPT_DEFAULT_PROTOCOL, "https");
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, FALSE);
	struct curl_slist *headers = NULL;
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	curl_mime *mime;
	curl_mimepart *part;
	mime = curl_mime_init(curl);
	part = curl_mime_addpart(mime);
	curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);

	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, cb); // Ecrit les données
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&resultResponse); // Callback qui récupère les données.
	res = curl_easy_perform(curl); // Code HTTP (0 si OK)

	if(res != CURLE_OK)
	{
		fprintf(stderr, "curl_easy_perform () a échoué : %s\n", curl_easy_strerror (res));
	}
	else
	{
		printf("Request HTTP OK \n");
		printf("Response back-end : %s \n", resultResponse); // Response back-end.
	}
			
	curl_mime_free(mime);
	curl_easy_cleanup(curl);
}


// Initialisation variables, pointeurs.. de la librairie : curl 
CURL *curl;
CURLcode res;


// Initialisation variables, pointeurs.. + données du Server de la librairie : ENET 
ENetAddress address;
ENetHost *server;
ENetEvent eventENET;

ENetPacket *packet;
ENetPeer *peer;

int clientsConnected = 0;


// Functions for GameLoop
class Client 
{     
  public:
	string currentServer; // player connected for server (FR, EN..)
    string pseudo; // pseudo player for inscription in BDD.
	ENetPeer *peer; // ref client for peer ENET.
	int HP;
	int PE;
	int level;
	int badgeAnneeJeu;
	int badgePVP;
	int badgePVE;
	int currentMap;
	int positionX;
	int positionY;
	string designBoat;
	vector<double> cannon;
	vector<double> cannonball;

	Client(string currentServer2, string pseudo2, ENetPeer *peer2, int HP2, int PE2, int level2, int badgeAnneeJeu2, int badgePVP2, int badgePVE2, int currentMap2, int positionX2, int positionY2, string designBoat2)
	{
		currentServer = currentServer2;
		pseudo = pseudo2;
		peer = peer2;
		HP = HP2;
		PE = PE2;
		level = level2;
		badgeAnneeJeu = badgeAnneeJeu2;
		badgePVP = badgePVP2;
		badgePVE = badgePVE2;
		currentMap = currentMap2;
		positionX = positionX2;
		positionY = positionY2;
		designBoat = designBoat2;

		printf("Object Client creer. \n");
	}

	void test() 
	{

	}
};


void load()
{
	// Color console
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	// DataTime
	time_t now = time(0); 
    tm *ltm = localtime(&now);

	// Print infos for Server
    SetConsoleTextAttribute(hConsole, 12);
    printf("GAME SERVER START : ");

	SetConsoleTextAttribute(hConsole, 15);
	cout << ltm->tm_mday << "/" << 1 + ltm->tm_mon << "/" << 1900 + ltm->tm_year << " - " << ltm->tm_hour << ":" << ltm->tm_min << ":" << ltm->tm_sec ;	printf("\n");
	
	SetConsoleTextAttribute(hConsole, 12);
	printf("ADRESSE IP/PORT SERVER : ");
	SetConsoleTextAttribute(hConsole, 15);
	printf("192.168.1.74:7777 \n");

	SetConsoleTextAttribute(hConsole, 12);
	printf("CLIENTS CONNECTED : ");
	SetConsoleTextAttribute(hConsole, 15);
	printf("%u/200 \n\n\n\n", clientsConnected);

	SetConsoleTextAttribute(hConsole, 32);
	printf("LOG                                                                                                                     \n");	

	SetConsoleTextAttribute(hConsole, 10);
}









// Main Program
int main(int argc, char *argv[])
{
	// Initialisation de la librairie : ENET (UDP)
    if (enet_initialize () != 0)
    {
		printf("ENET non initialiser !");
        return -1;
    }


	// Create server ENET
	address.host = ENET_HOST_ANY;
	address.port = 7777;
	server = enet_host_create(&address, 32, 2, 0, 0);

	if (server == NULL)
	{
		printf("ERREUR CREATE SERVER ENET \n");
		return -1;
	}


	// Initialisation de la librairie : curl (request HTTP - GET / POST)
	curl_global_init(CURL_GLOBAL_ALL);
	curl = curl_easy_init();

	if (curl == NULL)
    {
		printf("CURL non initialiser ! \n");
        return -1;
    }


	// Run Server
	load();

	while (true)
	{
		enet_host_service(server, &eventENET, 0);
		switch (eventENET.type)
		{
			case ENET_EVENT_TYPE_CONNECT:
				printf("[SERVER-RECEIVED-%x:%u][CLIENT-CONNECTED] : New client connected. \n", eventENET.peer->address.host, eventENET.peer->address.port);
			
				// CONNECTION DU CLIENT - INFORMATIONS CLIENT / SERVER
				// Faire une requete HTTP/GET pour récupèrer les data sauvegarder en BDD de sont compte (boulets, currentMap..etc) en comparant la valeur de : eventENET.peer à la colonne peer.
				// Puis créer un objet player avec les data de la BDD coté Server.
				// Lui envoyer un packet du server vers le client avec l'objet Player qui à était créer (contenant c'est données du jeu).
				// Puis stocker l'objet player coté client également quand ont le recois (comme dans le server).
				//int lengthPacket = NULL;
				packet = enet_packet_create("test", strlen("test") + 1, ENET_PACKET_FLAG_RELIABLE);			
				enet_peer_send(eventENET.peer, 0, packet);
				printf("[SERVER-SEND-%x:%u][PACKET:%u] : Envoie d'un packet au client. \n",eventENET.peer->address.host, eventENET.peer->address.port, strlen("test"));

				packet = enet_packet_create("test2", strlen("test2") + 1, ENET_PACKET_FLAG_RELIABLE);			
				enet_host_broadcast(server, 0, packet);
				printf("[SERVER-BROADCAST][PACKET:%u] : Envoie d'un packet a tout les clients. \n", strlen("test2"));

				// Mise a jour des données du serveur concernant les clients connectees
				clientsConnected += 1;

				// Stockez ici toutes les informations client pertinentes.
				eventENET.peer->data = eventENET.peer;
				break;

			case ENET_EVENT_TYPE_RECEIVE:
				printf("[SERVER-RECEIVED][PACKET:%u] : Un paquet contenant %s a etais recue de %s sur le canal %u \n",
				eventENET.packet->dataLength,
				eventENET.packet->data,
				eventENET.peer->data,
				eventENET.channelID);

				// Nettoyez le paquet maintenant que nous avons fini de l'utiliser.
				enet_packet_destroy(eventENET.packet);
				break;

			case ENET_EVENT_TYPE_DISCONNECT:
				clientsConnected -= 1;

				printf("[SERVER-RECEIVED-%x:%u][CLIENT-DISCONNECTED] : Le client c'est deconnecter. \n", eventENET.peer->address.host, eventENET.peer->address.port);

				//printf("peer : %u deconnecter. \n", eventENET.peer->data);

				// Faire une requête PUT pour mettre à jour les données de la BDD (boulets, position X..etc) (pour la prochaine connexion)
				// Supprimer l'objet Client le concernant dans le server ici-même.
				// Réinitialiser les informations client de l'homologue.
				eventENET.peer->data = NULL;
		}
	}
	

	// Destroy instance for memory (Free memory)
	enet_host_destroy(server);
	enet_deinitialize();
	curl_global_cleanup();

	return 0;
}