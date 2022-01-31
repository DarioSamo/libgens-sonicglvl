using System;
using System.IO;
using System.IO.Pipes;
using LiteNetLib;
using Reloaded.Messaging;
using Reloaded.Messaging.Interfaces;
using Reloaded.Messaging.Messages;
using Reloaded.Messaging.Structs;

namespace SonicGLvl
{
    public class PipeHost<TMessageType> : IDisposable where TMessageType : unmanaged
    {
        public NamedPipeServerStream ServerPipe { get; private set; }
        public NamedPipeServerStream ClientPipe { get; private set; }
        
        public string PipeName { get; }
        
        public MessageHandler<TMessageType> MessageHandler { get; }

        protected byte[] DataBuffer;

        public PipeHost(string pipeName, int bufferSize = 0x400)
        {
            PipeName = pipeName;

            ServerPipe = CreatePipe(PipeDirection.In);
            ClientPipe = CreatePipe(PipeDirection.Out);

            DataBuffer = new byte[bufferSize];
            MessageHandler = new MessageHandler<TMessageType>();
        }

        public PipeHost<TMessageType> Start()
        {
            ServerPipe.BeginWaitForConnection(ConnectionCallback, ServerPipe);
            ClientPipe.BeginWaitForConnection(ConnectionCallback, ClientPipe);
            return this;
        }

        public void SendMessage<TStruct>(Message<TMessageType, TStruct> msg) where TStruct : unmanaged, IMessage<TMessageType>
        {
            try
            {
                var disposed = ClientPipe.SafePipeHandle.IsClosed;
            }
            catch (ObjectDisposedException)
            {
                ClientPipe = CreatePipe(PipeDirection.Out);
                ClientPipe.BeginWaitForConnection(ConnectionCallback, ClientPipe);
            }

            var data = msg.Serialize();

            try
            {
                ClientPipe.BeginWrite(data, 0, data.Length, WriteCallback, ClientPipe);
            }
            catch(IOException)
            {
                ClientPipe.Close();
                ClientPipe = CreatePipe(PipeDirection.Out);
                ClientPipe.BeginWaitForConnection(ConnectionCallback, ClientPipe);
            }
        }

        protected void ConnectionCallback(IAsyncResult state)
        {
            var pipe = (NamedPipeServerStream) state.AsyncState;
            Console.WriteLine("Connected.");

            pipe.EndWaitForConnection(state);

            if(pipe.CanRead)
                ServerPipe.BeginRead(DataBuffer, 0, DataBuffer.Length, ReadCallback, this);
        }

        protected void WriteCallback(IAsyncResult state)
        {
            var pipe = (NamedPipeServerStream) state.AsyncState;
            try
            {
                pipe.EndWrite(state);
            }
            catch (IOException)
            {
                pipe.Close();
            }
        }

        protected void ReadCallback(IAsyncResult state)
        {
            ServerPipe.EndRead(state);
            if (!ServerPipe.IsConnected)
            {
                ServerPipe.Close();
                ClientPipe.Close();
                ServerPipe = CreatePipe(PipeDirection.In);
                ClientPipe = CreatePipe(PipeDirection.Out);
                ServerPipe.BeginWaitForConnection(ConnectionCallback, ServerPipe);
                ClientPipe.BeginWaitForConnection(ConnectionCallback, ClientPipe);
                return;
            }

            var msg = new RawNetMessage(DataBuffer, null, null, DeliveryMethod.ReliableOrdered);
            MessageHandler.Handle(ref msg);
            ServerPipe.BeginRead(DataBuffer, 0, DataBuffer.Length, ReadCallback, this);
        }

        public void Dispose()
        {
            Dispose(true);
        }

        protected virtual void Dispose(bool disposing)
        {
            ServerPipe.Disconnect();
            ServerPipe.Dispose();
        }

        protected NamedPipeServerStream CreatePipe(PipeDirection direction)
        {
            var pipe = new NamedPipeServerStream($"{PipeName}{direction}", direction, 254
                , PipeTransmissionMode.Message, PipeOptions.Asynchronous);

            return pipe;
        }
    }
}
