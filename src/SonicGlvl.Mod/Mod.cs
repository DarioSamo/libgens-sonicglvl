using System;
using System.IO;
using System.Runtime.InteropServices;
using Reloaded.Hooks;
using Reloaded.Hooks.Definitions;
using Reloaded.Hooks.Definitions.X86;
using Reloaded.Hooks.X86;
using Reloaded.Messaging.Messages;
using Reloaded.Messaging.Structs;
using SonicGLvl.Messages;

namespace SonicGLvl
{
    class Mod
    {
        [Function(CallingConventions.MicrosoftThiscall)]
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public unsafe delegate void UpdatePlayerSpeed(IntPtr mThis, float* anim);

        [Function(CallingConventions.MicrosoftThiscall)]
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public unsafe delegate IntPtr MsgGetAnimationInfoDef(CSonic* cSonicPtr, ref AnimationInfo data);

        public static IHook<UpdatePlayerSpeed> UpdatePlayerSpeedHook;
        public static unsafe CSonicContext** SonicContext = (CSonicContext**)0x01E5E2F0;
        public static MsgGetAnimationInfoDef MsgGetAnimationInfo = Wrapper.Create<MsgGetAnimationInfoDef>(0xE6A370);

        public static unsafe void** GameDocument = (void**) 0x01E0BE5C;
        public static Ghost Ghost = new Ghost();
        public static bool IsRecording;
        public static PipeHost<MessageTypes> Server;

        public static unsafe void Init()
        {
            UpdatePlayerSpeedHook = new Hook<UpdatePlayerSpeed>(UpdatePlayerSpeedImpl, 0xE6BF20).Activate();
            Server = new PipeHost<MessageTypes>("Hedgehog").Start();
            Server.MessageHandler.AddOrOverrideHandler(MessageTypes.SetRecording,
                (ref NetMessage<MsgSetRecording> message) => 
                { 
                    IsRecording = message.Message.Status;
                    Server.SendMessage(new Message<MessageTypes,MsgSetRecording>(new MsgSetRecording() { Status = message.Message.Status}));
                });
            Server.MessageHandler.AddOrOverrideHandler<MsgSaveRecording>(MessageTypes.SaveRecording, ProcessMessageSaveRecording);
        }

        public static void ProcessMessageSaveRecording(ref NetMessage<MsgSaveRecording> msg)
        {
            var path = msg.Message.Path;
            if (string.IsNullOrEmpty(path))
                path = Path.GetTempFileName();

            IsRecording = false;
            using (var stream = File.Create(path))
            {
                Ghost.Write(stream);
            }

            Ghost.Nodes.Clear();
            Server.SendMessage(new Message<MessageTypes, MsgSaveRecording>(new MsgSaveRecording() { Path = path }));
        }

        public static void OnFrame()
        {

        }

        public static unsafe void UpdatePlayerSpeedImpl(IntPtr mThis, float* deltaTime)
        {
            if (*SonicContext != null && IsRecording)
            {
                var context = (*SonicContext);
                var info = new AnimationInfo();
                MsgGetAnimationInfo(context->Sonic, ref info);
                Ghost.Nodes.Add(new GhostNode()
                {
                    Animation = info.Name,
                    State = info.State,
                    Frame = info.Frame,
                    Position = context->Transform->Position,
                    Rotation = context->Transform->Rotation,
                    Time = *deltaTime
                });
            }

            UpdatePlayerSpeedHook.OriginalFunction(mThis, deltaTime);
        }
    }
}
