#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <atomic>
#include <thread>

class PipeClientIO
{

public:
	PipeClientIO();
	~PipeClientIO();
	
	void Connect(const char* pipeName);
	void Disconnect();
	void Send(SHORT v);
	void ReciveLoopStart();

private:
	const char* pipeName;
	OVERLAPPED overlapped;
	OVERLAPPED readOverlapped;
	HANDLE hPipe;
	HANDLE hShutdownEvent;
	std::thread workerThread;
	std::atomic<bool> running;
};

extern PipeClientIO* g_pipeClient;
