#include <iostream>
#include <pthread.h>
#include <vector>
#include <arpa/inet.h>
#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <cstring>
#include <unistd.h>
#include <assert.h>
#include <netdb.h>
#include <sys/uio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/param.h>
#include <sys/utsname.h>
#include <map>
#include <vector>
#include <time.h>

#include "Common.hpp"

using namespace std;

/* TODO Create control program to drive nodes */

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
map<int, int>adjDataPorts;
map<int, int>adjContPorts;
map<int, string>adjHostnames;

/* flag for when a data message has been requested to send */
bool sendMessage = false;
int sendToNode;

/* current packet id to send out */
char pckt_id = 0;

int createSock(void* input)
{
  /* cast input back to sockaddr */
  struct sockaddr_in* sa = static_cast<struct sockaddr_in*>(input);

  socklen_t size = sizeof(sockaddr);

  /* create a datagram socket */
  int retVal = socket(AF_INET, SOCK_DGRAM, 0);

  /* if socket doesn't bind something went very wrong */
  if(bind(retVal, (struct sockaddr*)sa, size) == -1){
    perror("Error: bind function in createSocket function failed");
    return -1;
  }

  return retVal;
}

static void* dataProcess(void* input)
{
#ifdef DATADEBUG
  cout << "Data process starting yo" << endl;
#endif

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

    /* set a timeout for select */
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;

    /* check for incoming meesages */
    select(sd+1, &rfds, NULL, NULL, &tv);

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
        send_data.sin_port = htons(adjDataPorts.at(nextNode));
        //send_data.sin_addr.s_addr = adjAddrs.at(nextNode);

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
    }
    pthread_mutex_lock(&dataLock);

    if(sendMessage)
    {

#ifdef DATADEBUG
      printf("Recieved a message request to send to %d\n", sendToNode);
#endif
      /* reset */
      sendMessage = false;

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

      /* determine where to send the message next */
      int nextHop = routeNodes.find(sendToNode)->second;

#ifdef DATADEBUG
      printf("Dest:%d, next node to hop to: %d\n", sendToNode, nextHop);
#endif
      send_data.sin_port = htons(adjDataPorts.at(nextHop)); 

      /* gets info about the node being sent to based on the host name */
      struct hostent* hInfo = gethostbyname(adjHostnames.find(nextHop)->second.c_str());

#ifdef DATADEBUG
      printf("Hostname: %s, Port: %d\n", adjHostnames.find(nextHop)->second.c_str(), adjDataPorts.at(nextHop));
#endif 
      memcpy(&send_data.sin_addr, hInfo->h_addr, hInfo->h_length);

      sendto(sd, (const void*)data_packet, PACKET_SIZE_BYTES, 0, (struct sockaddr*)&send_data, sizeof(sockaddr));
    }

    pthread_mutex_unlock(&dataLock);
  }
}

