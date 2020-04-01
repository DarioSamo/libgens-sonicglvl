using System;
using System.Runtime.InteropServices;
using Reloaded.Messaging.Interfaces;
using Reloaded.Messaging.Serializer.ReloadedMemory;

namespace SonicGLvl.Messages
{
    [StructLayout(LayoutKind.Sequential)]
    public unsafe struct MsgSaveRecording : IMessage<MessageTypes>
    {
        public MessageTypes GetMessageType() => MessageTypes.SaveRecording;

        public ISerializer GetSerializer() => new ReloadedMemorySerializer(false);

        public ICompressor GetCompressor() => null;

        public fixed byte FilePath[260];

        public string Path
        {
            get
            {
                fixed (byte* ptr = FilePath)
                    return Marshal.PtrToStringAnsi((IntPtr)ptr);
            }

            set
            {
                int i = 0;
                fixed (char* ptr = value)
                {
                    for (; i < value.Length; i++)
                    {
                        FilePath[i] = (byte)ptr[i];
                    }

                    FilePath[i] = 0;
                }
            }
        }
    }
}
