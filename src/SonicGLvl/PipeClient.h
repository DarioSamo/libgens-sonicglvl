#pragma once

struct PipeMessage;

class PipeClient
{
protected:
	HANDLE hServer;
	HANDLE hClient;
	pthread_t message_thread;
	DWORD connected;
	std::vector<void(*) (PipeClient* client, PipeMessage* msg)> processors;

public:
	PipeClient();
	~PipeClient();
	bool Connect();
	void Disconnect();
	void AddMessageProcessor(void(*processor)(PipeClient* client, PipeMessage* msg));
	void ProcessMessage(PipeMessage* msg);
	DWORD UploadMessage(PipeMessage* msg, size_t size);
	bool client_running;

	HANDLE getClientHandle() {
		return hClient;
	}

	bool checkConnection() {
		GetNamedPipeHandleState(hClient, NULL, &connected, NULL, NULL, NULL, NULL);
		if (client_running) {
			pthread_kill(message_thread, 0);
			client_running = false;
		}
		return connected;
	}
};


struct PipeMessage 
{
	UINT ID;
};
