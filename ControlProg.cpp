#include <stdin>
#include <string>
#include <map>

#include"Common.hpp"

using namespace std;

Map<int, int>ports;
Map<int, unsigned long>addrs;
int main()
{
 /* TODO Read in port/address info for each Node in network */
 
 sockaddr_in send_sock;
 control_header header;
 control_data data;
 
 char data_packet[PACKET_SIZE_BYTES];

 while(1)
 {

  String input;
  int node1, node2;
  
  cout << "Enter Command: ";
  cin >> input >> node1 >> node2;

  if(input.equals("generate-packet"))
  {
    /* reset packet struct */
    memset(data_packet, 0, PACKET_SIZE_BYTES);

    /* set up socket info for sending */
    send_sock.sin_family = AF_INET;
    send_sock.sin_port = htons(ports.find(node1));
    send_sock.sin_addr = adjAddrs.find(node1);

    /* packet info */
    header.pkt_type = CTRL_MSG_SEND_PCKT;
    data.node_ids[0] = node2;

    /* fill packet */
    memcpy(data_packet, &header, sizeof(header));
    memcpy(data_packet, &data, sizeof(data));

    /* send packet */
    sendto(sd, (const void*)data_packet, PACKET_SIZE_BYTES, 0, (struct sockaddr*)&send_data, sizeof(sockaddr));

  }
  else
  {
    /* determine whether to create or remove a link */
    control_type ctrl_tp = (input.equals("create-link")) ? CTRL_MSG_CREATE_LINK : CTRL_MSG_REMOVE_LINK;
    
    /* need to send message to both nodes about other node */
    /* reset packet struct */
    memset(data_packet, 0, PACKET_SIZE_BYTES);

    /* set up socket info for sending */
    send_sock.sin_family = AF_INET;
    send_sock.sin_port = htons(ports.find(node1));
    send_sock.sin_addr = adjAddrs.find(node1);

    /* packet info */
    header.pkt_type = ctrl_tp;
    data.node_ids[0] = node2;

    /* fill packet */
    memcpy(data_packet, &header, sizeof(header));
    memcpy(data_packet, &data, sizeof(data));

    /* send packet */
    sendto(sd, (const void*)data_packet, PACKET_SIZE_BYTES, 0, (struct sockaddr*)&send_data, sizeof(sockaddr));

    /* reset packet struct */
    memset(data_packet, 0, PACKET_SIZE_BYTES);

    /* set up socket info for sending */
    send_sock.sin_family = AF_INET;
    send_sock.sin_port = htons(ports.find(node1));
    send_sock.sin_addr = adjAddrs.find(node1);

    /* packet info */
    header.pkt_type = ctrl_tp;
    data.node_ids[0] = node1;

    /* fill packet */
    memcpy(data_packet, &header, sizeof(header));
    memcpy(data_packet, &data, sizeof(data));

    /* send packet */
    sendto(sd, (const void*)data_packet, PACKET_SIZE_BYTES, 0, (struct sockaddr*)&send_data, sizeof(sockaddr));

  }
 }
}
