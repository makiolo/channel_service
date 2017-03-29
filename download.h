#ifndef _DOWNLOAD_H_
#define _DOWNLOAD_H_

void download(const char* url, const char* filename)
{
    RakNet::TCPInterface tcp;
    RakNet::HTTPConnection httpConnection;

    httpConnection.Init(&tcp, url);
    httpConnection.Get(filename);

    tcp.Start(0, 64);

    for(;;)
    {
      RakNet::Packet *packet = tcp.Receive();
      if(packet)
      {
         httpConnection.ProcessTCPPacket(packet);
         tcp.DeallocatePacket(packet);
      }
      httpConnection.Update();

      if (httpConnection.IsBusy()==false)
      {
         RakNet::RakString fileContents = httpConnection.Read();
         printf(fileContents.C_String());
         return;
      }

      // Prevent 100% cpu usage
      // TODO: use yield()
      RakSleep(30);
    }

}

#endif
