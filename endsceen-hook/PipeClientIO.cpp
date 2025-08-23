#include "PipeClientIO.h"
#include <string>
PipeClientIO* g_pipeClient = nullptr;

PipeClientIO::PipeClientIO() {
	pipeName = R"(\\.\pipe\pipeserver)";
	overlapped = {};
	Connect(pipeName);
}

void PipeClientIO::Send(SHORT v) {

	bool res = WriteFile(
		hPipe,
		&v,
		sizeof(v),
		NULL,
		&overlapped
	);
	if (!res) {
		DWORD err = GetLastError();
		if (err == ERROR_IO_PENDING) {
			MessageBeep(MB_ICONQUESTION);
		}
		else
		{
			MessageBox(0, "[PIPE_IO] WriteFile failed to many writes in the queued up", ":(", 0);
		}
	}

}



void PipeClientIO::Disconnect() {
	if (hPipe != INVALID_HANDLE_VALUE && hPipe != NULL) {
		CloseHandle(hPipe);
		hPipe = NULL;
	}
}

void PipeClientIO::Connect(const char* pipeName) {
	hPipe = CreateFileA(
		pipeName,
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_FLAG_OVERLAPPED,
		NULL
	);

	if (hPipe == INVALID_HANDLE_VALUE)
	{
		MessageBox(0, std::string("[PipeClient] Failed to get a handle to pipe.").c_str(),">:(", 0);
	}

}

