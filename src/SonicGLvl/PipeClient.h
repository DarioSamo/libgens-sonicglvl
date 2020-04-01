#pragma once

struct PipeMessage;

class PipeClient
{
protected:
	HANDLE hServer;
	HANDLE hClient;
	pthread_t message_thread;
	bool connected;
	std::vector<void(*) (PipeClient* client, PipeMessage* msg)> processors;

public:
	PipeClient();
	~PipeClient();
	bool Connect();
	void Disconnect();
	void AddMessageProcessor(void(*processor)(PipeClient* client, PipeMessage* msg));
	void ProcessMessage(PipeMessage* msg);
	DWORD UploadMessage(PipeMessage* msg, size_t size);

	HANDLE getClientHandle() {
		return hClient;
	}

	bool checkConnection() {
		return connected;
	}

	void forceConnectionCheck(){
		DWORD state;
		connected = GetNamedPipeHandleState(hClient, &state, NULL, NULL, NULL, NULL, NULL);
	}
};


struct PipeMessage 
{
	UINT ID;
};
