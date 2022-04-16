#pragma once

namespace LibGens {
	class ArchiveFile {
	protected:
		string file_path;
	public:
		ArchiveFile();
		~ArchiveFile();

		const string& getPath() const { return file_path; }
	};

	class ArchiveTreeNode {
	protected:
		string name;
		string archive;
		string def_append;
		list<ArchiveTreeNode *> append_nodes;
	public:
		ArchiveTreeNode();
		~ArchiveTreeNode();
	};

	class ArchiveTree {
	protected:
		list<ArchiveTreeNode *> nodes;
	public:
		ArchiveTree();
		~ArchiveTree();

		void load(string filename);
	};

	class ArchiveSystem {
	protected:
		list<ArchiveFile *> files;
	public:
		ArchiveSystem();
		~ArchiveSystem();

		void buildFromTreeNode(ArchiveTree *tree, string node_name);
		ArchiveFile *findFile(string filename) const;
	};
};