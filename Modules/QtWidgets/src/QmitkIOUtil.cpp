/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "QmitkIOUtil.h"

#include "mitkCoreServices.h"
#include "mitkCustomMimeType.h"
#include "mitkFileReaderRegistry.h"
#include "mitkFileWriterRegistry.h"
#include "mitkIMimeTypeProvider.h"
#include "mitkMimeType.h"
#include <mitkCoreObjectFactory.h>
#include <mitkIOUtil.h>

#include "QmitkFileReaderOptionsDialog.h"
#include "QmitkFileWriterOptionsDialog.h"

// QT
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <QSet>

// ITK
#include <itksys/SystemTools.hxx>

#include <algorithm>

struct QmitkIOUtil::Impl
{
  struct ReaderOptionsDialogFunctor : public ReaderOptionsFunctorBase
  {
    bool operator()(LoadInfo &loadInfo) const override
    {
      QmitkFileReaderOptionsDialog dialog(loadInfo);
      if (dialog.exec() == QDialog::Accepted)
      {
        return !dialog.ReuseOptions();
      }
      else
      {
        loadInfo.m_Cancel = true;
        return true;
      }
    }
  };

  struct WriterOptionsDialogFunctor : public WriterOptionsFunctorBase
  {
    bool operator()(SaveInfo &saveInfo) const override
    {
      QmitkFileWriterOptionsDialog dialog(saveInfo);
      if (dialog.exec() == QDialog::Accepted)
      {
        return !dialog.ReuseOptions();
      }
      else
      {
        saveInfo.m_Cancel = true;
        return true;
      }
    }
  };
};

struct MimeTypeComparison : public std::unary_function<mitk::MimeType, bool>
{
  MimeTypeComparison(const std::string &mimeTypeName) : m_Name(mimeTypeName) {}
  bool operator()(const mitk::MimeType &mimeType) const { return mimeType.GetName() == m_Name; }
  const std::string m_Name;
};

QString QmitkIOUtil::GetFileOpenFilterString()
{
  QString filters;

  mitk::CoreServicePointer<mitk::IMimeTypeProvider> mimeTypeProvider(mitk::CoreServices::GetMimeTypeProvider());
  std::vector<std::string> categories = mimeTypeProvider->GetCategories();
  for (std::vector<std::string>::iterator cat = categories.begin(); cat != categories.end(); ++cat)
  {
    QSet<QString> filterExtensions;
    std::vector<mitk::MimeType> mimeTypes = mimeTypeProvider->GetMimeTypesForCategory(*cat);
    for (std::vector<mitk::MimeType>::iterator mt = mimeTypes.begin(); mt != mimeTypes.end(); ++mt)
    {
      std::vector<std::string> extensions = mt->GetExtensions();
      for (std::vector<std::string>::iterator ext = extensions.begin(); ext != extensions.end(); ++ext)
      {
        filterExtensions << QString::fromStdString(*ext);
      }
    }

    QString filter = QString::fromStdString(*cat) + " (";
    foreach (const QString &extension, filterExtensions)
    {
      filter += "*." + extension + " ";
    }
    filter = filter.replace(filter.size() - 1, 1, ')');
    filters += ";;" + filter;
  }
  filters.prepend("All (*)");
  return filters;
}

QList<mitk::BaseData::Pointer> QmitkIOUtil::Load(const QStringList &paths, QWidget *parent)
{
  std::vector<LoadInfo> loadInfos;
  foreach (const QString &file, paths)
  {
    loadInfos.push_back(LoadInfo(file.toLocal8Bit().constData()));
  }

  Impl::ReaderOptionsDialogFunctor optionsCallback;
  std::string errMsg = Load(loadInfos, nullptr, nullptr, &optionsCallback);
  if (!errMsg.empty())
  {
    QMessageBox::warning(parent, "Error reading files", QString::fromStdString(errMsg));
    mitkThrow() << errMsg;
  }

  QList<mitk::BaseData::Pointer> qResult;
  for (std::vector<LoadInfo>::const_iterator iter = loadInfos.begin(), iterEnd = loadInfos.end(); iter != iterEnd;
       ++iter)
  {
    for (const auto &elem : iter->m_Output)
    {
      qResult << elem;
    }
  }
  return qResult;
}