static void* controlProcess(void* input)
{
  int sd = createSock(input);
  int recieved_len, itr, rec_id;
  struct sockaddr_in recieved_data, send_data;

  char control_packet[PACKET_SIZE_BYTES];

  ControlHeader header;
  ControlData data;

  /* set up fd_sets*/
  fd_set rfds, store;

  FD_ZERO(&rfds);
  FD_ZERO(&store);

  FD_SET(sd, &store);

  /* set timer for sending vectors */
  time_t start, end;
  start = time(0);

#ifdef DATADEBUG
  /* testing data message requests */
  time_t testMsg = time(0);
#endif

  while(1)
  {
    memset(control_packet, 0, PACKET_SIZE_BYTES);

    rfds = store;

    socklen_t size = sizeof(sockaddr);

    /* set a timeout for select */
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;

    /* check for incoming meesages */
    select(sd+1, &rfds, NULL, NULL, &tv);

    if(FD_ISSET(sd, &rfds))
    {
      /* recieve data_packet */
      recieved_len = recvfrom(sd, (void*)control_packet, PACKET_SIZE_BYTES, 0, (struct sockaddr*)&recieved_data, &size);

#ifdef CONTDEBUG
      printf("recieved a control message\n");
#endif
      /* parse out packet data into header and data structs */
      memcpy(&header, control_packet, sizeof(header));
      memcpy(&data, control_packet+sizeof(header), PACKET_SIZE_BYTES-sizeof(header));

      /* if we recieved a routing vector */
      if(ROUTING_VECTOR == header.pkt_type)
      {
        pthread_mutex_lock(&dataLock);

        /* need to figure out the ID of the sending node */
        rec_id = header.sending_node;

#ifdef CONTDEBUG
        printf("Recieved from: %d\n", rec_id);
#endif
        /* determine if the new control packet offers us any shorter paths */
        itr = 0;
        while(data.node_ids[itr] != -1)
        {
          int temp_id = data.node_ids[itr];

#ifdef CONTDEBUG
          printf("node in routing table: %d\n", temp_id);
#endif
          /* if we don't already have a path to the node, or the path through the
           * old intermediary was overwritten, or this new path is shorter,
           * update */
          if(routeNodes.find(temp_id) == routeNodes.end())
          {
            routeNodes.insert(pair<int,int>(temp_id, rec_id));
            nodeDistances.insert(pair<int,int>(temp_id, data.weights[itr]+1));
          }
          else if(rec_id == routeNodes.find(temp_id)->second || 
              (data.weights[itr]+1) < nodeDistances.find(temp_id)->second)
          {
            nodeDistances.find(temp_id)->second = data.weights[itr]+1;
            routeNodes.find(temp_id)->second = rec_id;
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

    /* check if time to send vector */
    end = time(0);
    if(difftime(end, start)*1000 > 2)
    {
      start = end;
      map<int,int>::iterator iter = adjContPorts.begin();

      /* need to set up the packet */
      ControlHeader header;
      ControlData data;
      header.pkt_type = ROUTING_VECTOR;
      header.sending_node = node_id;

      memset(&data, 0, sizeof(data));

      int index = 0;
      for(map<int,int>::iterator iter2 = nodeDistances.begin(); iter2 != nodeDistances.end(); ++iter2, ++index)
      {
        data.node_ids[index] = iter2->first;
        data.weights[index] = iter2->second;
#ifdef CONTDEBUG
        printf("Node: %d, Distance: %d\n", data.node_ids[index], data.weights[index]);
#endif
      }
      /* use -1 as end flag*/
      data.node_ids[index] = -1;

      /* zero out packet and fill it */
      memset(control_packet, 0, PACKET_SIZE_BYTES);
      memcpy(control_packet, &header, sizeof(header));
      memcpy(control_packet+sizeof(header), &data, sizeof(data));

      while(iter != adjContPorts.end())
      {
#ifdef CONTDEBUG
        printf("Sending routing vector to node: %d\n", iter->first);
#endif
        /* need to set up sockaddr struct to sent out packet */
        /* get the port and address of the next host to send to*/
        send_data.sin_family = AF_INET;
        send_data.sin_port = htons(iter->second);

        /* gets info about the node being sent to based on the host name */
        struct hostent* hInfo = gethostbyname(adjHostnames.find(iter->first)->second.c_str());

#ifdef CONTDEBUG
        printf("Hostname: %s, Port: %d\n", adjHostnames.find(iter->first)->second.c_str(), iter->second);
#endif 
        memcpy(&send_data.sin_addr, hInfo->h_addr, hInfo->h_length);

        sendto(sd, (const void*)control_packet, PACKET_SIZE_BYTES, 0, (struct sockaddr*)&send_data, sizeof(sockaddr));

        ++iter;
      }
    }
#ifdef DATADEBUG
    /* until we are capable of sending control messages from program, use this to
     * test data thread */
    if(node_id == 1 && difftime(end, testMsg) * 1000 > 5)
    {
      printf("request for data message generated\n");
      testMsg = end;
      sendMessage = true;
      sendToNode = 3;
    } 
#endif
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

  string filename = argv[1];
  node_id = atoi(argv[2]);

  /* retrieves the current node's hostname, control port, and data port */
  string hostname = getHostname(filename, node_id);
  int data_port = getDataPort(filename, node_id);
  int cont_port = getContPort(filename, node_id);

  /* retrieves the adjacent nodes' hostnames, data ports, and control ports */
  vector<pair<int, int> > adjacent_data_ports = getAdjacentDataPorts(filename, node_id);
  vector<pair<int, int> > adjacent_cont_ports = getAdjacentContPorts(filename, node_id);
  vector<pair<int, string> > adjacent_hostnames = getAdjacentHostnames(filename, node_id);

  /* initializes the map for adjacent nodes' hostnames, data ports, and control ports */
  for(int i = 0; i < adjacent_data_ports.size(); ++i){
    adjDataPorts.insert(adjacent_data_ports[i]);
    adjContPorts.insert(adjacent_cont_ports[i]);
    adjHostnames.insert(adjacent_hostnames[i]);
    nodeDistances.insert(make_pair(adjacent_data_ports[i].first, 1));
    routeNodes.insert(make_pair(adjacent_data_ports[i].first, node_id));
#ifdef CONTDEBUG
    printf("Adjacent Node: %d: hostname: %s, data port:%d, control port:%d\n", adjacent_data_ports[i].first,
        adjacent_hostnames[i].second.c_str(),
        adjacent_data_ports[i].second,
        adjacent_cont_ports[i].second);
#endif
  }
  /* add self to nodeDistance map */
  nodeDistances.insert(pair<int, int>(node_id, 0));
  routeNodes.insert(pair<int,int>(node_id,node_id));

  struct hostent *name;
  if ((name = gethostbyname(hostname.c_str())) == NULL){
    perror("utils: gethostbyname fucntion error");
    exit(1);
  }

  memset(&data_sockaddr, 0, sizeof(data_sockaddr));
  data_sockaddr.sin_family = AF_INET;
  data_sockaddr.sin_port = htons(data_port);
  data_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  //memcpy((void *)&data_sockaddr.sin_addr, name->h_addr_list[0], name->h_length);

  cont_sockaddr.sin_family = AF_INET;
  cont_sockaddr.sin_port = htons(cont_port);
  memcpy((void *)&cont_sockaddr.sin_addr, name->h_addr_list[0], name->h_length);


  /* create threads for data and control message processing */
  int dataRet = pthread_create(&dataThread, NULL, dataProcess, (void*)&data_sockaddr);
  int contRet = pthread_create(&controlThread, NULL, controlProcess, (void*)&cont_sockaddr);

  pthread_join(dataThread, NULL);
  if(dataRet != 0)
  {
    printf("Error: pthread_create failed");
    exit(1);
  }
  pthread_join(controlThread, NULL);
  if(dataRet != 0)
  {
    printf("Error: pthread_create failed");
    exit(1);
  }
}
