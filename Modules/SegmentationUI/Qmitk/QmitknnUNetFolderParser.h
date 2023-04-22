/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitknnUNetFolderParser_h
#define QmitknnUNetFolderParser_h

#include <QDirIterator>
#include <memory>
#include <functional>
#include <QString>
#include <vector>

/**
 * @brief Struct to store each (Folder) Node of the hierarchy tree structure.
 *
 */
struct FolderNode
{
  QString name;
  QString path; // parent
  std::vector<std::shared_ptr<FolderNode>> subFolders;
};

/**
 * @brief Class to store and retreive folder hierarchy information
 * of RESULTS_FOLDER. Only Root node is explicitly stored in m_RootNode.
 * No. of sub levels in the hierachry is defined in the LEVEL constant.
 *
 */
class QmitknnUNetFolderParser
{
public:
  /**
   * @brief Construct a new QmitknnUNetFolderParser object
   * Initializes root folder node object pointer calls
   * @param parentFolder
   */
  QmitknnUNetFolderParser(const QString parentFolder);
  /**
   * @brief Destroy the QmitknnUNetFolderParser object
   *
   */
  ~QmitknnUNetFolderParser() = default;
  /**
   * @brief Returns the "Results Folder" string which is parent path of the root node.
   *
   * @return QString
   */
  QString getResultsFolder(); 

  /**
   * @brief Returns the Model Names from root node. Template function,
   * type can be any of stl or Qt containers which supports push_back call.
   *
   * @tparam T
   * @return T (any of stl or Qt containers which supports push_back call)
   */
  template <typename T>
  T getModelNames()
  {
    auto models = GetSubFolderNamesFromNode<T>(m_RootNode);
    return models;
  }

  /**
   * @brief Returns the task names for a given model. Template function,
   * type can be any of stl or Qt containers which supports push_back call.
   *
   * @tparam T
   * @param modelName
   * @return T (any of stl or Qt containers which supports push_back call)
   */
  template <typename T>
  T getTasksForModel(const QString &modelName)
  {
    std::shared_ptr<FolderNode> modelNode = GetSubNodeMatchingNameCrietria(modelName, m_RootNode);
    auto tasks = GetSubFolderNamesFromNode<T>(modelNode);
    return tasks;
  }

  /**
   * @brief Returns the models names for a given task. Template function,
   * type can be any of stl or Qt containers which supports push_back call.
   *
   * @tparam T
   * @param taskName
   * @return T (any of stl or Qt containers which supports push_back call)
   */
  template <typename T>
  T getModelsForTask(const QString &taskName)
  {
    T modelsForTask;
    auto models = GetSubFolderNamesFromNode<T>(m_RootNode);
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

  /**
   * @brief Returns all the task names present in the root node with possible duplicates.
   * Template function, type can be any of stl or Qt containers which supports push_back call.
   *
   * @tparam T
   * @return T (any of stl or Qt containers which supports push_back call)
   */
  template <typename T>
  T getAllTasks()
  {
    T allTasks;
    auto models = GetSubFolderNamesFromNode<T>(m_RootNode);
    foreach (QString model, models)
    {
      allTasks << getTasksForModel<QStringList>(model);
    }
    return allTasks;
  }

  /**
   * @brief Returns the trainer / planner names for a given task & model. Template function,
   * type can be any of stl or Qt containers which supports push_back call.
   *
   * @tparam T
   * @param taskName
   * @param modelName
   * @return T (any of stl or Qt containers which supports push_back call)
   */
  template <typename T>
  T getTrainerPlannersForTask(const QString &taskName, const QString &modelName)
  {
    std::shared_ptr<FolderNode> modelNode = GetSubNodeMatchingNameCrietria(modelName, m_RootNode);
    std::shared_ptr<FolderNode> taskNode = GetSubNodeMatchingNameCrietria(taskName, modelNode);
    auto tps = GetSubFolderNamesFromNode<T>(taskNode);
    return tps;
  }

  /**
   * @brief Returns the Folds names for a given trainer,planner,task & model name. Template function,
   * type can be any of stl or Qt containers which supports push_back call.
   *
   * @tparam T
   * @param trainer
   * @param planner
   * @param taskName
   * @param modelName
   * @return T (any of stl or Qt containers which supports push_back call)
   */
  template <typename T>
  T getFoldsForTrainerPlanner(const QString &trainer,
                              const QString &planner,
                              const QString &taskName,
                              const QString &modelName)
  {
    std::shared_ptr<FolderNode> modelNode = GetSubNodeMatchingNameCrietria(modelName, m_RootNode);
    std::shared_ptr<FolderNode> taskNode = GetSubNodeMatchingNameCrietria(taskName, modelNode);
    QString trainerPlanner = trainer + QString("__") + planner;
    std::shared_ptr<FolderNode> tpNode = GetSubNodeMatchingNameCrietria(trainerPlanner, taskNode);
    auto folds = GetSubFolderNamesFromNode<T>(tpNode);
    return folds;
  }

private:
  const int m_LEVEL = 4;
  std::shared_ptr<FolderNode> m_RootNode;
  
  /**
   * @brief Returns rule function wrapper to check for specific files at given Result_Folder hierarchy level.
   * 
   * @param level 
   * @return std::function<bool(QString)> 
   */
  std::function<bool(QString)> RuleEngine(int level);

  /**
   * @brief Iterates through the root node and returns the sub FolderNode object Matching Name Crietria
   *
   * @param queryName
   * @param parentNode
   * @return std::shared_ptr<FolderNode>
   */
  std::shared_ptr<FolderNode> GetSubNodeMatchingNameCrietria(const QString &queryName, std::shared_ptr<FolderNode> parentNode);

  /**
   * @brief Returns the sub folder names for a folder node object. Template function,
   * type can be any of stl or Qt containers which supports push_back call.
   *
   * @tparam T
   * @param std::shared_ptr<FolderNode>
   * @return T (any of stl or Qt containers which supports push_back call)
   */
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

  /**
   * @brief Iterates through the sub folder hierarchy upto a level provided
   * and create a tree structure.
   *
   * @param parent
   * @param level
   */
  void InitDirs(std::shared_ptr<FolderNode> parent, int level);

  /**
   * @brief Iterates through the sub folder hierarchy upto a level provided
   * and clears the sub folder std::vector from each node.
   *
   * @param parent
   * @param level
   */
  void DeleteDirs(std::shared_ptr<FolderNode> parent, int level);

  /**
   * @brief Template function to fetch all folders inside a given path.
   * The type can be any of stl or Qt containers which supports push_back call.
   *
   * @tparam T
   * @param path
   * @return T
   */
  template <typename T>
  T FetchFoldersFromDir(const QString &path, std::function<bool(QString)> callback)
  {
    T folders;
    for (QDirIterator it(path, QDir::AllDirs, QDirIterator::NoIteratorFlags); it.hasNext();)
    {
      it.next();
      if (!it.fileName().startsWith('.') && callback(it.filePath()))
      {
        folders.push_back(it.fileName());
      }
    }
    return folders;
  }
};
#endif
