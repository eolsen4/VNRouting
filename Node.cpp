#include<pthread.h>
#include<semaphore.h>
#include<vector>
#include<arpa/inet.h>
#include<cassert>
#include<cstring>
#include<unistd.h>
#include<sys/uio.h>

using namespace std;

/* packet header struct */
typedef struct header
{
  char src_id;
  char dest_id;
  char pckt_id;
  char ttl;
} header;

/* packet data struct */
typedef struct Data
{
  char data[1000-sizeof(header)];
} Data;

/* ID of this Node */
int node_id;
/* name of host */
String host_name;

/* distance info for routing */
Map<int, int>hostDistances;

/* routeNodes contains mappings for the next node to route to when trying to
 * reach a destination on the network */
Map<int,int>routeNodes;

/* adjPorts and adjAddrs contain mappings from the node ID of adjacent nodes to
 * their respective ports and addresses */
Map<int,int>adjPorts;
Map<int, unsigned long>adjAddrs;

/* char array for holding packet data */
char packet[1000];

sem_t dataMsgReq;

int createSock(void* input)
{
  /* cast input back to sockaddr */
  struct sockaddr_in* sa = static_cast<struct sockaddr_in*>input;
 
  /* create a datagram socket */
  int retVal = socket(AF_INET, SOCK_DGRAM, 0)

  /* if socket doesn't bind something went very wrong */
  assert(bind(retVal, (struct sockaddr*)sa, sizeof(sa)) != -1);

  return retVal;
}

void dataProcess(void* input)
{
  /* create socket */
  int sd = createSock(input);
  sockaddr_in recieved_data, send_data;
  int recieved_len;

  /* structs for parsing packet data */
  Header header;
  Data data;

  /* set up fd_sets*/
  fd_set rfds, store;

  FD_ZERO(&rfds);
  FD_ZERO(&store);

  FD_SET(sd, &store);

  while(1)
  {
    memset(packet, 0, 1000);
  
    rfds = store;

    /* check for incoming meesages */
    select(sd+1, &rfds, NULL, NULL, NULL);

    if(FD_ISSET(sd, &rfds))
    {
      /* recieve packet */
      recieved_len = recvfrom(sd, (void*)packet, 1000, 0, (struct sockaddr*)&recieved_data, sizeof(sockaddr));
      
      /* pull data out of packet for processing */
      memcpy(&header, packet, sizeof(header));
      memcpy(&data, packet+sizeof(header), sizeof(data))

     printf("Packet from: %d\nDestined for:%d\n Arrived at:%d\n TTL:%d\n", header.src_id, header.dest_id, node_id, header.ttl);

     if(header.dest_id == node_id)
     {
      
      /* data should be formatted as a C-string so this should work */
      printf("Forwarding path: %s\n", data.data);
     }
     else
     {
      /* find next node to route to */
      int nextNode = routeNodes.find(header.dest_id);

      /* get the port and address of the next host to send to*/
      send_data.sin_family = AF_INET;
      send_data.sin_port = htons(adjPorts.find(nextNode));
      send_data.sin_addr = adjAddrs.find(nextNode);

      /* edit ttl of packet and add this node to list */
      String newData(data.data);
      newData.append(" "+atoi(node_id));
      /* copy over everything except newline character */
      memcpy(packet+sizeof(header), newData.c_str(), newData.length-1);

      header.ttl--;
      memcpy(packet, header, sizeof(header));

      sendto(sd, (const void*)packet, 1000, 0, (struct sockaddr*)&send_data, sizeof(sockaddr));

     }
    }
  }
}

void controlProcess(void* input)
{
  int sd = createSock(input);

  while(1)
  {

  }
}

int main()
{
  pthread_t dataThread, controlThread;

  /* init semaphore for requesting new data message */
  sem_init(&dataMsgReq, 0, 0);

  /* TODO: input parsing to actually fill the sockaddr structs */
  struct sockaddr_in data_sockaddr, cont_sockaddr;

  /* create threads for data and control message processing */
  pthread_create(&dataThread, NULL, dataProcess, (void*)&data_sockaddr);
  pthread_create(&controlThread, NULL, controlProcess, (void*)&cont_sockaddr);
}