mitk::DataStorage::SetOfObjects::Pointer QmitkIOUtil::Load(const QStringList &paths,
                                                           mitk::DataStorage &storage,
                                                           QWidget *parent)
{
  std::vector<LoadInfo> loadInfos;
  foreach (const QString &file, paths)
  {
    loadInfos.push_back(LoadInfo(file.toLocal8Bit().constData()));
  }

  mitk::DataStorage::SetOfObjects::Pointer nodeResult = mitk::DataStorage::SetOfObjects::New();
  Impl::ReaderOptionsDialogFunctor optionsCallback;
  std::string errMsg = Load(loadInfos, nodeResult, &storage, &optionsCallback);
  if (!errMsg.empty())
  {
    QMessageBox::warning(parent, "Error reading files", QString::fromStdString(errMsg));
  }
  return nodeResult;
}

QList<mitk::BaseData::Pointer> QmitkIOUtil::Load(const QString &path, QWidget *parent)
{
  QStringList paths;
  paths << path;
  return Load(paths, parent);
}

mitk::DataStorage::SetOfObjects::Pointer QmitkIOUtil::Load(const QString &path,
                                                           mitk::DataStorage &storage,
                                                           QWidget *parent)
{
  QStringList paths;
  paths << path;
  return Load(paths, storage, parent);
}

QString QmitkIOUtil::Save(const mitk::BaseData *data,
                          const QString &defaultBaseName,
                          const QString &defaultPath,
                          QWidget *parent,
                          bool setPathProperty)
{
  std::vector<const mitk::BaseData *> dataVector;
  dataVector.push_back(data);
  QStringList defaultBaseNames;
  defaultBaseNames.push_back(defaultBaseName);
  return Save(dataVector, defaultBaseNames, defaultPath, parent, setPathProperty).back();
}

