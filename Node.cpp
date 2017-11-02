#include <iostream>
#include <pthread.h>
#include <vector>
#include <arpa/inet.h>
#include <cassert>
#include <cstring>
#include <unistd.h>
#include <assert.h>
#include <sys/uio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <map>

#include "Common.hpp"

using namespace std;

/* TODO Create control program to drive nodes */

using namespace std;

/* packet header struct */
typedef struct Header
{
  char src_id;
  char dest_id;
  char pckt_id;
  char ttl;
} Header;
 
/* packet data struct */
typedef struct Data
{
  char data[PACKET_SIZE_BYTES-sizeof(Header)];
} Data;

/* lock for critical sections of code */
pthread_mutex_t dataLock;

/* ID of this Node */
int node_id;
/* name of host */
string host_name;

/* distance info for routing */
map<int, int>nodeDistances;

/* routeNodes contains mappings for the next node to route to when trying to
 * reach a destination on the network */
map<int,int>routeNodes;

/* adjPorts and adjAddrs contain mappings from the node ID of adjacent nodes to
 * their respective ports and addresses */
map<int,int>adjPorts;
map<int, in_addr_t>adjAddrs;

/* flag for when a data message has been requested to send */
bool sendMessage = false;
int sendToNode;

/* current packet id to send out */
char pckt_id = 0;

int createSock(void* input)
{
  /* cast input back to sockaddr */
  struct sockaddr_in* sa = static_cast<struct sockaddr_in*>(input);

  /* create a datagram socket */
  int retVal = socket(AF_INET, SOCK_DGRAM, 0);

  /* if socket doesn't bind something went very wrong */
  if(bind(retVal, (struct sockaddr*)sa, sizeof(sa)) == -1){
    perror("node: bind function failed");
    return -1;
  }

  return retVal;
}

/*TODO Implement locking for structures accessed on both threads. */
static void* dataProcess(void* input)
{
  /* create socket */
  int sd = createSock(input);
  sockaddr_in recieved_data, send_data;
  int recieved_len;
  
  /* structs for parsing data_packet data */
  char data_packet[PACKET_SIZE_BYTES];
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

    socklen_t size = sizeof(sockaddr); 

    /* check for incoming meesages */
    select(sd+1, &rfds, NULL, NULL, NULL);

    if(FD_ISSET(sd, &rfds))
    {
      /* recieve data_packet */
      recieved_len = recvfrom(sd, (void*)data_packet, PACKET_SIZE_BYTES, 0, (struct sockaddr*)&recieved_data, &size);

      /* pull data out of data_packet for processing */
      memcpy(&header, data_packet, sizeof(header));
      memcpy(&data, data_packet+sizeof(header), sizeof(data));

      printf("data_packet from: %d\nDestined for:%d\n Arrived at:%d\n TTL:%d\n", header.src_id, header.dest_id, node_id, header.ttl);

      if(header.dest_id == node_id)
      {

        /* print path data */
        printf("Forwarding path: ");
        for(int itr = 0; data.data[itr] != -1; itr++)
          printf("%d ", data.data[itr]);
        printf("\n");
      }
      else
      {

        pthread_mutex_lock(&dataLock);
        /* find next node to route to */
        int nextNode = routeNodes.at(header.dest_id);

        printf("Packet sent from: %d Destined for: %d Arrived at: %d Sending next to %d", header.src_id, 
                                                                                          header.dest_id, 
                                                                                          node_id,
                                                                                          nextNode);


        /* get the port and address of the next host to send to*/
        send_data.sin_family = AF_INET;
        send_data.sin_port = htons(adjPorts.at(nextNode));
        send_data.sin_addr.s_addr = adjAddrs.at(nextNode);

        /* edit ttl of data_packet and add this node to list */
        int itr = 0;
        while(data.data[itr] != -1)
          itr++;
        
        data.data[itr] = node_id;
        data.data[itr+1] = -1;

        /* copy over everything except newline character */
        memcpy(data_packet+sizeof(header), data.data, sizeof(data));

        header.ttl--;
        memcpy(data_packet, &header, sizeof(header));

        if(header.ttl > 0)
          sendto(sd, (const void*)data_packet, PACKET_SIZE_BYTES, 0, (struct sockaddr*)&send_data, sizeof(sockaddr));

        pthread_mutex_unlock(&dataLock);
      }

      pthread_mutex_lock(&dataLock);
      
      if(sendMessage)
      {
        memset(data_packet, 0, PACKET_SIZE_BYTES);
        header.src_id = node_id;
        header.dest_id = sendToNode;
        header.pckt_id = pckt_id;
        pckt_id++;
        header.ttl = 15;
        
        /* clear data and add the current node id to the routing list */
        memset(&data, 0, sizeof(data));
        data.data[0] = node_id;
        data.data[1] = -1;

        memcpy(data_packet, &header, sizeof(header));
        memcpy(data_packet+sizeof(header), &data, sizeof(data));

        /* need to set up sockaddr struct to sent out packet */
        /* get the port and address of the next host to send to*/
        send_data.sin_family = AF_INET;
        send_data.sin_port = htons(adjPorts.at(sendToNode));
        send_data.sin_addr.s_addr = adjAddrs.at(sendToNode);
        
        sendto(sd, (const void*)data_packet, PACKET_SIZE_BYTES, 0, (struct sockaddr*)&send_data, sizeof(sockaddr));
      }
      
      pthread_mutex_unlock(&dataLock);
    }
  }
}

