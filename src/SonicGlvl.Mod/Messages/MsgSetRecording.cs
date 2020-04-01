using System.Runtime.InteropServices;
using Reloaded.Messaging.Interfaces;
using Reloaded.Messaging.Serializer.ReloadedMemory;

namespace SonicGLvl.Messages
{
    [StructLayout(LayoutKind.Sequential)]
    public struct MsgSetRecording : IMessage<MessageTypes>
    {
        public MessageTypes GetMessageType() => MessageTypes.SetRecording;

        public ISerializer GetSerializer() => new ReloadedMemorySerializer(false);

        public ICompressor GetCompressor() => null;

        public bool Status;
    }
}
