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

#include <mitkIOUtil.h>
#include <mitkImageWriter.h>
#include <mitkCoreObjectFactory.h>
#include "mitkCoreServices.h"
#include "mitkIMimeTypeProvider.h"
#include "mitkFileReaderRegistry.h"
#include "mitkFileWriterRegistry.h"

#include "QmitkFileReaderOptionsDialog.h"
#include "QmitkFileWriterOptionsDialog.h"

// QT
#include <QFileDialog>
#include <QMessageBox>
#include <QSet>

//ITK
#include <itksys/SystemTools.hxx>

struct QmitkIOUtil::Impl
{
  struct ReaderOptionsDialogFunctor : public ReaderOptionsFunctorBase
  {

    virtual bool operator()(const std::string& path, const std::vector<std::string>& readerLabels,
                            const std::vector<mitk::IFileReader*>& readers, mitk::IFileReader*& selectedReader)
    {
      QStringList qLabels;
      for (std::vector<std::string>::const_iterator iter = readerLabels.begin(), iterEnd = readerLabels.end();
           iter != iterEnd; ++iter)
      {
        qLabels.push_back(QString::fromStdString(*iter));
      }
      QList<mitk::IFileReader*> qReaders;
      for (std::vector<mitk::IFileReader*>::const_iterator iter = readers.begin(), iterEnd = readers.end();
           iter != iterEnd; ++iter)
      {
        qReaders.push_back(*iter);
      }

      QmitkFileReaderOptionsDialog dialog(QString::fromStdString(path), qLabels, qReaders);
      if (dialog.exec() == QDialog::Accepted)
      {
        selectedReader = dialog.GetReader();
        return !dialog.ReuseOptions();
      }
      else
      {
        selectedReader = NULL;
        return true;
      }
    }
  };

  struct WriterOptionsDialogFunctor : public WriterOptionsFunctorBase
  {

    virtual bool operator()(const std::string& path, const std::vector<std::string>& writerLabels,
                            const std::vector<mitk::IFileWriter*>& writers, mitk::IFileWriter*& selectedWriter)
    {
      QStringList qLabels;
      for (std::vector<std::string>::const_iterator iter = writerLabels.begin(), iterEnd = writerLabels.end();
           iter != iterEnd; ++iter)
      {
        qLabels.push_back(QString::fromStdString(*iter));
      }
      QList<mitk::IFileWriter*> qWriters;
      for (std::vector<mitk::IFileWriter*>::const_iterator iter = writers.begin(), iterEnd = writers.end();
           iter != iterEnd; ++iter)
      {
        qWriters.push_back(*iter);
      }

      QmitkFileWriterOptionsDialog dialog(QString::fromStdString(path), qLabels, qWriters);
      if (dialog.exec() == QDialog::Accepted)
      {
        selectedWriter = dialog.GetWriter();
        return !dialog.ReuseOptions();
      }
      else
      {
        selectedWriter = NULL;
        return true;
      }
    }
  };

};

QString QmitkIOUtil::GetFileOpenFilterString()
{
  QString filters;

  mitk::CoreServicePointer<mitk::IMimeTypeProvider> mimeTypeProvider(mitk::CoreServices::GetMimeTypeProvider());
  std::vector<std::string> categories = mimeTypeProvider->GetCategories();
  for (std::vector<std::string>::iterator cat = categories.begin(); cat != categories.end(); ++cat)
  {
    QSet<QString> filterExtensions;
    std::vector<std::string> mimeTypes = mimeTypeProvider->GetMimeTypesForCategory(*cat);
    for (std::vector<std::string>::iterator mt = mimeTypes.begin(); mt != mimeTypes.end(); ++mt)
    {
      std::vector<std::string> extensions = mimeTypeProvider->GetExtensions(*mt);
      for (std::vector<std::string>::iterator ext = extensions.begin(); ext != extensions.end(); ++ext)
      {
        filterExtensions << QString::fromStdString(*ext);
      }
    }

    QString filter = QString::fromStdString(*cat) + " (";
    foreach(const QString& extension, filterExtensions)
    {
      filter += "*." + extension + " ";
    }
    filter = filter.replace(filter.size()-1, 1, ')');
    filters += ";;" + filter;
  }
  filters.prepend("All (*.*)");
  return filters;
}

