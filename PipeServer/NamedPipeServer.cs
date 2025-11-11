using CoD4_dm1.config;
using CoD4_dm1.FileFormats;
using System.Buffers.Binary;
using System.IO.Pipes;
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

        public NamedPipeServer(Record record, string pipeName = "pipeserver")
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
                                _ = Task.Run(() => _csv.ExportToCsvAsync(_header, new List<Structs.Entitys.Camera>(_recordClass.GetCamFrameList())))
                                    .ContinueWith(t =>
                                    {
                                        if (t.IsFaulted)
                                        {
                                            var agg = t.Exception;
                                            ConsoleSetting.WriteError("CSV export failed: " + agg.Flatten().InnerException);
                                        }
                                        else if (t.IsCanceled)
                                        {
                                            ConsoleSetting.WriteWarning("CSV export cancelled");
                                        }
                                    }, TaskScheduler.Default);

                                _ = Task.Run(() => GlTF.ExportToGLB(_header, new List<Structs.Entitys.Camera>(_recordClass.GetCamFrameList())))
                                    .ContinueWith(t =>
                                    {
                                        if (t.IsFaulted)
                                        {
                                            var agg = t.Exception;
                                            ConsoleSetting.WriteError("GLTF Export failed : " + agg.Flatten().InnerException);
                                        }
                                        else if (t.IsCanceled)
                                        {
                                            ConsoleSetting.WriteWarning("GLTF exprot cancelled");
                                        }
                                    }, TaskScheduler.Default);
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
                ConsoleSetting.WriteInfo($"[ Server ] Recording started _recordState: {_recordState}");
            }
            else
            {
                ConsoleSetting.WriteInfo($"[ Server ] Recording stopped _recordState: {_recordState}");
                _lastRecFrameCount = 0;
            }
        }

        private async Task WriteToPipeAsync(int lastFrameNumber, NamedPipeServerStream pipe)
        {
            byte[] buffer = BitConverter.GetBytes(lastFrameNumber);

            await pipe.WriteAsync(buffer, 0, buffer.Length);

        }

        private void Send(int frameCount, bool recordState, NamedPipeServerStream pipe)
        {
            Span<byte> buf = stackalloc byte[5];
            BinaryPrimitives.WriteInt32LittleEndian(buf, frameCount);
            buf[4] = recordState ? (byte)1 : (byte)0;
            pipe.Write(buf);
            pipe.Flush();
        }
    }
}