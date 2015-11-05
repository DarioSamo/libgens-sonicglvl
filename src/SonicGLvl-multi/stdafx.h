#define SONICGLVL_INDENT  4

#include <QApplication>
#include <QMainWindow>
#include <QWidget>
#include <QGridLayout>
#include <QElapsedTimer>
#include <QMouseEvent>
#include <QString>
#include <QStringList>
#include <QDomDocument>
#include <QTextStream>
#include <QDir>
#include <QMessageBox>
#include <QStyle>
#include <QDesktopWidget>
#include <QStyleFactory>
#include <QFileDialog>
#include <QProgressDialog>
#include <QTreeWidget>
#include <OGRE/Ogre.h>
#include <OGRE/OgreRenderWindow.h>
#include <OGRE/OgreCamera.h>
#include <OGRE/OgreResourceManager.h>
#include <OGRE/OgreResourceGroupManager.h>
#include <OGRE/OgreSceneManager.h>
#include <string>

#define MsgBox(x) QMessageBox::about(NULL, "Debug", QString("%1").arg(x))