QmitkIOUtil::SaveFilter QmitkIOUtil::GetFileSaveFilter(const mitk::BaseData* baseData)
{
  return SaveFilter(baseData);
}

QmitkIOUtil::SaveFilter QmitkIOUtil::GetFileSaveFilter(const std::string& baseDataType)
{
  return SaveFilter(baseDataType);
}

QList<mitk::BaseData::Pointer> QmitkIOUtil::Load(const QStringList& paths, QWidget* parent)
{
  std::vector<std::string> qPaths;
  foreach(const QString& file, paths)
  {
    qPaths.push_back(file.toStdString());
  }

  std::vector<mitk::BaseData::Pointer> result;
  Impl::ReaderOptionsDialogFunctor optionsCallback;
  std::string errMsg = Load(qPaths, &result, NULL, NULL, &optionsCallback);
  if (!errMsg.empty())
  {
    QMessageBox::warning(parent, "Error reading files", QString::fromStdString(errMsg));
    mitkThrow() << errMsg;
  }

  QList<mitk::BaseData::Pointer> qResult;
  for(std::vector<mitk::BaseData::Pointer>::const_iterator iter = result.begin(),
      iterEnd = result.end(); iter != iterEnd; ++iter)
  {
    qResult << *iter;
  }
  return qResult;
}

mitk::DataStorage::SetOfObjects::Pointer QmitkIOUtil::Load(const QStringList& paths, mitk::DataStorage& storage, QWidget* parent)
{
  std::vector<std::string> qPaths;
  foreach(const QString& file, paths)
  {
    qPaths.push_back(file.toStdString());
  }

  mitk::DataStorage::SetOfObjects::Pointer nodeResult = mitk::DataStorage::SetOfObjects::New();
  Impl::ReaderOptionsDialogFunctor optionsCallback;
  std::string errMsg = Load(qPaths, NULL, nodeResult, &storage, &optionsCallback);
  if (!errMsg.empty())
  {
    QMessageBox::warning(parent, "Error reading files", QString::fromStdString(errMsg));
    mitkThrow() << errMsg;
  }
  return nodeResult;
}

QList<mitk::BaseData::Pointer> QmitkIOUtil::Load(const QString& path, QWidget* parent)
{
  QStringList paths;
  paths << path;
  return Load(paths, parent);
}

mitk::DataStorage::SetOfObjects::Pointer QmitkIOUtil::Load(const QString& path, mitk::DataStorage& storage,
                                                           QWidget* parent)
{
  QStringList paths;
  paths << path;
  return Load(paths, storage, parent);
}

QString QmitkIOUtil::Save(const mitk::BaseData* data, const QString& defaultBaseName,
                          const QString& defaultPath, QWidget* parent)
{
  std::vector<const mitk::BaseData*> dataVector;
  dataVector.push_back(data);
  QStringList defaultBaseNames;
  defaultBaseNames.push_back(defaultBaseName);
  return Save(dataVector, defaultBaseNames, defaultPath, parent).back();
}

