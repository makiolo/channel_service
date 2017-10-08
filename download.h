#ifndef _CU_DOWNLOAD_H_
#define _CU_DOWNLOAD_H_

#include <raknet/TCPInterface.h>
#include <raknet/HTTPConnection.h>

void download(cu::yield_type& yield, const char* url, const char* filename)
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

      yield( cu::control_type{} );
    }

}

#endif
