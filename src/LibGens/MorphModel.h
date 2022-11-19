#pragma once

namespace LibGens {
    class Mesh;
    class Vertex;
    class VertexFormat;

    class MorphTarget {
        protected:
            string name;
            vector<Vector3> vertices;

        public:
            void readDescription(File* file);
            void writeDescription(File* file);

            void read(File* file, size_t vertex_count);
            void write(File* file);
    };

    class MorphModel {
        protected:
            vector<Vertex*> vertices;
            vector<MorphTarget*> morphs;
            Mesh* mesh;
            VertexFormat* vertex_format;

        public:
            MorphModel();
            ~MorphModel();

            void fixVertexFormatForPC();

            Mesh* getMesh();

            void read(File* file, Topology topology);
            void write(File* file);
    };
}