//***************************************************************************
//                          main.cpp  -  description
//                             -------------------
//  begin            : Sa. Dez. 31 12:35:10 2016
//  generated by     : pvdevelop (C) Lehrig Software Engineering
//  email            : lehrig@t-online.de
//***************************************************************************
#include "pvapp.h"
// todo: comment me out. you can insert these objects as extern in your masks.
// Change "modbus"(Change if applicable) and "modbusdaemon_" with the "name" of your file "name.mkmodbus".
//rlModbusClient     modbus(modbusdaemon_MAILBOX,modbusdaemon_SHARED_MEMORY,modbusdaemon_SHARED_MEMORY_SIZE);
//rlSiemensTCPClient siemensTCP(siemensdaemon_MAILBOX,siemensdaemon_SHARED_MEMORY,siemensdaemon_SHARED_MEMORY_SIZE);
//rlPPIClient        ppi(ppidaemon_MAILBOX,ppidaemon_SHARED_MEMORY,ppidaemon_SHARED_MEMORY_SIZE);

static void parseUrl(char *url, rlString *filename, rlString *parameters = NULL)
{
  char *end;
  rlString fname, params;
  //printf("Begin: parseUrl(%s)\n",url);
  if(url == NULL) return;
  end = strchr(url, 0x0d);
  if(end != NULL) *end = '\0';
  end = strchr(url, 0x0a);
  if(end != NULL) *end = '\0';
  printf("Begin: parseUrl(%s) filename=%s parameters=%s\n",url,filename->text(),parameters->text());
  if(strncmp(url,"GET ",4) != 0) return;
  if(filename == NULL) return;
  const char *cptr = &url[4];
  while(*cptr == ' ') cptr++;
  if(*cptr == '/') cptr++;

  fname.setText(cptr);
  if(parameters != NULL)
  {
    const char *par = strchr(fname.text(),'?');
    if(par != NULL)
    {
      par++;
      params.setText(par);
      char *end = strchr(params.text(),' ');
      if(end != NULL)
      {
        *end = '\0';
        *parameters = params;
      }
    }
  }
  end = strchr(fname.text(),'?');
  if(end != NULL)
  {
    *end = '\0';
  }
  end = strchr(fname.text(),' ');
  if(end != NULL)
  {
    *end = '\0';
  }
  *filename = fname;
  printf("Return: parseUrl(%s) filename=%s parameters=%s\n",url,filename->text(),parameters->text());
}

int runHttpServer(PARAM *p)
{
  char buf[MAX_EVENT_LENGTH];
  int upgrade = 0;
  int first = 1;
  while(1)
  {
    while(1) // read the http header that follows the http url 
    { // here we simply ignore the http header
      if(first == 1) strcpy(buf,p->url);
      else           pvtcpreceive(p,buf, sizeof(buf) -1);
      first = 0;
      if(trace) printf("while buf=%s\n", buf);
      if(strncmp(buf,"GET ",4) == 0)
      {
        rlString filename, parameters;
        parseUrl(buf,&filename,&parameters);
        while(1) // read the http header that follows the http url 
        { // here we simply ignore the http header
          pvtcpreceive(p,buf, sizeof(buf) -1);
          printf("while buf=%s\n", buf);
          if(strncmp(buf,"Upgrade:",8) == 0) upgrade = 1;
          if(strlen(buf) < 3) break;
        }
        if(upgrade) return 1;
        if(trace) printf("send response to %s\n", p->url);
        pvSendHttpResponseFile(p, filename.text());
        //pvSendHttpResponse(p, html.text());
      }
    }
  }
  return 0;
}

int pvMain(PARAM *p)
{
int ret=1;

  pvGetInitialMask(p);
  if(trace) printf("pvMain start with ret=%d p->url=%s\n", ret, p->url);
  if(strncmp(p->url,"GET ",4) == 0) // test if we got a http GET request
  { 
    printf("starting with http request\n");
    runHttpServer(p);
  }  
  else
  {
    pvSendVersion(p); // if a pvbrowser client is connected
    pvSetCaption(p,"pvs");
  }

  while(1)
  {
    switch(ret)
    {
      case 1:
        //pvStatusMessage(p,-1,-1,-1,"mask1");
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
  /* here you may interpret ac,av and set p->user to your data */
  while(1)
  {
    s = pvAccept(&p);
    if(s != -1) pvCreateThread(&p,s);
  }
  return 0;
}
#endif
