#include "pcompat.h"
#include "plibs.h"

#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <strsafe.h>

#define POVERLAY_BUFSIZE 4096

#include "poverlay.h"

LPCWSTR PORT = TEXT("\\\\.\\pipe\\pStatusPipe");

void overlay_main_loop(VOID)
{
   BOOL   fConnected = FALSE;
   HANDLE hPipe = INVALID_HANDLE_VALUE;
   for (;;)
   {
      debug(D_NOTICE, "\nPipe Server: Main thread awaiting client connection on %s\n", PORT);
      hPipe = CreateNamedPipe(
          PORT,                     // pipe name
          PIPE_ACCESS_DUPLEX,       // read/write access
          PIPE_TYPE_MESSAGE |       // message type pipe
          PIPE_READMODE_MESSAGE |   // message-read mode
          PIPE_WAIT,                // blocking mode
          PIPE_UNLIMITED_INSTANCES, // max. instances
          POVERLAY_BUFSIZE,         // output buffer size
          POVERLAY_BUFSIZE,         // input buffer size
          0,                        // client time-out
          NULL);                    // default security attribute

      if (hPipe == INVALID_HANDLE_VALUE)
      {
          debug(D_NOTICE, "CreateNamedPipe failed, GLE=%d.\n", GetLastError());
          return;
      }

      fConnected = ConnectNamedPipe(hPipe, NULL) ?
         TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);

      if (fConnected)
      {
         debug(D_NOTICE, "Client connected, creating a processing thread.\n");
         psync_run_thread1(
            "Pipe request handle routine",
            instance_thread,    // thread proc
            (LPVOID) hPipe     // thread parameter
         );
       }
      else
         CloseHandle(hPipe);
   }

   return;
}

void instance_thread(LPVOID lpvParam)
{
   HANDLE hHeap      = GetProcessHeap();
   message* request = (message*)HeapAlloc(hHeap, 0, POVERLAY_BUFSIZE);
   message* reply   = (message*)HeapAlloc(hHeap, 0, POVERLAY_BUFSIZE);

   DWORD cbBytesRead = 0, cbWritten = 0;
   BOOL fSuccess = FALSE;
   HANDLE hPipe  = NULL;

   if (lpvParam == NULL)
   {
       debug(D_ERROR,  "InstanceThread got an unexpected NULL value in lpvParam.\n");
       if (pchReply != NULL) HeapFree(hHeap, 0, pchReply);
       if (pchRequest != NULL) HeapFree(hHeap, 0, pchRequest);
       return (DWORD)-1;
   }

   if (pchRequest == NULL)
   {
       debug(D_ERROR,  "   InstanceThread got an unexpected NULL heap allocation.\n");
       if (pchReply != NULL) HeapFree(hHeap, 0, pchReply);
       return (DWORD)-1;
   }

   if (pchReply == NULL)
   {
       debug(D_ERROR,  "   InstanceThread got an unexpected NULL heap allocation.\n");
       if (pchRequest != NULL) HeapFree(hHeap, 0, pchRequest);
       return (DWORD)-1;
   }

   debug(D_NOTICE, "InstanceThread created, receiving and processing messages.\n");
   hPipe = (HANDLE) lpvParam;
   while (1)
   {
      fSuccess = ReadFile(
         hPipe,        // handle to pipe
         request,    // buffer to receive data
         POVERLAY_BUFSIZE, // size of buffer
         &cbBytesRead, // number of bytes read
         NULL);        // not overlapped I/O

      if (!fSuccess || cbBytesRead == 0)
      {
          if (GetLastError() == ERROR_BROKEN_PIPE)
              debug(D_NOTICE, "InstanceThread: client disconnected.\n");
          else

              debug(D_NOTICE, "InstanceThread ReadFile failed, GLE=%d.\n", GetLastError());
          break;
      }
      get_answer_to_request(request, reply);
      fSuccess = WriteFile(
         hPipe,        // handle to pipe
         reply,     // buffer to write from
         reply->length, // number of bytes to write
         &cbWritten,   // number of bytes written
         NULL);        // not overlapped I/O

      if (!fSuccess || reply->length != cbWritten)
      {
          debug(D_NOTICE, "InstanceThread WriteFile failed, GLE=%d.\n", GetLastError());
          break;
      }
  }
  FlushFileBuffers(hPipe);
  DisconnectNamedPipe(hPipe);
  CloseHandle(hPipe);
  HeapFree(hHeap, 0, pchRequest);
  HeapFree(hHeap, 0, pchReply);


   debug(D_NOTICE, "InstanceThread exitting.\n");
   return;
}

void get_answer_to_request(message request, message replay)
{
    const char msg[4] = "Ok.";
    msg[3] = '\0';
    printf( "Client Request type [%d] len [%d] string: [%s]", request.type, request.length, request.value );
    replay.type = 10;
    replay.length = sizeof(message)+ 3;
    strncpy(replay.value, msg, 4);

}