QStringList QmitkIOUtil::Save(const std::vector<const mitk::BaseData*>& data,
                              const QStringList& defaultBaseNames,
                              const QString& defaultPath,
                              QWidget* parent)
{
  QStringList fileNames;
  QString currentPath = defaultPath;

  std::vector<SaveInfo> saveInfos;

  int counter = 0;
  for(std::vector<const mitk::BaseData*>::const_iterator dataIter = data.begin(),
      dataIterEnd = data.end(); dataIter != dataIterEnd; ++dataIter, ++counter)
  {
    SaveFilter filters(*dataIter);

    // If there is only the "__all__" filter string, it means there is no writer for this base data
    if (filters.Size() < 2)
    {
      QMessageBox::warning(parent,
                           "Saving not possible",
                           QString("No writer available for type \"%1\"").arg(
                             QString::fromStdString((*dataIter)->GetNameOfClass())));
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
      std::pair<std::string,std::string> defaultExt = mitk::FileWriterRegistry::GetDefaultExtension(*dataIter);
      if (!defaultExt.first.empty())
      {
        fileName += "." + QString::fromStdString(defaultExt.first);
        QString  filter = filters.GetFilterForMimeType(defaultExt.second);
        if (!filter.isEmpty())
        {
          selectedFilter = filter;
        }
      }
    }

    // Ask the user for a file name
    QString nextName =  QFileDialog::getSaveFileName(parent,
                                                     dialogTitle,
                                                     fileName,
                                                     filterString,
                                                     &selectedFilter);

    if (nextName.isEmpty())
    {
      continue;
    }

    fileName = nextName;
    QFileInfo fileInfo(fileName);
    currentPath = fileInfo.absolutePath();
    QString suffix = fileInfo.completeSuffix();
    std::string mimeType = filters.GetMimeTypeForFilter(selectedFilter);

    mitk::CoreServicePointer<mitk::IMimeTypeProvider> mimeTypeProvider(mitk::CoreServices::GetMimeTypeProvider());
    // If the filename contains a suffix, use it but check if it is valid
    if (!suffix.isEmpty())
    {
      std::vector<std::string> availableTypes = mimeTypeProvider->GetMimeTypesForExtension(suffix.toStdString());

      // Check if the selected mime-type is related to the specified suffix (file extension).
      // If not, get the best matching mime-type for the suffix.
      if (std::find(availableTypes.begin(), availableTypes.end(), mimeType) == availableTypes.end())
      {
        mimeType.clear();
        for (std::vector<std::string>::const_iterator availIter = availableTypes.begin(),
             availIterEnd = availableTypes.end(); availIter != availIterEnd; ++availIter)
        {
          if (filters.ContainsMimeType(*availIter))
          {
            mimeType = *availIter;
            break;
          }
        }
      }

      if (mimeType.empty())
      {
        // The extension is not valid (no mime-type found), bail out
        QMessageBox::warning(parent,
                             "Saving not possible",
                             QString("Extension \"%1\" unknown for type \"%2\"")
                             .arg(suffix)
                             .arg(QString::fromStdString((*dataIter)->GetNameOfClass())));
        continue;
      }
    }
    else
    {
      // Create a default suffix, unless the file already exists and the user
      // already confirmed to overwrite it (without using a suffix)
      if (mimeType == SaveFilter::ALL_MIMETYPE)
      {
        // Use the highest ranked mime-type from the list
        mimeType = filters.GetDefaultMimeType();
      }
      if (!fileInfo.exists())
      {
        suffix = QString::fromStdString(
                   mimeTypeProvider->GetExtensions(mimeType).front());
        fileName += "." + suffix;
        // We changed the file name (added a suffix) so ask in case
        // the file aready exists.
        fileInfo = QFileInfo(fileName);
        if (fileInfo.exists())
        {
          if (!fileInfo.isFile())
          {
            QMessageBox::warning(parent, "Saving not possible", QString("The path \"%1\" is not a file").arg(fileName));
            continue;
          }
          if (fileInfo.exists())
          {
            if (QMessageBox::question(parent, "Replace File",
                                      "A file named \"%1\" already exists. Do you want to replace it?") ==
                QMessageBox::No)
            {
              continue;
            }
          }
        }
      }
    }

    if (!QFileInfo(fileInfo.absolutePath()).isWritable())
    {
      QMessageBox::warning(parent, "Saving not possible", QString("The path \"%1\" is not writable").arg(fileName));
      continue;
    }

    fileNames.push_back(fileName);
    saveInfos.push_back(SaveInfo(*dataIter, mimeType, fileName.toStdString()));
    MITK_INFO << "****** SAVING TO FILENAME: " << fileName.toStdString();
  }

  Impl::WriterOptionsDialogFunctor optionsCallback;
  std::string errMsg = Save(saveInfos, &optionsCallback);
  if (!errMsg.empty())
  {
    QMessageBox::warning(parent, "Error writing files", QString::fromStdString(errMsg));
    mitkThrow() << errMsg;
  }

  return fileNames;
}

void QmitkIOUtil::SaveBaseDataWithDialog(mitk::BaseData* data, std::string fileName, QWidget* /*parent*/)
{
  Save(data, fileName);
}

void QmitkIOUtil::SaveSurfaceWithDialog(mitk::Surface::Pointer surface, std::string fileName, QWidget* /*parent*/)
{
  Save(surface, fileName);
}

void QmitkIOUtil::SaveImageWithDialog(mitk::Image::Pointer image, std::string fileName, QWidget* /*parent*/)
{
  Save(image, fileName);
}

