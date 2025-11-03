using CoD4_dm1.FileFormats;
using System.Buffers.Binary;
using System.Collections.Generic;
using System.IO.Pipes;
using System.Threading.Tasks;
namespace CoD4_dm1.PipeServer
{
    public class NamedPipeServer
    {
        private readonly string _pipeName;

        private bool _recordState = false;
        private int _lastRecFrameCount = 0;
        private Structs.Entitys.Header _header;
        private readonly Record _recordClass;
        private readonly Csv _csv;

        public NamedPipeServer(Record record ,string pipeName = "pipeserver")
        {
            _pipeName = pipeName;
            _recordClass = record;
            _csv = new Csv();
        }

        /// <summary>
        /// Protocol:
        ///  - int (2 byte little-endian)
        ///  - 0 => heartbeat
        ///  - 1 => toggle recordState
        ///  - >1 => IDK
        /// </summary>
        public void PipeServerStart()
        {
            Console.WriteLine("===================================");
            using (var pipeServer = new NamedPipeServerStream(_pipeName, PipeDirection.InOut, 1, PipeTransmissionMode.Byte, PipeOptions.Asynchronous))
            {
                
                Console.WriteLine("[ Server ] Pipe is running, Waiting for client...");
                pipeServer.WaitForConnection();
                Console.WriteLine("[ Server ] Client connected!");

                try
                {
                    int incomingInstr;
                    using var reader = new BinaryReader(pipeServer);
                    while (pipeServer.IsConnected)
                    {
                        // ReadInt32 blocks until 2 bytes are available or client disconnects
                        try
                        {
                            //Console.WriteLine($"Raw Bytes recived: {reader.ReadBytes()}")
                            incomingInstr = reader.ReadInt16();
                        }
                        catch (EndOfStreamException)
                        {
                            Console.WriteLine("[ Server ] Client disconnected!");
                            break;
                        }

                        // heartbeat
                        if (incomingInstr == 0)
                        {
                            if (_recordState) 
                            {
                                _lastRecFrameCount = _recordClass.AddNewFrameToList();
                                Send(_lastRecFrameCount, _recordState, pipeServer);
                                continue;
                            }
                            else
                            {
                                Send(_lastRecFrameCount, _recordState, pipeServer);
                                continue;
                            }

                        }

                        if (incomingInstr == 1)
                        {
                            // Start Record
                            if (!_recordState)
                            {
                                ToggleRecordState();
                                _header = _recordClass.InitRecord();
                                Console.WriteLine($"Header info:\n   RecordFps: {_header.ConstCaptureFps}, CurrentMap: {_header.MapName}");
                                _lastRecFrameCount = _recordClass.AddNewFrameToList();
                                Send(_lastRecFrameCount, _recordState, pipeServer);
                                continue;
                            }
                            // Stop Record
                            else
                            {
                                //_recordClass.PrintFramesConsole();
                                
                                ToggleRecordState();
                                _ = Task.Run(() => _csv.ExportToCsvAsync(_header, new List<Structs.Entitys.Camera>(_recordClass.GetCamFrameList())));
                                _ = Task.Run(() => GlTF.ExportToGLB(_header, new List<Structs.Entitys.Camera>(_recordClass.GetCamFrameList())));
                                continue;
                            }
                        }
                        else 
                        {
                            Console.WriteLine($"[ Server ] Unkonw instruction: {incomingInstr}");
                        }

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
                Console.BackgroundColor = ConsoleColor.Cyan;
                Console.ForegroundColor = ConsoleColor.Black;
                Console.WriteLine($"[ Server ] Recording started _recordState: {_recordState}");
                Console.ResetColor();
            }
            else
            {
                Console.BackgroundColor = ConsoleColor.Yellow;
                Console.ForegroundColor = ConsoleColor.Black;
                Console.WriteLine($"[ Server ] Recording stopped _recordState: {_recordState}");
                _lastRecFrameCount = 0;
                Console.ResetColor();
            }
        }

        private async Task WriteToPipeAsync(int lastFrameNumber, NamedPipeServerStream pipe)
        {
            byte[] buffer = BitConverter.GetBytes(lastFrameNumber);

            await pipe.WriteAsync(buffer, 0, buffer.Length);
            
        }

        private void Send(int frameCount,bool recordState, NamedPipeServerStream pipe)
        {
            Span<byte> buf = stackalloc byte[5];
            BinaryPrimitives.WriteInt32LittleEndian(buf, frameCount);
            buf[4] = recordState ? (byte)1 : (byte)0;
            pipe.Write(buf);
            pipe.Flush();
        }
    }
}