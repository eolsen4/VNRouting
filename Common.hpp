/* max packet size */
#define PACKET_SIZE_BYTES 1000

enum control_type
{
  ROUTING_VECTOR,
  CTRL_MSG_SEND_PCKT,
  CTRL_MSG_CREATE_LINK
};

/* header for control structure */
typedef struct controlHeader
{
  /* what type of packet is this */
  control_type pkt_type;
}

/* control data struct */
typedef struct ControlData
{
  char node_ids[((PACKET_SIZE_BYTES)/2)-2];
  char weights[((PACKET_SIZE_BYTES)/2)-2];

} ControlData;