static void* controlProcess(void* input)
{
  int sd = createSock(input);
  int recieved_len, itr, rec_id;
  struct sockaddr_in recieved_data;

  char control_packet[PACKET_SIZE_BYTES];

  ControlHeader header;
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

    socklen_t size = sizeof(sockaddr);

    /* check for incoming meesages */
    select(sd+1, &rfds, NULL, NULL, NULL);

    if(FD_ISSET(sd, &rfds))
    {
      /* recieve data_packet */
      recieved_len = recvfrom(sd, (void*)control_packet, PACKET_SIZE_BYTES, 0, (struct sockaddr*)&recieved_data, &size);

      /* parse out packet data into header and data structs */
      memcpy(&header, control_packet, sizeof(header));
      memcpy(&data, control_packet+sizeof(header), PACKET_SIZE_BYTES-sizeof(header));

      /* if we recieved a routing vector */
      if(ROUTING_VECTOR == header.pkt_type)
      {
        pthread_mutex_lock(&dataLock);
        
        /* need to figure out the ID of the sending node */
        map<int, in_addr_t>::iterator iter = adjAddrs.begin();

        for(; iter != adjAddrs.end(); ++iter)
        {
          if(iter->second == recieved_data.sin_addr.s_addr)
          //if(iter->second == inet_ntoa(recieved_data.sin_addr)
          {
            rec_id = iter->first;
            break;
          }
        }

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
            routeNodes.insert(pair<int,int>(temp_id, rec_id));
          }
          else if(rec_id == routeNodes.find(temp_id)->second || 
              (data.weights[itr]+1) < nodeDistances.find(temp_id)->second)
          {
            nodeDistances.find(temp_id)->second = data.weights[itr]+1;
          }
          itr++;
        }

        pthread_mutex_unlock(&dataLock);
      }
      else
      {
      
        if(CTRL_MSG_SEND_PCKT == header.pkt_type)
        {
          pthread_mutex_lock(&dataLock);
          
          sendMessage = true;
          sendToNode = data.node_ids[0];

          pthread_mutex_unlock(&dataLock);
        }
        if(CTRL_MSG_CREATE_LINK == header.pkt_type)
        {
          /* TODO: write functionality for creating a new link. Blocked by need
           * for input parsing function */
        }
      }
    }
  }
}

int main(int argc, char **argv)
{
  if(argc != 3) {
    cout << "USAGE: " << argv[0] << " <input file path> <node number>" << endl;
    return -1;
  }  

  pthread_t dataThread, controlThread;

  /* init semaphore for requesting new data message */
  pthread_mutex_init(&dataLock, NULL);

  /* TODO: input parsing to actually fill the sockaddr structs */
  struct sockaddr_in data_sockaddr, cont_sockaddr;

  /*data_sockaddr.sin_family = AF_INET;
  data_sockaddr.sin_port = htons(data_port));
  data_sockaddr.sin_addr.s_addr = data_addr;

  cont_sockaddr.sin_family = AF_INET;
  cont_sockaddr.sin_port = htons(cont_port);
  cont_sockaddr.sin_addr.s_addr = cont_addr;*/

  /* create threads for data and control message processing */
  pthread_create(&dataThread, NULL, dataProcess, (void*)&data_sockaddr);
  pthread_create(&controlThread, NULL, controlProcess, (void*)&cont_sockaddr);
}
