#include<pthread.h>
#include<semaphore.h>
#include<vector>

using namespace std;

vector<pair<int, int>>hostDistances;
vector<int>routePorts;
int controlPrt
int dataPrt;

void dataProcess()
{
  while(1)
  {

  }
}
void controlProcess()
{
  while(1)
  {

  }
}

int main()
{
  pthread_t dataThread, controlThread;

  /* create threads for data and control message processing */
  pthread_create(&dataThread, NULL, dataProcess, NULL);
  pthread_create(&controlThread, NULL, controlProcess, NULL);
}
