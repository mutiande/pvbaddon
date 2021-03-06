//***************************************************************************
//                          main.cpp  -  description
//                             -------------------
//  begin            : So. Juni 8 09:08:14 2014
//  generated by     : pvdevelop (C) Lehrig Software Engineering
//  email            : lehrig@t-online.de
//***************************************************************************
#include "pvapp.h"
#include "rlwebcam.h"
#include "rlthread.h"
#include "rlsocket.h"
#include "rltime.h"
rlThread cam1_thread;
rlSocket *cam1_alarm;
// todo: comment me out. you can insert these objects as extern in your masks.
//rlModbusClient     modbus(modbusdaemon_MAILBOX,modbusdaemon_SHARED_MEMORY,modbusdaemon_SHARED_MEMORY_SIZE);
//rlSiemensTCPClient siemensTCP(siemensdaemon_MAILBOX,siemensdaemon_SHARED_MEMORY,siemensdaemon_SHARED_MEMORY_SIZE);
//rlPPIClient        ppi(ppidaemon_MAILBOX,ppidaemon_SHARED_MEMORY,ppidaemon_SHARED_MEMORY_SIZE);

void *Cam1_Thread(void *arg) // define your thread function
{
  char line[1024];
  if(arg == NULL) return NULL;
  THREAD_PARAM  *p = (THREAD_PARAM *) arg;
  //myThreadsData *d = (myThreadsData *) p->user;

  cam1_alarm = new rlSocket("localhost", 5051, 0);
  if(cam1_alarm == NULL)
  {
    printf("Cam1_Thread::Error could not construct rlsocket cam1_alarm.\n");
    return 0;
  }

  while(p->running)
  {
    printf("hello here is Cam1_Thread\n");
    if(cam1_alarm->isConnected() == 0) cam1_alarm->connect();
    if(cam1_alarm->isConnected())
    {
      //p->thread->lock();    // do something critical
      // for example write to a database
      int ret;
      int alarm = 0;
      while(1)
      {
        ret = cam1_alarm->readStr(line,sizeof(line)-1,100);
        if(ret > 0)
        {
          printf("Cam1_Thread::line=%s\n", line);
          if(strncmp(line,"HOST",4) == 0)
          {
            alarm = 1;
          }
        }
        else
        {
          printf("Cam1_Thread::else ret=%d\n", ret);
          break;
        }
      }
      rlsleep(100);
      if(alarm)
      {
        printf("Take alarm snapshot\n");
        rlTime now;
        now.getLocalTime();
        rlWebcam webcamBig;
        webcamBig.debug = 0;
        webcamBig.filename.printf("cam1_%04d-%02d-%02d_%02d-%02d-%02d.jpg",
                                                               now.year,
                                                               now.month,
                                                               now.day,
                                                               now.hour,
                                                               now.minute,
                                                               now.second);
        webcamBig.setUrl("http://192.168.1.32:80/snapshot.cgi?user=admin&pwd=");
        printf("filename=%s\n",webcamBig.filename.text());
        webcamBig.getFrame();
      }
      //p->thread->unlock();
    }
    else
    {
      printf("Cam1_Thread::Warning cam1 is not connected\n");
    }
    //rlsleep(100);        // goto sleep for 100 milliseconds
  }
  return arg;
}

int pvMain(PARAM *p)
{
int ret;

  pvSendVersion(p);
  pvSetCaption(p,"pvs");
  pvResize(p,0,1280,1024);
  //pvScreenHint(p,1024,768); // this may be used to automatically set the zoomfactor
  ret = 1;
  pvGetInitialMask(p);
  if(strcmp(p->initial_mask,"mask1") == 0) ret = 1;

  while(1)
  {
    if(trace) printf("show_mask%d\n", ret);
    switch(ret)
    {
      case 1:
        pvStatusMessage(p,-1,-1,-1,"mask1");
        ret = show_mask1(p);
        break;
      default:
        return 0;
    }
  }
}

#ifdef USE_INETD
int main(int ac, char **av)
{
PARAM p;

  pvInit(ac,av,&p);
  /* here you may interpret ac,av and set p->user to your data */
  pvMain(&p);
  return 0;
}
#else  // multi threaded server
int main(int ac, char **av)
{
PARAM p;
int   s;

  pvInit(ac,av,&p);
  cam1_thread.create(Cam1_Thread,NULL);
  /* here you may interpret ac,av and set p->user to your data */
  while(1)
  {
    s = pvAccept(&p);
    if(s != -1) pvCreateThread(&p,s);
    else        break;
  }
  return 0;
}
#endif
