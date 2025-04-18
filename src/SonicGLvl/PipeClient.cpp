#include "StdAfx.h"
#include "PipeClient.h"
#include <Windows.h>

void* thread_client(void* handle);
BYTE pipe_buffer[0x400];
UINT INVALID_MESSAGE = UINT32_MAX;

PipeClient::PipeClient()
{
	connected = false;
	client_running = false;
	hServer = 0;
	hClient = 0;
}

PipeClient::~PipeClient()
{
	if (client_running && message_thread.joinable())
	{
		message_thread.join();
	}
	Disconnect();
}

bool PipeClient::Connect()
{
	Disconnect();
	hServer = CreateFile("\\\\.\\pipe\\HedgehogIn", GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
	hClient = CreateFile("\\\\.\\pipe\\HedgehogOut", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);

	DWORD dwMode = PIPE_READMODE_MESSAGE;
	SetNamedPipeHandleState(hServer, &dwMode, NULL, NULL);
	SetNamedPipeHandleState(hClient, &dwMode, NULL, NULL);

	if (hServer != INVALID_HANDLE_VALUE && hClient != INVALID_HANDLE_VALUE)
	{
		connected = true;
		message_thread = std::thread(thread_client, this);
	}

	return false;
}

void PipeClient::AddMessageProcessor(void(*processor)(PipeClient* client, PipeMessage* msg))
{
	processors.push_back(processor);
}

void PipeClient::ProcessMessage(PipeMessage* msg)
{
	for (size_t i = 0; i < processors.size(); i++)
	{
		processors[i](this, msg);
	}
}

DWORD PipeClient::UploadMessage(const PipeMessage& msg, size_t size)
{
	DWORD dwWrittenTemp;
	DWORD dwWritten;
	
	DWORD state;
	if (GetNamedPipeHandleState(hServer, &state, NULL, NULL, NULL, NULL, NULL))
	{
		WriteFile(hServer, &msg, size, &dwWritten, NULL);
		WriteFile(hServer, &INVALID_MESSAGE, sizeof(INVALID_MESSAGE), &dwWrittenTemp, NULL);
		return dwWritten;
	}

	return 0;
}

void PipeClient::Disconnect() 
{
	CloseHandle(hClient);
	CloseHandle(hServer);
	connected = false;
}

void* thread_client(void* handle)
{
	PipeClient* client = (PipeClient*)handle;
	client->client_running = true;
	DWORD dwRead;
	while (ReadFile(client->getClientHandle(), pipe_buffer, sizeof(pipe_buffer), &dwRead, NULL)) {
		client->ProcessMessage((PipeMessage*)pipe_buffer);
	}
	client->client_running = false;
	return handle;
}