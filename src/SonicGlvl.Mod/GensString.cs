using System;
using System.Runtime.InteropServices;

namespace SonicGLvl
{
    [StructLayout(LayoutKind.Sequential)]
    public unsafe struct GensString
    {
        public IntPtr ValuePtr;
        public short Length => *((short*)(ValuePtr - 2));

        public string Value => Marshal.PtrToStringAnsi(ValuePtr);
    }
}
