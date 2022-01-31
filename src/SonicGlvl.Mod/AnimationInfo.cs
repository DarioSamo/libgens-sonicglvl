using System;
using System.Runtime.InteropServices;

namespace SonicGLvl
{
    [StructLayout(LayoutKind.Sequential)]
    public struct AnimationInfo
    {
        private IntPtr vTable;
        public uint Field00;
        public uint Field04;
        public uint Field08;
        public ushort FieldA;
        public VisualState State;
        public IntPtr NamePtr;
        public float Frame;

        public string Name => Marshal.PtrToStringAnsi(NamePtr);
    }
}