QStringList QmitkIOUtil::Save(const std::vector<const mitk::BaseData *> &data,
                              const QStringList &defaultBaseNames,
                              const QString &defaultPath,
                              QWidget *parent,
                              bool setPathProperty)
{
  QStringList fileNames;
  QString currentPath = defaultPath;

  std::vector<SaveInfo> saveInfos;

  int counter = 0;
  for (std::vector<const mitk::BaseData *>::const_iterator dataIter = data.begin(), dataIterEnd = data.end();
       dataIter != dataIterEnd;
       ++dataIter, ++counter)
  {
    SaveInfo saveInfo(*dataIter, mitk::MimeType(), std::string());

    SaveFilter filters(saveInfo);

    // If there is only the "__all__" filter string, it means there is no writer for this base data
    if (filters.Size() < 2)
    {
      QMessageBox::warning(
        parent,
        "Saving not possible",
        QString("No writer available for type \"%1\"").arg(QString::fromStdString((*dataIter)->GetNameOfClass())));
      continue;
    }

    // Construct a default path and file name
    QString filterString = filters.ToString();
    QString selectedFilter = filters.GetDefaultFilter();
    QString fileName = currentPath;
    QString dialogTitle = "Save " + QString::fromStdString((*dataIter)->GetNameOfClass());
    if (counter < defaultBaseNames.size())
    {
      dialogTitle += " \"" + defaultBaseNames[counter] + "\"";
      fileName += QDir::separator() + defaultBaseNames[counter];
      // We do not append an extension to the file name by default. The extension
      // is chosen by the user by either selecting a filter or writing the
      // extension in the file name himself (in the file save dialog).
      /*
      QString defaultExt = filters.GetDefaultExtension();
      if (!defaultExt.isEmpty())
      {
        fileName += "." + defaultExt;
      }
      */
    }

    // Ask the user for a file name
    QString nextName = QFileDialog::getSaveFileName(parent, dialogTitle, fileName, filterString, &selectedFilter);

    if (nextName.isEmpty())
    {
      // We stop asking for further file names, but we still save the
      // data where the user already confirmed the save dialog.
      break;
    }

    fileName = nextName;
    std::string stdFileName = fileName.toLocal8Bit().constData();
    QFileInfo fileInfo(fileName);
    currentPath = fileInfo.absolutePath();
    QString suffix = fileInfo.completeSuffix();
    mitk::MimeType filterMimeType = filters.GetMimeTypeForFilter(selectedFilter);
    mitk::MimeType selectedMimeType;

    if (fileInfo.exists() && !fileInfo.isFile())
    {
      QMessageBox::warning(parent, "Saving not possible", QString("The path \"%1\" is not a file").arg(fileName));
      continue;
    }

    // Theoretically, the user could have entered an extension that does not match the selected filter
    // The extension then has prioritry over the filter
    // Check if one of the available mime-types match the filename
    std::vector<mitk::MimeType> filterMimeTypes = filters.GetMimeTypes();
    for (std::vector<mitk::MimeType>::const_iterator mimeTypeIter = filterMimeTypes.begin(),
                                                     mimeTypeIterEnd = filterMimeTypes.end();
         mimeTypeIter != mimeTypeIterEnd;
         ++mimeTypeIter)
    {
      if (mimeTypeIter->MatchesExtension(stdFileName))
      {
        selectedMimeType = *mimeTypeIter;
        break;
      }
    }

    if (!selectedMimeType.IsValid())
    {
      // The file name either does not contain an extension or the
      // extension is unknown.

      // If the file already exists, we stop here because we are unable
      // to (over)write the file without adding a custom suffix. If the file
      // does not exist, we add the default extension from the currently
      // selected filter. If the "All" filter was selected, we only add the
      // default extensions if the file name itself does not already contain
      // an extension.
      if (!fileInfo.exists())
      {
        if (filterMimeType == SaveFilter::ALL_MIMETYPE())
        {
          if (suffix.isEmpty())
          {
            // Use the highest ranked mime-type from the list
            selectedMimeType = filters.GetDefaultMimeType();
          }
        }
        else
        {
          selectedMimeType = filterMimeType;
        }

        if (selectedMimeType.IsValid())
        {
          suffix = QString::fromStdString(selectedMimeType.GetExtensions().front());
          fileName += "." + suffix;
          stdFileName = fileName.toLocal8Bit().constData();
          // We changed the file name (added a suffix) so ask in case
          // the file aready exists.
          fileInfo = QFileInfo(fileName);
          if (fileInfo.exists())
          {
            if (!fileInfo.isFile())
            {
              QMessageBox::warning(
                parent, "Saving not possible", QString("The path \"%1\" is not a file").arg(fileName));
              continue;
            }
            if (QMessageBox::question(
                  parent,
                  "Replace File",
                  QString("A file named \"%1\" already exists. Do you want to replace it?").arg(fileName)) ==
                QMessageBox::No)
            {
              continue;
            }
          }
        }
      }
    }

    if (!selectedMimeType.IsValid())
    {
      // The extension/filename is not valid (no mime-type found), bail out
      QMessageBox::warning(
        parent, "Saving not possible", QString("No mime-type available which can handle \"%1\".").arg(fileName));
      continue;
    }

    if (!QFileInfo(fileInfo.absolutePath()).isWritable())
    {
      QMessageBox::warning(parent, "Saving not possible", QString("The path \"%1\" is not writable").arg(fileName));
      continue;
    }

    fileNames.push_back(fileName);
    saveInfo.m_Path = stdFileName;
    saveInfo.m_MimeType = selectedMimeType;
    // pre-select the best writer for the chosen mime-type
    saveInfo.m_WriterSelector.Select(selectedMimeType.GetName());
    saveInfos.push_back(saveInfo);
  }

  if (!saveInfos.empty())
  {
    Impl::WriterOptionsDialogFunctor optionsCallback;
    std::string errMsg = Save(saveInfos, &optionsCallback, setPathProperty);
    if (!errMsg.empty())
    {
      QMessageBox::warning(parent, "Error writing files", QString::fromStdString(errMsg));
      mitkThrow() << errMsg;
    }
  }

  return fileNames;
}

void QmitkIOUtil::SaveBaseDataWithDialog(mitk::BaseData *data, std::string fileName, QWidget * /*parent*/)
{
  Save(data, fileName);
}

void QmitkIOUtil::SaveSurfaceWithDialog(mitk::Surface::Pointer surface, std::string fileName, QWidget * /*parent*/)
{
  Save(surface, fileName);
}

void QmitkIOUtil::SaveImageWithDialog(mitk::Image::Pointer image, std::string fileName, QWidget * /*parent*/)
{
  Save(image, fileName);
}

void QmitkIOUtil::SavePointSetWithDialog(mitk::PointSet::Pointer pointset, std::string fileName, QWidget * /*parent*/)
{
  Save(pointset, fileName);
}

