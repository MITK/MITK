/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.s

============================================================================*/

#include "QmitknnUNetFolderParser.h"

QmitknnUNetFolderParser::QmitknnUNetFolderParser(const QString parentFolder)
{
  m_RootNode = std::make_shared<FolderNode>();
  m_RootNode->path = parentFolder;
  m_RootNode->name = QString("nnUNet");
  m_RootNode->subFolders.clear();
  InitDirs(m_RootNode, 0);
}

QString QmitknnUNetFolderParser::getResultsFolder()
{
  return m_RootNode->path;
}

std::function<bool(QString)> QmitknnUNetFolderParser::RuleEngine(int level)
{
  if (level == m_LEVEL - 1)
  {
    return [](QString path)
    {
      return (QFile::exists(path + QDir::separator() + QString("model_final_checkpoint.model")) &&
              QFile::exists(path + QDir::separator() + QString("model_final_checkpoint.model.pkl")) &&
              QFile::exists(path + QDir::separator() + QString("debug.json")));
    };
  }
  if (level == m_LEVEL - 2)
  {
    return [](QString path) { return QFile::exists(path + QDir::separator() + QString("plans.pkl")); };
  }
  else
  {
    return [](QString /*path*/) { return true; };
  }
}

std::shared_ptr<FolderNode> QmitknnUNetFolderParser::GetSubNodeMatchingNameCrietria(
  const QString &queryName, std::shared_ptr<FolderNode> parentNode)
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

void QmitknnUNetFolderParser::InitDirs(std::shared_ptr<FolderNode> parent, int level)
{
  QString searchFolder = parent->path + QDir::separator() + parent->name;
  auto rules = RuleEngine(level);
  auto subFolders = FetchFoldersFromDir<QStringList>(searchFolder, rules);
  level++;
  foreach (QString folder, subFolders)
  {
    std::shared_ptr<FolderNode> fp = std::make_shared<FolderNode>();
    fp->path = searchFolder;
    fp->name = folder;
    if (level < this->m_LEVEL)
    {
      InitDirs(fp, level);
    }
    parent->subFolders.push_back(fp);
  }
}

void QmitknnUNetFolderParser::DeleteDirs(std::shared_ptr<FolderNode> parent, int level)
{
  level++;
  for (std::shared_ptr<FolderNode> subFolder : parent->subFolders)
  {
    if (level < m_LEVEL)
    {
      DeleteDirs(subFolder, level);
    }
    parent->subFolders.clear();
  }
}
