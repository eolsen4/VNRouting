#include<pthread.h>
#include<semaphore.h>
#include<vector>
#include<arpa/inet.h>
#include<cassert>
#include<cstring>
#include<unistd.h>
#include<sys/uio.h>

using namespace std;

#define PACKET_SIZE_BYTES 1000

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
  char data[PACKET_SIZE_BYTES-sizeof(header)];
} Data;

/* control data struct */
typedef struct ControlData
{
  char node_ids[(PACKET_SIZE_BYTES)/2];
  char weights[(PACKET_SIZE_BYTES)/2];

} ControlData;

/* ID of this Node */
int node_id;
/* name of host */
String host_name;

/* distance info for routing */
Map<int, int>NodeDistances;

/* routeNodes contains mappings for the next node to route to when trying to
 * reach a destination on the network */
Map<int,int>routeNodes;

/* adjPorts and adjAddrs contain mappings from the node ID of adjacent nodes to
 * their respective ports and addresses */
Map<int,int>adjPorts;
Map<int, unsigned long>adjAddrs;

/* char array for holding data_packet data */
char data_packet[PACKET_SIZE_BYTES];
char control_packet[PACKET_SIZE_BYTES];

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

  /* structs for parsing data_packet data */
  Header header;
  Data data;

  /* set up fd_sets*/
  fd_set rfds, store;

  FD_ZERO(&rfds);
  FD_ZERO(&store);

  FD_SET(sd, &store);

  while(1)
  {
    memset(data_packet, 0, PACKET_SIZE_BYTES);
  
    rfds = store;

    /* check for incoming meesages */
    select(sd+1, &rfds, NULL, NULL, NULL);

    if(FD_ISSET(sd, &rfds))
    {
      /* recieve data_packet */
      recieved_len = recvfrom(sd, (void*)data_packet, PACKET_SIZE_BYTES, 0, (struct sockaddr*)&recieved_data, sizeof(sockaddr));
      
      /* pull data out of data_packet for processing */
      memcpy(&header, data_packet, sizeof(header));
      memcpy(&data, data_packet+sizeof(header), sizeof(data))

     printf("data_packet from: %d\nDestined for:%d\n Arrived at:%d\n TTL:%d\n", header.src_id, header.dest_id, node_id, header.ttl);

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

      /* edit ttl of data_packet and add this node to list */
      String newData(data.data);
      newData.append(" "+atoi(node_id));
      /* copy over everything except newline character */
      memcpy(data_packet+sizeof(header), newData.c_str(), newData.length-1);

      header.ttl--;
      memcpy(data_packet, header, sizeof(header));

      sendto(sd, (const void*)data_packet, PACKET_SIZE_BYTES, 0, (struct sockaddr*)&send_data, sizeof(sockaddr));

     }
    }
  }
}

void controlProcess(void* input)
{
  int sd = createSock(input);
  int recieved_len, itr, rec_id, itr;
  struct sockaddr_in recieved_data;

  ControlData data;

  /* set up fd_sets*/
  fd_set rfds, store;

  FD_ZERO(&rfds);
  FD_ZERO(&store);

  FD_SET(sd, &store);
  
  while(1)
  {
    memset(control_packet, 0, PACKET_SIZE_BYTES);
    
    rfds = store;

    /* check for incoming meesages */
    select(sd+1, &rfds, NULL, NULL, NULL);

    if(FD_ISSET(sd, &rfds))
    {
      /* recieve data_packet */
      recieved_len = recvfrom(sd, (void*)control_packet, PACKET_SIZE_BYTES, 0, (struct sockaddr*)&recieved_data, sizeof(sockaddr));
     
      /* need to figure out the ID of the sending node */
      Iterator<int,unsigned int> iter = adjAddrs.begin();
      
      for(; iter != adjAddrs.end(); ++iter)
      {
        if(iter->second = recieved_data.sin_addr.s_addr)
        {
          rec_id = iter->first;
          break;
        }
      }
      memcpy(data, control_packet, PACKET_SIZE_BYTES);

      /* determine if the new control packet offers us any shorter paths */
      itr = 0;
      while(data.node_ids[itr] != -1)
      {
       int temp_id = data.node_ids[itr];

       /* if we don't already have a path to the node, or the path through the
        * old intermediary was overwritten, or this new path is shorter,
        * update */
       if(routeNodes.find(temp_id) == routeNodes.end())
       {
        routeNodes.insert(temp_id, rec_id);
       }
       else if(rec_id = routeNodes.find(temp_id)->second || 
          (data.weights[itr]+1) < nodeDistances.find(temp_id)->second)
       {
          nodeDistances.find(temp_id)->second = data.weights[itr]+1;
       }
       itr++;
      }
    }
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
