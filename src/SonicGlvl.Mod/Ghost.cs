using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Numerics;
using AmicitiaLibrary.IO;

namespace SonicGLvl
{
    public class Ghost
    {
        public List<GhostNode> Nodes = new List<GhostNode>();

        public void Read(Stream stream)
        {
            var reader = new EndianBinaryReader(stream, Endianness.BigEndian);
            var animCount = reader.ReadInt32();
            var nodeCount = reader.ReadInt32();
            var animations = reader.ReadStrings(animCount, StringBinaryFormat.FixedLength, 32);
            Nodes = new List<GhostNode>();
            for (int i = 0; i < nodeCount; i++)
            {
                var node = new GhostNode();
                node.Read(reader, animations);
                Nodes.Add(node);
            }
        }

        public void Write(Stream stream)
        {
            var writer = new EndianBinaryWriter(stream, Endianness.BigEndian);
            var animations = Nodes.Select(x => x.Animation).Distinct().ToList();

            writer.Write(animations.Count);
            writer.Write(Nodes.Count);
            animations.ForEach(anim => writer.Write(anim, StringBinaryFormat.FixedLength, 32));

            foreach (var node in Nodes)
            {
                node.Write(writer, animations);
            }
        }
    }

    public class GhostNode
    {
        public float Time;
        public Vector3 Position;
        public Quaternion Rotation;
        public VisualState State;
        public string Animation;
        public float Frame;

        public void Read(EndianBinaryReader reader, IList<string> animations)
        {
            Time = reader.ReadSingle();
            Position = new Vector3(reader.ReadSingle(), reader.ReadSingle(), reader.ReadSingle());
            Rotation = new Quaternion(reader.ReadSingle(), reader.ReadSingle(), reader.ReadSingle(), reader.ReadSingle());
            Animation = animations[reader.ReadInt16()];
            State = (VisualState)reader.ReadUInt16();
            Frame = reader.ReadSingle();
        }

        public void Write(EndianBinaryWriter writer, IList<string> animations)
        {
            writer.Write(Time);

            writer.Write(Position.X);
            writer.Write(Position.Y);
            writer.Write(Position.Z);

            writer.Write(Rotation.X);
            writer.Write(Rotation.Y);
            writer.Write(Rotation.Z);
            writer.Write(Rotation.W);

            writer.Write((short)animations.IndexOf(Animation));
            writer.Write((ushort)State);
            writer.Write(Frame);
        }
    }

    public enum VisualState : ushort
    {
        Normal,
        Ball,
        Board
    }
}