void QmitkIOUtil::SavePointSetWithDialog(mitk::PointSet::Pointer pointset, std::string fileName, QWidget* /*parent*/)
{
  Save(pointset, fileName);
}

struct QmitkIOUtil::SaveFilter::Impl
{
  void Init(const std::string& baseDataType)
  {
    // Add an artifical filter for "All"
    m_MimeTypes.push_back(ALL_MIMETYPE);
    m_FilterStrings.push_back("All (*.*)");

    // Get all writers and their mime types for the given base data type
    std::vector<std::string> mimeTypes;
    std::vector<mitk::FileWriterRegistry::WriterReference> refs = mitk::FileWriterRegistry::GetReferences(baseDataType);
    for (std::vector<mitk::FileWriterRegistry::WriterReference>::const_iterator iter = refs.begin(),
         iterEnd = refs.end(); iter != iterEnd; ++iter)
    {
      std::string mimeType = iter->GetProperty(mitk::IFileWriter::PROP_MIMETYPE()).ToString();
      if (!mimeType.empty() && std::find(mimeTypes.begin(), mimeTypes.end(), mimeType) == mimeTypes.end())
      {
        mimeTypes.push_back(mimeType);
      }
    }

    mitk::CoreServicePointer<mitk::IMimeTypeProvider> mimeTypeProvider(mitk::CoreServices::GetMimeTypeProvider());
    for (std::vector<std::string>::const_iterator iter = mimeTypes.begin(), iterEnd = mimeTypes.end();
         iter != iterEnd; ++iter)
    {
      QSet<QString> filterExtensions;
      std::vector<std::string> extensions = mimeTypeProvider->GetExtensions(*iter);
      for (std::vector<std::string>::iterator extIter = extensions.begin(), extIterEnd = extensions.end();
           extIter != extIterEnd; ++extIter)
      {
        filterExtensions << QString::fromStdString(*extIter);
      }

      QString filter = QString::fromStdString(mimeTypeProvider->GetDescription(*iter)) + " (";
      foreach(const QString& extension, filterExtensions)
      {
        filter += "*." + extension + " ";
      }
      filter = filter.replace(filter.size()-1, 1, ')');
      m_MimeTypes.push_back(*iter);
      m_FilterStrings.push_back(filter);
    }
  }

  std::vector<std::string> m_MimeTypes;
  QStringList m_FilterStrings;
};

std::string QmitkIOUtil::SaveFilter::ALL_MIMETYPE = "__all__";

QmitkIOUtil::SaveFilter::SaveFilter(const QmitkIOUtil::SaveFilter& other)
  : d(new Impl(*other.d))
{
}

QmitkIOUtil::SaveFilter::SaveFilter(const std::string& baseDataType)
  : d(new Impl)
{
  if (!baseDataType.empty())
  {
    d->Init(baseDataType);
  }
}

QmitkIOUtil::SaveFilter::SaveFilter(const mitk::BaseData* baseData)
  : d(new Impl)
{
  if (baseData != NULL)
  {
    d->Init(baseData->GetNameOfClass());
  }
}

QmitkIOUtil::SaveFilter& QmitkIOUtil::SaveFilter::operator=(const QmitkIOUtil::SaveFilter& other)
{
  d.reset(new Impl(*other.d));
  return *this;
}

QString QmitkIOUtil::SaveFilter::GetFilterForMimeType(const std::string& mimeType) const
{
  std::vector<std::string>::const_iterator iter =
      std::find(d->m_MimeTypes.begin(), d->m_MimeTypes.end(), mimeType);
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

std::string QmitkIOUtil::SaveFilter::GetMimeTypeForFilter(const QString& filter) const
{
  int index = d->m_FilterStrings.indexOf(filter);
  if (index < 0)
  {
    return std::string();
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

std::string QmitkIOUtil::SaveFilter::GetDefaultMimeType() const
{
  if (d->m_MimeTypes.size() > 1)
  {
    return d->m_MimeTypes.at(1);
  }
  else if (d->m_MimeTypes.size() > 0)
  {
    return d->m_MimeTypes.front();
  }
  return std::string();
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

bool QmitkIOUtil::SaveFilter::ContainsMimeType(const std::string& mimeType)
{
  return std::find(d->m_MimeTypes.begin(), d->m_MimeTypes.end(), mimeType) != d->m_MimeTypes.end();
}
