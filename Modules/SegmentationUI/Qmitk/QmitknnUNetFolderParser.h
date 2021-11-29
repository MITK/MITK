/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.s

============================================================================*/

#ifndef QmitknnUNetFolderParser_h_Included
#define QmitknnUNetFolderParser_h_Included

#include "QmitknnUNetToolGUI.h"
#include <QDirIterator>
#include <QString>
#include <vector>

struct FolderNode
{
  QString name;
  QString path; // parent
  std::vector<std::shared_ptr<FolderNode>> subFolders;
};

class MITKSEGMENTATIONUI_EXPORT QmitknnUNetFolderParser
{
public:
  QmitknnUNetFolderParser(const QString parentFolder)
  {
    m_RootNode = std::make_shared<FolderNode>();
    m_RootNode->path = parentFolder;
    m_RootNode->name = QString("nnUNet");
    m_RootNode->subFolders.clear();
    RefreshHierarchy();
  }
  ~QmitknnUNetFolderParser() = default; /*{ DeleteDirs(m_RootNode, LEVEL); }*/

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
    std::shared_ptr<FolderNode> modelNode = GetSubNodeMatchingNameCrietria(modelName, m_RootNode);
    QStringList tasks = GetSubFolderNamesFromNode<T>(modelNode);
    return tasks;
  }

  template <typename T>
  T getModelsForTask(const QString &taskName)
  {
    QStringList modelsForTask;
    QStringList models = GetSubFolderNamesFromNode<T>(m_RootNode);
    foreach (QString model, models)
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
    std::shared_ptr<FolderNode> modelNode = GetSubNodeMatchingNameCrietria(modelName, m_RootNode);
    std::shared_ptr<FolderNode> taskNode = GetSubNodeMatchingNameCrietria(taskName, modelNode);
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
    std::shared_ptr<FolderNode> modelNode = GetSubNodeMatchingNameCrietria(modelName, m_RootNode);
    std::shared_ptr<FolderNode> taskNode = GetSubNodeMatchingNameCrietria(taskName, modelNode);
    QString trainerPlanner = trainer + QString("__") + planner;
    std::shared_ptr<FolderNode> tpNode = GetSubNodeMatchingNameCrietria(trainerPlanner, taskNode);
    folds = GetSubFolderNamesFromNode<T>(tpNode);
    return folds;
  }

private:
  const int LEVEL = 4;
  std::shared_ptr<FolderNode> m_RootNode;

  std::shared_ptr<FolderNode> GetSubNodeMatchingNameCrietria(const QString &queryName,
                                                             std::shared_ptr<FolderNode> parentNode)
  {
    std::shared_ptr<FolderNode> retNode;
    std::vector<std::shared_ptr<FolderNode>> subNodes = parentNode->subFolders;
    for (std::shared_ptr<FolderNode> node : subNodes)
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
  T GetSubFolderNamesFromNode(const std::shared_ptr<FolderNode> parent)
  {
    T folders;
    std::vector<std::shared_ptr<FolderNode>> subNodes = parent->subFolders;
    for (std::shared_ptr<FolderNode> folder : subNodes)
    {
      folders.push_back(folder->name);
    }
    return folders;
  }

  void InitDirs(std::shared_ptr<FolderNode> parent, int level)
  {
    QString searchFolder = parent->path + QDir::separator() + parent->name;
    auto subFolders = FetchFoldersFromDir<QStringList>(searchFolder);
    level++;
    foreach (QString folder, subFolders)
    {
      std::shared_ptr<FolderNode> fp = std::make_shared<FolderNode>();
      fp->path = searchFolder;
      fp->name = folder;
      if (level < this->LEVEL)
      {
        InitDirs(fp, level);
      }
      parent->subFolders.push_back(fp);
    }
  }

  void DeleteDirs(std::shared_ptr<FolderNode> parent, int level)
  {
    level++;
    for (std::shared_ptr<FolderNode> subFolder : parent->subFolders)
    {
      if (level < LEVEL)
      {
        DeleteDirs(subFolder, level);
      }
      parent->subFolders.clear();
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
