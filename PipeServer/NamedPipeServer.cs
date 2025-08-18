using System;
using System.Collections.Generic;
using System.IO.Pipes;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CoD4_dm1.PipeServer
{
    public class NamedPipeServer
    {
        private readonly string _pipeName;

        // State
        private bool _recordState = false;
        private int _lastFrameNumber = 0;

        //Hooks
        public Action? OnStartRecording;
        public Action? OnStopRecording;
        public Action<int>? OnFrameNumber;
        public Action? OnHeartbeat;

        public NamedPipeServer(string pipeName = "drainpipe")
        {
            _pipeName = pipeName;
        }

        /// <summary>
        /// Protocol:
        ///  - int (4 byte little-endian)
        ///  - 0 => heartbeat
        ///  - 1 => toggle recordState
        ///  - >1 => frame number
        /// </summary>
        public void PipeServerStart()
        {
            Console.WriteLine("===============================");
            using (var pipeServer = new NamedPipeServerStream(_pipeName, PipeDirection.InOut, 1, PipeTransmissionMode.Message, PipeOptions.None)) // No async
            {
                Console.WriteLine("[ Server ] Pipe is running, Waiting for client...");
                pipeServer.WaitForConnection();
                Console.WriteLine("[ Server ] Client connected!");

                try
                {
                    using var reader = new BinaryReader(pipeServer);
                    while (pipeServer.IsConnected)
                    {
                        // ReadInt32 blocks until 4 bytes are available or client disconnects
                        int value;
                        try
                        {
                            value = reader.ReadInt32();
                        }
                        catch (EndOfStreamException)
                        {
                            Console.WriteLine("[ Server ] Client disconnected!");
                            break;
                        }

                        // Evaluate value
                        if (value == 0)
                        {
                            // heartbeat
                            OnHeartbeat?.Invoke();
                            // nothing else
                            continue;
                        }

                        if (value == 1)
                        {
                            ToggleRecordState();
                            continue;
                        }

                        // value > 1 => frame number (assumption)
                        UpdateLastFrameNumber(value);
                    }
                }
                catch (IOException ioEx)
                {
                    Console.WriteLine("[ Server ] IO error on pipe: " + ioEx.Message);
                }
                finally
                {
                    if (pipeServer.IsConnected) pipeServer.Disconnect();
                    Console.WriteLine("[ Server ] server stopped.");
                }
            }
        }

        private void ToggleRecordState()
        {
                        
            _recordState = !_recordState;
            
            if (_recordState)
            {
                Console.WriteLine($"Recording started _recordState: {_recordState}");
                OnStartRecording?.Invoke();
            }
            else
            {
                Console.WriteLine($"Recording stopped _recordState: {_recordState}");
                OnStopRecording?.Invoke();
            }
        }

        private void UpdateLastFrameNumber(int frame)
        {
            
            _lastFrameNumber = frame;
            
            Console.WriteLine($"Frame received: {frame}");
            OnFrameNumber?.Invoke(frame);
        }

        public (bool recordState, int lastFrameNumber) GetState()
        {
            
            return (_recordState, _lastFrameNumber);
        }
    }
}