struct QmitkIOUtil::SaveFilter::Impl
{
  Impl(const mitk::IOUtil::SaveInfo &saveInfo) : m_SaveInfo(saveInfo)
  {
    // Add an artifical filter for "All"
    m_MimeTypes.push_back(ALL_MIMETYPE());
    m_FilterStrings.push_back("All (*.*)");

    // Get all writers and their mime types for the given base data type
    // (this is sorted already)
    std::vector<mitk::MimeType> mimeTypes = saveInfo.m_WriterSelector.GetMimeTypes();

    for (std::vector<mitk::MimeType>::const_reverse_iterator iter = mimeTypes.rbegin(), iterEnd = mimeTypes.rend();
         iter != iterEnd;
         ++iter)
    {
      QList<QString> filterExtensions;
      mitk::MimeType mimeType = *iter;
      std::vector<std::string> extensions = mimeType.GetExtensions();
      for (auto &extension : extensions)
      {
        filterExtensions << QString::fromStdString(extension);
      }
      if (m_DefaultExtension.isEmpty())
      {
        m_DefaultExtension = QString::fromStdString(extensions.front());
      }

      QString filter = QString::fromStdString(mimeType.GetComment()) + " (";
      foreach (const QString &extension, filterExtensions)
      {
        filter += "*." + extension + " ";
      }
      filter = filter.replace(filter.size() - 1, 1, ')');
      m_MimeTypes.push_back(mimeType);
      m_FilterStrings.push_back(filter);
    }
  }

  const mitk::IOUtil::SaveInfo m_SaveInfo;
  std::vector<mitk::MimeType> m_MimeTypes;
  QStringList m_FilterStrings;
  QString m_DefaultExtension;
};

mitk::MimeType QmitkIOUtil::SaveFilter::ALL_MIMETYPE()
{
  static mitk::CustomMimeType allMimeType(std::string("__all__"));
  return mitk::MimeType(allMimeType, -1, -1);
}

QmitkIOUtil::SaveFilter::SaveFilter(const QmitkIOUtil::SaveFilter &other) : d(new Impl(*other.d))
{
}

QmitkIOUtil::SaveFilter::SaveFilter(const SaveInfo &saveInfo) : d(new Impl(saveInfo))
{
}

QmitkIOUtil::SaveFilter &QmitkIOUtil::SaveFilter::operator=(const QmitkIOUtil::SaveFilter &other)
{
  d.reset(new Impl(*other.d));
  return *this;
}

std::vector<mitk::MimeType> QmitkIOUtil::SaveFilter::GetMimeTypes() const
{
  return d->m_MimeTypes;
}

QString QmitkIOUtil::SaveFilter::GetFilterForMimeType(const std::string &mimeType) const
{
  std::vector<mitk::MimeType>::const_iterator iter =
    std::find_if(d->m_MimeTypes.begin(), d->m_MimeTypes.end(), MimeTypeComparison(mimeType));
  if (iter == d->m_MimeTypes.end())
  {
    return QString();
  }
  int index = static_cast<int>(iter - d->m_MimeTypes.begin());
  if (index < 0 || index >= d->m_FilterStrings.size())
  {
    return QString();
  }
  return d->m_FilterStrings[index];
}

mitk::MimeType QmitkIOUtil::SaveFilter::GetMimeTypeForFilter(const QString &filter) const
{
  int index = d->m_FilterStrings.indexOf(filter);
  if (index < 0)
  {
    return mitk::MimeType();
  }
  return d->m_MimeTypes[index];
}

QString QmitkIOUtil::SaveFilter::GetDefaultFilter() const
{
  if (d->m_FilterStrings.size() > 1)
  {
    return d->m_FilterStrings.at(1);
  }
  else if (d->m_FilterStrings.size() > 0)
  {
    return d->m_FilterStrings.front();
  }
  return QString();
}

QString QmitkIOUtil::SaveFilter::GetDefaultExtension() const
{
  return d->m_DefaultExtension;
}

mitk::MimeType QmitkIOUtil::SaveFilter::GetDefaultMimeType() const
{
  if (d->m_MimeTypes.size() > 1)
  {
    return d->m_MimeTypes.at(1);
  }
  else if (d->m_MimeTypes.size() > 0)
  {
    return d->m_MimeTypes.front();
  }
  return mitk::MimeType();
}

QString QmitkIOUtil::SaveFilter::ToString() const
{
  return d->m_FilterStrings.join(";;");
}

int QmitkIOUtil::SaveFilter::Size() const
{
  return d->m_FilterStrings.size();
}

bool QmitkIOUtil::SaveFilter::IsEmpty() const
{
  return d->m_FilterStrings.isEmpty();
}

bool QmitkIOUtil::SaveFilter::ContainsMimeType(const std::string &mimeType)
{
  return std::find_if(d->m_MimeTypes.begin(), d->m_MimeTypes.end(), MimeTypeComparison(mimeType)) !=
         d->m_MimeTypes.end();
}
