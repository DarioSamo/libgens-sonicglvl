#include "ArchiveTree.h"

namespace LibGens {
	ArchiveFile::ArchiveFile() {
	}

	ArchiveFile::~ArchiveFile() {
	}

	ArchiveTreeNode::ArchiveTreeNode() {
		name = "Unnamed";
		archive = "UnnamedArchive";
		def_append = "Unnamed";
	}

	ArchiveTreeNode::~ArchiveTreeNode() {
	}

	ArchiveTree::ArchiveTree() {

	}

	ArchiveTree::~ArchiveTree() {

	}

	void ArchiveTree::load(string filename) {
		TiXmlDocument doc(filename);
		doc.LoadFile();

		TiXmlHandle hDoc(&doc);
		TiXmlHandle hRoot(0);
		TiXmlElement *pElem=hDoc.FirstChildElement().Element();
		for(pElem; pElem; pElem=pElem->NextSiblingElement()) {
			string entry_name = pElem->ValueStr();
			printf("%s\n", entry_name.c_str());
		}
	}


	ArchiveSystem::ArchiveSystem() {
	}

	ArchiveSystem::~ArchiveSystem() {
	}

	void ArchiveSystem::buildFromTreeNode(ArchiveTree *tree, string node_name) {
	}

	ArchiveFile *ArchiveSystem::findFile(string filename) const {

		for (auto file = files.begin(); file != files.end(); ++file) {
			if ((*file)->getPath() == filename)
				return *file;
		}

		return nullptr;
	}
}