#include <stdin>
#include <string>

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

 while(1)
 {
  String input;
  int node1, node2;
  
  cout << "Enter Command: ";
  cin >> input >> node1 >> node2;

  if(input.equals("generate-packet"))
  {
    send_sock.sin_family = AF_INET;
    send_sock.sin_port = htons(ports.find(node1));
    send_sock.sin_addr = adjAddrs.find(node1);

    header.pkt_type = CTRL_MSG_SEND_PCKT;
    data.node_ids[0] = node2;

    sendto(sd, (const void*)data_packet, PACKET_SIZE_BYTES, 0, (struct sockaddr*)&send_data, sizeof(sockaddr));

  }
  else if(input.equals("create-link"))
  {

  }
  else
  {

  }
 }
}
