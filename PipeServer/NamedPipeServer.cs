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
        private readonly string _pipeName = "drainpipe";

        public NamedPipeServer()
        {
            
        }

        public async Task PipeServerStart()
        {
            using (var pipeServer = new NamedPipeServerStream(_pipeName, PipeDirection.InOut, 1, PipeTransmissionMode.Byte, PipeOptions.Asynchronous))
            {
                Console.WriteLine("NamedPipe server is running");
                await pipeServer.WaitForConnectionAsync();
                Console.WriteLine("Client connected!");
                
                using var reader = new StreamReader(pipeServer, Encoding.UTF8, leaveOpen: true);
                using (var writer = new StreamWriter(pipeServer, Encoding.UTF8, leaveOpen: true) { AutoFlush = true })
                {
                    string? instruction = await reader.ReadLineAsync();

                    string response = EvaulateMessagae(instruction);

                    await writer.WriteLineAsync(response);
                }



            }
        }

        private string EvaulateMessagae(string instruction)
        {

            return "";
            
        }

    }
}
