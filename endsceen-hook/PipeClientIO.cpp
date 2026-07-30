#include "PipeClientIO.h"
#include "FrameCounter.h"
#include <string>
PipeClientIO* g_pipeClient = nullptr;

PipeClientIO::PipeClientIO()
	: hPipe(INVALID_HANDLE_VALUE), hShutdownEvent(NULL), running(true) {
	pipeName = R"(\\.\pipe\pipeserver)";
	overlapped = {};
	readOverlapped = {};
	hShutdownEvent = CreateEventA(NULL, TRUE, FALSE, NULL);
	Connect(pipeName);

	workerThread = std::thread(&PipeClientIO::ReciveLoopStart, this);
}

PipeClientIO::~PipeClientIO() {
	Disconnect();
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
			PipeClientIO* self = this;
			g_pipeClient = nullptr;
			self->Disconnect();
			delete self;
		}
	}

}

void PipeClientIO::ReciveLoopStart() {
	while (running.load(std::memory_order_acquire) && hPipe != INVALID_HANDLE_VALUE) {
		uint8_t buffer[5] = {};
		DWORD bytesRead = 0;

		ResetEvent(readOverlapped.hEvent);
		BOOL ok = ReadFile(hPipe, buffer, sizeof(buffer), NULL, &readOverlapped);
		if (!ok && GetLastError() != ERROR_IO_PENDING) {
			break;
		}

		HANDLE waitHandles[2] = { readOverlapped.hEvent, hShutdownEvent };
		DWORD waitResult = WaitForMultipleObjects(2, waitHandles, FALSE, INFINITE);

		if (waitResult == WAIT_OBJECT_0 + 1) {
			CancelIoEx(hPipe, &readOverlapped);
			break;
		}
		if (waitResult != WAIT_OBJECT_0) {
			break;
		}

		if (!GetOverlappedResult(hPipe, &readOverlapped, &bytesRead, FALSE)) {
			break;
		}

		if (bytesRead >= 5 && g_frameCounter) {
			g_frameCounter->registedFrame = *reinterpret_cast<int*>(&buffer[0]);
			g_frameCounter->recState = buffer[4] != 0;
		}
	}
}

void PipeClientIO::Disconnect() {
	running.store(false, std::memory_order_release);
	if (hShutdownEvent) {
		SetEvent(hShutdownEvent);
	}
	if (workerThread.joinable()) {
		workerThread.join();
	}

	if (hPipe != INVALID_HANDLE_VALUE) {
		CloseHandle(hPipe);
		hPipe = INVALID_HANDLE_VALUE;
	}
	if (readOverlapped.hEvent) {
		CloseHandle(readOverlapped.hEvent);
		readOverlapped.hEvent = NULL;
	}
	if (hShutdownEvent) {
		CloseHandle(hShutdownEvent);
		hShutdownEvent = NULL;
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
		return;
	}

	readOverlapped.hEvent = CreateEventA(NULL, TRUE, FALSE, NULL);
}
