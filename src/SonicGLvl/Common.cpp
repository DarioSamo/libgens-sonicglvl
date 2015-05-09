//=============================================================================================================
// Copied/Modified from the Ogre3D Forums: http://www.ogre3d.org/forums/viewtopic.php?f=2&t=53647&start=0
//=============================================================================================================

#include "Common.h"

void destroyAllAttachedMovableObjects(Ogre::SceneNode* node, bool unload_resource) {
   if (!node) return;

   Ogre::SceneNode::ObjectIterator itObject = node->getAttachedObjectIterator();

   while (itObject.hasMoreElements()) {
		Ogre::MovableObject *movable_object=itObject.getNext();
		node->getCreator()->destroyMovableObject(movable_object);
   }

   Ogre::SceneNode::ChildNodeIterator itChild = node->getChildIterator();

   while (itChild.hasMoreElements()) {
      Ogre::SceneNode* pChildNode = static_cast<Ogre::SceneNode*>(itChild.getNext());
      destroyAllAttachedMovableObjects( pChildNode, unload_resource);
   }
}

void destroySceneNode(Ogre::SceneNode* node, bool unload_resource) {
   if (!node) return;
   destroyAllAttachedMovableObjects(node, unload_resource);
   node->removeAndDestroyAllChildren();
   node->getCreator()->destroySceneNode(node);
}