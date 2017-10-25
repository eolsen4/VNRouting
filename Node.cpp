#include<pthread.h>
#include<semaphore.h>
#include<vector>
#include<arpa/inet.h>
#include<cassert>

using namespace std;

/* distance info for routing */
Map<int, int>hostDistances;
/* ports and addresses for any Node directly linked to this one */
Map<int,int>routePorts;
Map<int, unsigned long>hostAddrs;

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

  while(1)
  {
    
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
