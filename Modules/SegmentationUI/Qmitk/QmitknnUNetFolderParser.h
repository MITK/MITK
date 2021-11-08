#ifndef QmitknnUNetFolderParser_h_Included
#define QmitknnUNetFolderParser_h_Included

#include <QString>
#include <vector>
#include <QDirIterator>
#include "QmitknnUNetToolGUI.h"


class FolderNode
{
public:
  QString name;
  QString path; //parent
  std::vector<FolderNode*> subFolders;
};

class MITKSEGMENTATIONUI_EXPORT QmitknnUNetFolderParser
{
public:
  QmitknnUNetFolderParser(const QString parentFolder)
  {
    m_RootNode = new FolderNode;
    RefreshHierarchy(parentFolder);
  }
  ~QmitknnUNetFolderParser() = default;

  void RefreshHierarchy(const QString parentFolder)
  {
    m_RootNode->path = parentFolder;
    m_RootNode->name = QString("nnUNet");
    m_RootNode->subFolders.clear();
    InitDirs(m_RootNode, 0);
  }

  void InitDirs(FolderNode* parent, int level)
  {
    QString searchFolder = parent->path + QDir::separator() + parent->name;
    auto subFolders = FetchFoldersFromDir<QStringList>(searchFolder);
    level++;
    foreach(QString folder, subFolders)
    {
      FolderNode* fp = new FolderNode;
      fp->path = searchFolder;
      fp->name = folder;
      if (level < this->LEVEL)
      {
        InitDirs(fp, level);
      }
      parent->subFolders.push_back(fp);
    }
  }

  template <typename T>
  T FetchFoldersFromDir(const QString& path)
  {
    T folders;
    for (QDirIterator it(path, QDir::AllDirs, QDirIterator::NoIteratorFlags); it.hasNext();)
    {
      it.next();
      if (!it.fileName().startsWith('.'))
      {
        folders.push_back(it.fileName());
      }
    }
    return folders;
  }

  QStringList getModelNames()
  {
    QStringList models = GetSubFolderNamesFromNode<QStringList>(m_RootNode);
    return models;
  }

  QStringList getTasksForModel(const QString& modelName)
  {
    FolderNode* modelNode = GetSubNodeMatchingNameCrietria(modelName, m_RootNode);
    QStringList tasks = GetSubFolderNamesFromNode<QStringList>(modelNode);
    return tasks;
  }

  FolderNode* GetSubNodeMatchingNameCrietria(const QString& queryName, FolderNode* parentNode)
  {
    FolderNode* retNode;
    std::vector<FolderNode*> subNodes = parentNode->subFolders;
    for (FolderNode* node : subNodes)
    {
      if (node->name == queryName)
      {
        retNode = node;
        break;
      }
    }
    return retNode;
  }

  template <typename T>
  T GetSubFolderNamesFromNode(const FolderNode* parent)
  {
    T folders;
    std::vector<FolderNode*> subNodes = parent->subFolders;
    for (FolderNode* folder : subNodes)
    {
      folders.push_back(folder->name);
    }
    return folders;
  }

  QStringList getTrainerPlannersForTask(const QString& taskName, const QString& modelName)
  {
    QStringList tps;
    FolderNode* modelNode = GetSubNodeMatchingNameCrietria(modelName, m_RootNode);
    FolderNode* taskNode = GetSubNodeMatchingNameCrietria(taskName, modelNode);
    tps = GetSubFolderNamesFromNode<QStringList>(taskNode);
    return tps;
  }

  QStringList getFoldsForTrainerPlanner(const QString& trainer, const QString& planner, const QString& taskName, const QString& modelName)
  {
    QStringList folds;
    FolderNode* modelNode = GetSubNodeMatchingNameCrietria(modelName, m_RootNode);
    FolderNode* taskNode = GetSubNodeMatchingNameCrietria(taskName, modelNode);
    QString trainerPlanner = trainer + QString("__") + planner;
    FolderNode* tpNode = GetSubNodeMatchingNameCrietria(trainerPlanner, taskNode);
    folds = GetSubFolderNamesFromNode<QStringList>(tpNode);
    return folds;
  }

private:
  const int LEVEL = 4;
  FolderNode* m_RootNode;
};
#endif


