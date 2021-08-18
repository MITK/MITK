#include "QmitknnUNetToolGUI.h"
#include <QDir>
#include <QDirIterator>
#include <QMessageBox>
#include <QProcess>
#include <QStringListModel>
#include <QtGlobal>
#include <algorithm>

void QmitknnUNetToolGUI::EnableWidgets(bool enabled)
{
  Superclass::EnableWidgets(enabled);
  m_Controls.previewButton->setEnabled(enabled);
}

void QmitknnUNetToolGUI::ClearAllComboBoxes()
{
  m_Controls.modelBox->clear();
  m_Controls.taskBox->clear();
  m_Controls.foldBox->clear();
  m_Controls.trainerBox->clear();
}

template <typename T>
T QmitknnUNetToolGUI::FetchFoldersFromDir(const QString &path)
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

void QmitknnUNetToolGUI::OnDirectoryChanged(const QString &dir)
{
  this->ClearAllComboBoxes();
  auto models = FetchFoldersFromDir<QStringList>(dir);
  std::for_each(models.begin(), models.end(), [this](QString model) { m_Controls.modelBox->addItem(model); });
}

void QmitknnUNetToolGUI::OnModelChanged(const QString &text)
{
  m_ModelDirectory = m_Controls.modeldirectoryBox->directory();
  QString updatedPath(QDir::cleanPath(m_ModelDirectory + QDir::separator() + text));
  m_Controls.taskBox->clear();
  std::vector<QString> datasets = FetchFoldersFromDir<std::vector<QString>>(updatedPath);
  std::for_each(datasets.begin(), datasets.end(), [this](QString dataset) { m_Controls.taskBox->addItem(dataset); });
}

void QmitknnUNetToolGUI::OnTaskChanged(const QString &text)
{
  m_ModelDirectory = m_Controls.modeldirectoryBox->directory();

  QString updatedPath = QDir::cleanPath(m_ModelDirectory + QDir::separator() + m_Controls.modelBox->currentText() +
                                        QDir::separator() + text);
  m_Controls.trainerBox->clear();
  std::vector<QString> trainers = FetchFoldersFromDir<std::vector<QString>>(updatedPath);
  std::for_each(trainers.begin(), trainers.end(), [this](QString trainer) { m_Controls.trainerBox->addItem(trainer); });
}

void QmitknnUNetToolGUI::OnTrainerChanged(const QString &trainerSelected)
{
  m_Controls.foldBox->clear();
  if (m_Controls.modelBox->currentText() != "ensembles")
  {
    m_ModelDirectory = m_Controls.modeldirectoryBox->directory(); // check syntax
    QString updatedPath(QDir::cleanPath(m_ModelDirectory + QDir::separator() + m_Controls.modelBox->currentText() +
                                        QDir::separator() + m_Controls.taskBox->currentText() + QDir::separator() +
                                        trainerSelected));
    std::vector<QString> folds = FetchFoldersFromDir<std::vector<QString>>(updatedPath);
    std::for_each(folds.begin(), folds.end(), [this](QString fold) { m_Controls.foldBox->addItem(fold); });
  }
}

void QmitknnUNetToolGUI::OnPythonChanged(const QString &pyEnv)
{
  if (pyEnv == QString("Select"))
  {
    QString path =
      QFileDialog::getExistingDirectory(m_Controls.pythonEnvComboBox->parentWidget(), "Python Path", "dir");
    if (!path.isEmpty())
    {
      m_Controls.pythonEnvComboBox->insertItem(0, path);
      m_Controls.pythonEnvComboBox->setCurrentIndex(0);
    }
  }
}

void QmitknnUNetToolGUI::OnCheckBoxChanged(int state)
{
  bool visibility = false;
  if (state == Qt::Checked)
  {
    visibility = true;
  }
  ctkCheckBox *box = qobject_cast<ctkCheckBox *>(sender());
  if (box != nullptr)
  {
    if (box->objectName() == QString("nopipBox"))
    {
      m_Controls.codedirectoryBox->setVisible(visibility);
      m_Controls.nnUnetdirLabel->setVisible(visibility);
    }
    else if (box->objectName() == QString("multiModalBox"))
    {
      m_Controls.multiModalPath->setVisible(visibility);
      m_Controls.multiModalPathLabel->setVisible(visibility);
    }
  }
}

void QmitknnUNetToolGUI::AutoParsePythonPaths()
{
  QString homeDir = QDir::homePath();
  std::vector<QString> searchDirs;
#if defined(__APPLE__) || defined(MACOSX) || defined(linux) || defined(__linux__)
  // Add search locations for possible standard python paths here
  searchDirs.push_back(homeDir + QDir::separator() + "environments");
  searchDirs.push_back(homeDir + QDir::separator() + "anaconda3");
  searchDirs.push_back(homeDir + QDir::separator() + "miniconda3");
  searchDirs.push_back(homeDir + QDir::separator() + "opt" + QDir::separator() + "miniconda3");
  searchDirs.push_back(homeDir + QDir::separator() + "opt" + QDir::separator() + "anaconda3");
#elif defined(_WIN32) || defined(_WIN64)
  searchDirs.push_back("C:" + QDir::separator() + "anaconda3");
#endif
  for (QString searchDir : searchDirs)
  {
    if (searchDir.endsWith("anaconda3", Qt::CaseInsensitive))
    {
      if (QDir(searchDir).exists()) // Do case insensitive search
      {
        m_Controls.pythonEnvComboBox->insertItem(0, "(base): " + searchDir);
        searchDir.append((QDir::separator() + QString("envs")));
      }
    }
    for (QDirIterator subIt(searchDir, QDir::AllDirs, QDirIterator::NoIteratorFlags); subIt.hasNext();)
    {
      subIt.next();
      QString envName = subIt.fileName();
      if (!envName.startsWith('.')) // Filter out irrelevent hidden folders, if any.
      {                             // folds.push_back(fold);
        m_Controls.pythonEnvComboBox->insertItem(0, "(" + envName + "): " + subIt.filePath());
      }
    }
  }
  m_Controls.pythonEnvComboBox->setCurrentIndex(-1);
}

std::vector<std::string> QmitknnUNetToolGUI::FetchSelectedFoldsFromUI()
{
  std::vector<std::string> folds;
  if (!(m_Controls.foldBox->allChecked() || m_Controls.foldBox->noneChecked()))
  {
    QModelIndexList foldList = m_Controls.foldBox->checkedIndexes();
    foreach (QModelIndex index, foldList)
    {
      QString foldQString =
        m_Controls.foldBox->itemText(index.row()).split("_", QString::SplitBehavior::SkipEmptyParts).last();
      folds.push_back(foldQString.toStdString());
    }
  }
  return folds;
}

mitk::ModelParams QmitknnUNetToolGUI::MapToRequest(
  QString &modelName, QString &taskName, QString &trainer, QString &planId, std::vector<std::string> &folds)
{
  mitk::ModelParams requestObject;
  requestObject.model = modelName.toStdString();
  requestObject.trainer = trainer.toStdString();
  requestObject.planId = planId.toStdString();
  requestObject.task = taskName.toStdString();
  requestObject.folds = folds;
  return requestObject;
}