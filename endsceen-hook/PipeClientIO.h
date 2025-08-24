#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

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
	HANDLE hPipe;
};

extern PipeClientIO* g_pipeClient;