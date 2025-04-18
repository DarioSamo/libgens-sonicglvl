#pragma once

struct PipeMessage;

class PipeClient
{
protected:
	HANDLE hServer;
	HANDLE hClient;
	std::thread message_thread;
	DWORD connected;
	std::vector<void(*) (PipeClient* client, PipeMessage* msg)> processors;

public:
	PipeClient();
	~PipeClient();
	bool Connect();
	void Disconnect();
	void AddMessageProcessor(void(*processor)(PipeClient* client, PipeMessage* msg));
	void ProcessMessage(PipeMessage* msg);
	DWORD UploadMessage(const PipeMessage& msg, size_t size);
	bool client_running;

	HANDLE getClientHandle() {
		return hClient;
	}

	bool checkConnection() {
		GetNamedPipeHandleState(hClient, NULL, &connected, NULL, NULL, NULL, NULL);
		return connected;
	}
};


struct PipeMessage 
{
	UINT ID;
};
