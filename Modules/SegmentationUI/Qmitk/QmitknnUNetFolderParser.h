#ifndef QmitknnUNetFolderParser_h_Included
#define QmitknnUNetFolderParser_h_Included

#include "QmitknnUNetToolGUI.h"
#include <QDirIterator>
#include <QString>
#include <vector>

class FolderNode
{
public:
  QString name;
  QString path; // parent
  std::vector<FolderNode *> subFolders;
};

class MITKSEGMENTATIONUI_EXPORT QmitknnUNetFolderParser
{
public:
  QmitknnUNetFolderParser(const QString parentFolder)
  {
    m_RootNode = new FolderNode;
    m_RootNode->path = parentFolder;
    m_RootNode->name = QString("nnUNet");
    m_RootNode->subFolders.clear();
    RefreshHierarchy();
  }
  ~QmitknnUNetFolderParser() = default;

  void RefreshHierarchy() { InitDirs(m_RootNode, 0); }

  QString getResultsFolder() { return m_RootNode->path; }

  template <typename T>
  T getModelNames()
  {
    QStringList models = GetSubFolderNamesFromNode<T>(m_RootNode);
    return models;
  }

  template <typename T>
  T getTasksForModel(const QString &modelName)
  {
    FolderNode *modelNode = GetSubNodeMatchingNameCrietria(modelName, m_RootNode);
    QStringList tasks = GetSubFolderNamesFromNode<T>(modelNode);
    return tasks;
  }

  template <typename T>
  T getModelsForTask(const QString &taskName)
  {
    QStringList modelsForTask;
    QStringList models = GetSubFolderNamesFromNode<T>(m_RootNode);
    foreach(QString model, models)
    {
      QStringList taskList = getTasksForModel<QStringList>(model);
      if (taskList.contains(taskName, Qt::CaseInsensitive))
      {
        modelsForTask << model;
      }
    }
    return modelsForTask;
  }


  template <typename T>
  T getTrainerPlannersForTask(const QString &taskName, const QString &modelName)
  {
    QStringList tps;
    FolderNode *modelNode = GetSubNodeMatchingNameCrietria(modelName, m_RootNode);
    FolderNode *taskNode = GetSubNodeMatchingNameCrietria(taskName, modelNode);
    tps = GetSubFolderNamesFromNode<T>(taskNode);
    return tps;
  }

  template <typename T>
  T getFoldsForTrainerPlanner(const QString &trainer,
                                        const QString &planner,
                                        const QString &taskName,
                                        const QString &modelName)
  {
    QStringList folds;
    FolderNode *modelNode = GetSubNodeMatchingNameCrietria(modelName, m_RootNode);
    FolderNode *taskNode = GetSubNodeMatchingNameCrietria(taskName, modelNode);
    QString trainerPlanner = trainer + QString("__") + planner;
    FolderNode *tpNode = GetSubNodeMatchingNameCrietria(trainerPlanner, taskNode);
    folds = GetSubFolderNamesFromNode<T>(tpNode);
    return folds;
  }

private:
  const int LEVEL = 4;
  FolderNode *m_RootNode;

  FolderNode *GetSubNodeMatchingNameCrietria(const QString &queryName, FolderNode *parentNode)
  {
    FolderNode *retNode;
    std::vector<FolderNode *> subNodes = parentNode->subFolders;
    for (FolderNode *node : subNodes)
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
  T GetSubFolderNamesFromNode(const FolderNode *parent)
  {
    T folders;
    std::vector<FolderNode *> subNodes = parent->subFolders;
    for (FolderNode *folder : subNodes)
    {
      folders.push_back(folder->name);
    }
    return folders;
  }

  void InitDirs(FolderNode *parent, int level)
  {
    QString searchFolder = parent->path + QDir::separator() + parent->name;
    auto subFolders = FetchFoldersFromDir<QStringList>(searchFolder);
    level++;
    foreach (QString folder, subFolders)
    {
      FolderNode *fp = new FolderNode;
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
  T FetchFoldersFromDir(const QString &path)
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
};
#endif
