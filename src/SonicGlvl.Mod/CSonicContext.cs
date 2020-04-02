using System.Numerics;
using System.Runtime.InteropServices;

namespace SonicGLvl
{
    [StructLayout(LayoutKind.Explicit)]
    public unsafe struct CSonicContext
    {
        [FieldOffset(0x10)]
        public CMatrixNodeTransform* Transform;

        [FieldOffset(0x110)]
        public CSonic* Sonic;
    }

    [StructLayout(LayoutKind.Explicit)]
    public unsafe struct CSonic
    {
        [FieldOffset(0x244)]
        public CAnimationStateMachine* AnimationState;
    }

    [StructLayout(LayoutKind.Explicit)]
    public unsafe struct CAnimationStateMachine
    {
        [FieldOffset(0x58)]
        public FrameData** FrameData;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct FrameData
    {
        public float Field_00;
        public float Field_04;
        public float Frame;
        public int Field_0C;
    }

    [StructLayout(LayoutKind.Explicit)]
    public struct CMatrixNodeTransform
    {
        [FieldOffset(0x60)]
        public Quaternion Rotation;

        [FieldOffset(0x70)]
        public Vector3 Position;
    }
}
