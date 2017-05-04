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

#include <map>
#include <string>

#include <boost/format.hpp>
#include <boost/thread/once.hpp>
#include <boost/numeric/conversion/cast.hpp>

#include <mitkIOUtil.h>
#include <mitkCoreObjectFactory.h>
#include "mitkCoreServices.h"
#include "mitkIMimeTypeProvider.h"
#include "mitkMimeType.h"
#include "mitkCustomMimeType.h"
#include "mitkFileReaderRegistry.h"
#include "mitkFileWriterRegistry.h"

#include "QmitkFileReaderOptionsDialog.h"
#include "QmitkFileWriterOptionsDialog.h"

#include "ConfigManager.h"

// QT
#include <QFileDialog>
#include <QMessageBox>
#include <QSet>
#include <QDebug>

//ITK
#include <itksys/SystemTools.hxx>

#include <algorithm>

#define MODULE_PREFIX "QmitkIOUtil/"

namespace
{
  const char REUSE_READER_COUNT[] = INTERNAL_PARAM_PREFIX MODULE_PREFIX "reader_count";
  const char REUSE_WRITER_COUNT[] = INTERNAL_PARAM_PREFIX MODULE_PREFIX "writer_count";

  const char REUSE_MIME_NAME[] = MODULE_PREFIX "mime_%s_%d";
  const char REUSE_MIME_READER[] = "reader";
  const char REUSE_MIME_WRITER[] = "writer";

  const char REUSE_REUSED_DESCRIPTION[] = INTERNAL_PARAM_PREFIX MODULE_PREFIX "description_%d";

  const char REUSE_OPTIONS_COUNT[] = INTERNAL_PARAM_PREFIX MODULE_PREFIX "options_count_%d";
  const char REUSE_OPTIONS_KEY[] = INTERNAL_PARAM_PREFIX MODULE_PREFIX "options_key_%d_%d";
  const char REUSE_OPTIONS_TYPE[] = INTERNAL_PARAM_PREFIX MODULE_PREFIX "options_type_%d_%d";
  const char REUSE_OPTIONS_VALUE[] = INTERNAL_PARAM_PREFIX MODULE_PREFIX "options_value_%d_%d";

  std::map<std::string, std::string> m_reusedReaders;
  std::map<std::string, std::string> m_reusedWriters;
  std::map<std::string, mitk::IFileIO::Options> m_reusedReadersOptions;
  std::map<std::string, mitk::IFileIO::Options> m_reusedWritersOptions;

  enum AnyType
  {
    ANY_UNKNOWN,
    ANY_STRING,
    ANY_BOOL,
    ANY_SHORT,
    ANY_INT,
    ANY_USHORT,
    ANY_UINT,
    ANY_FLOAT,
    ANY_DOUBLE
  };

  void LoadReusedImpl(int count, std::map<std::string, std::string>& reused, std::map<std::string, mitk::IFileIO::Options>& reusedOptions, bool isReader)
  {
    auto& configManager = config::ConfigManager::GetInstance();
    for (auto i = 1; i <= count; ++i) {
      const auto mime = configManager.GetString(boost::str(boost::format(REUSE_MIME_NAME) % (isReader ? REUSE_MIME_READER : REUSE_MIME_WRITER) % i).c_str(), QString()).toStdString();
      if (mime.empty()) {
        continue;
      }
      const auto index = isReader ? i : -i;
      reused.emplace(mime, configManager.GetString(boost::str(boost::format(REUSE_REUSED_DESCRIPTION) % index).c_str(), QString()).toStdString());
      const auto count = configManager.GetInt(boost::str(boost::format(REUSE_OPTIONS_COUNT) % index).c_str(), 0);
      auto& options = reusedOptions[mime];
      for (auto i = 0; i < count; ++i) {
        const auto key = configManager.GetString(boost::str(boost::format(REUSE_OPTIONS_KEY) % index % i).c_str(), QString()).toStdString();
        if (key.empty()) {
          continue;
        }
        const auto type = configManager.GetEnum(boost::str(boost::format(REUSE_OPTIONS_TYPE) % index % i).c_str(), ANY_UNKNOWN);
        switch (type) {
        case ANY_STRING:
          options.emplace(key, configManager.GetString(boost::str(boost::format(REUSE_OPTIONS_VALUE) % index % i).c_str(), QString()).toStdString());
          break;
        case ANY_BOOL:
          options.emplace(key, configManager.GetBool(boost::str(boost::format(REUSE_OPTIONS_VALUE) % index % i).c_str(), false));
          break;
        case ANY_SHORT:
          options.emplace(key, static_cast<short>(configManager.GetInt(boost::str(boost::format(REUSE_OPTIONS_VALUE) % index % i).c_str(), 0)));
          break;
        case ANY_INT:
          options.emplace(key, configManager.GetInt(boost::str(boost::format(REUSE_OPTIONS_VALUE) % index % i).c_str(), 0));
          break;
        case ANY_USHORT:
          options.emplace(key, static_cast<unsigned short>(configManager.GetInt(boost::str(boost::format(REUSE_OPTIONS_VALUE) % index % i).c_str(), 0)));
          break;
        case ANY_UINT:
          options.emplace(key, static_cast<unsigned int>(configManager.GetInt(boost::str(boost::format(REUSE_OPTIONS_VALUE) % index % i).c_str(), 0)));
          break;
        case ANY_FLOAT:
          options.emplace(key, static_cast<float>(configManager.GetDouble(boost::str(boost::format(REUSE_OPTIONS_VALUE) % index % i).c_str(), 0)));
          break;
        case ANY_DOUBLE:
          options.emplace(key, configManager.GetDouble(boost::str(boost::format(REUSE_OPTIONS_VALUE) % index % i).c_str(), 0));
          break;
        }
      }
    }
  }

  boost::once_flag onceLoadReused = BOOST_ONCE_INIT;
  void LoadReused()
  {
    auto& configManager = config::ConfigManager::GetInstance();
    LoadReusedImpl(configManager.GetInt(REUSE_READER_COUNT, 0), m_reusedReaders, m_reusedReadersOptions, true);
    LoadReusedImpl(configManager.GetInt(REUSE_WRITER_COUNT, 0), m_reusedWriters, m_reusedWritersOptions, false);
  }

  int GetReused(const mitk::IOUtil::LoadInfo&, std::map<std::string, std::string>* *reused,
    std::map<std::string, mitk::IFileIO::Options>** reusedOptions)
  {
    *reused = &m_reusedReaders;
    *reusedOptions = &m_reusedReadersOptions;
    return boost::numeric_cast<int>(m_reusedReaders.size()) + 1;
  }

  int GetReused(const mitk::IOUtil::SaveInfo&, std::map<std::string, std::string>** reused,
    std::map<std::string, mitk::IFileIO::Options>** reusedOptions)
  {
    *reused = &m_reusedWriters;
    *reusedOptions = &m_reusedWritersOptions;
    return -boost::numeric_cast<int>(m_reusedWriters.size()) - 1;
  }

  mitk::FileReaderSelector::Item GetSelected(mitk::IOUtil::LoadInfo& loadInfo)
  {
    return loadInfo.m_ReaderSelector.GetSelected();
  }

  mitk::FileWriterSelector::Item GetSelected(mitk::IOUtil::SaveInfo& saveInfo)
  {
    return saveInfo.m_WriterSelector.GetSelected();
  }

  mitk::IFileIO::Options GetOptions(mitk::FileReaderSelector::Item& item)
  {
    return item.GetReader()->GetOptions();
  }

  mitk::IFileIO::Options GetOptions(mitk::FileWriterSelector::Item& item)
  {
    return item.GetWriter()->GetOptions();
  }

  std::vector<mitk::MimeType> GetMimeTypes(mitk::IOUtil::LoadInfo& loadInfo)
  {
    return loadInfo.m_ReaderSelector.GetMimeTypes();
  }

  std::vector<mitk::MimeType> GetMimeTypes(mitk::IOUtil::SaveInfo& saveInfo)
  {
    return { saveInfo.m_WriterSelector.GetSelected().GetMimeType() };
  }

  std::vector<mitk::FileReaderSelector::Item> GetItems(mitk::IOUtil::LoadInfo& loadInfo)
  {
    return loadInfo.m_ReaderSelector.Get();
  }

  std::vector<mitk::FileWriterSelector::Item> GetItems(mitk::IOUtil::SaveInfo& saveInfo)
  {
    return saveInfo.m_WriterSelector.Get();
  }

  void SelectItem(mitk::IOUtil::LoadInfo& loadInfo,
    const mitk::FileReaderSelector::Item& reader, const mitk::IFileIO::Options& options)
  {
    loadInfo.m_ReaderSelector.Select(reader);
    loadInfo.m_ReaderSelector.GetSelected().GetReader()->SetOptions(options);
  }

  void SelectItem(mitk::IOUtil::SaveInfo& saveInfo,
    const mitk::FileWriterSelector::Item& writer, const mitk::IFileIO::Options& options)
  {
    saveInfo.m_WriterSelector.Select(writer);
    saveInfo.m_WriterSelector.GetSelected().GetWriter()->SetOptions(options);
  }

  template <typename TDialog, typename TInfo>
  bool OptionsDialogFunctor(TInfo& info)
  {
    boost::call_once(&LoadReused, onceLoadReused);

    std::map<std::string, std::string>* reused = nullptr;
    std::map<std::string, mitk::IFileIO::Options>* reusedOptions = nullptr;
    const auto index = GetReused(info, &reused, &reusedOptions);

    const auto items = GetItems(info);
    const auto mimeTypes = GetMimeTypes(info);
    for (auto type : mimeTypes) {
      auto iter = reused->find(type.GetName());
      if (reused->end() == iter) {
        continue;
      }
      for (auto it = items.begin(); it != items.end(); ++it) {
        if (it->GetMimeType().GetName() != type.GetName()) {
          continue;
        }
        if (it->GetDescription() == iter->second) {
          SelectItem(info, *it, (*reusedOptions)[type.GetName()]);
          return false;
        }
      }
    }

    TDialog dialog(info);
    if (dialog.exec() == QDialog::Accepted) {
      if (!dialog.ReuseOptions()) {
        return true;
      }

      auto selected = GetSelected(info);
      const auto mime = selected.GetMimeType().GetName();
      const auto description = selected.GetDescription();
      const auto options = GetOptions(selected);

      (*reused)[mime] = description;
      (*reusedOptions)[mime] = options;

      auto& configManager = config::ConfigManager::GetInstance();
      if (index > 0) {
        configManager.PutInt(REUSE_READER_COUNT, index);
      } else if (index < 0) {
        configManager.PutInt(REUSE_WRITER_COUNT, -index);
      } else {
        return false;
      }
      configManager.PutString(boost::str(boost::format(REUSE_MIME_NAME) % (index > 0 ? REUSE_MIME_READER : REUSE_MIME_WRITER) % abs(index)).c_str(), mime.c_str());
      configManager.PutString(boost::str(boost::format(REUSE_REUSED_DESCRIPTION) % index).c_str(), description.c_str());
      configManager.PutInt(boost::str(boost::format(REUSE_OPTIONS_COUNT) % index).c_str(), options.size());
      auto iter = options.begin();
      for (auto i = 0; i < options.size(); ++i, ++iter) {
        configManager.PutString(boost::str(boost::format(REUSE_OPTIONS_KEY) % index % i).c_str(), iter->first.c_str());
        const std::type_info& anyType = iter->second.Type();
        if (anyType == typeid(std::string)) {
          configManager.PutEnum(boost::str(boost::format(REUSE_OPTIONS_TYPE) % index % i).c_str(), ANY_STRING);
          configManager.PutString(boost::str(boost::format(REUSE_OPTIONS_VALUE) % index % i).c_str(), us::any_cast<std::string>(iter->second).c_str());
        } else if (anyType == typeid(bool)) {
          configManager.PutEnum(boost::str(boost::format(REUSE_OPTIONS_TYPE) % index % i).c_str(), ANY_BOOL);
          configManager.PutBool(boost::str(boost::format(REUSE_OPTIONS_VALUE) % index % i).c_str(), us::any_cast<bool>(iter->second));
        } else if (anyType == typeid(short)) {
          configManager.PutEnum(boost::str(boost::format(REUSE_OPTIONS_TYPE) % index % i).c_str(), ANY_SHORT);
          configManager.PutInt(boost::str(boost::format(REUSE_OPTIONS_VALUE) % index % i).c_str(), us::any_cast<short>(iter->second));
        } else if (anyType == typeid(int)) {
          configManager.PutEnum(boost::str(boost::format(REUSE_OPTIONS_TYPE) % index % i).c_str(), ANY_INT);
          configManager.PutInt(boost::str(boost::format(REUSE_OPTIONS_VALUE) % index % i).c_str(), us::any_cast<int>(iter->second));
        } else if (anyType == typeid(unsigned short)) {
          configManager.PutEnum(boost::str(boost::format(REUSE_OPTIONS_TYPE) % index % i).c_str(), ANY_USHORT);
          configManager.PutInt(boost::str(boost::format(REUSE_OPTIONS_VALUE) % index % i).c_str(), us::any_cast<unsigned short>(iter->second));
        } else if (anyType == typeid(unsigned int)) {
          configManager.PutEnum(boost::str(boost::format(REUSE_OPTIONS_TYPE) % index % i).c_str(), ANY_UINT);
          configManager.PutInt(boost::str(boost::format(REUSE_OPTIONS_VALUE) % index % i).c_str(), static_cast<int>(us::any_cast<unsigned int>(iter->second)));
        } else if (anyType == typeid(float)) {
          configManager.PutEnum(boost::str(boost::format(REUSE_OPTIONS_TYPE) % index % i).c_str(), ANY_FLOAT);
          configManager.PutDouble(boost::str(boost::format(REUSE_OPTIONS_VALUE) % index % i).c_str(), us::any_cast<float>(iter->second));
        } else if (anyType == typeid(double)) {
          configManager.PutEnum(boost::str(boost::format(REUSE_OPTIONS_TYPE) % index % i).c_str(), ANY_DOUBLE);
          configManager.PutDouble(boost::str(boost::format(REUSE_OPTIONS_VALUE) % index % i).c_str(), us::any_cast<double>(iter->second));
        }
      }
      return false;
    } else {
      info.m_Cancel = true;
      return true;
    }
  }
}

struct QmitkIOUtil::Impl
{
  struct ReaderOptionsDialogFunctor : public ReaderOptionsFunctorBase
  {

    virtual bool operator()(LoadInfo& loadInfo) override
    {
      return OptionsDialogFunctor<QmitkFileReaderOptionsDialog>(loadInfo);
    }
  };

  struct WriterOptionsDialogFunctor : public WriterOptionsFunctorBase
  {

    virtual bool operator()(SaveInfo& saveInfo) override
    {
      return OptionsDialogFunctor<QmitkFileWriterOptionsDialog>(saveInfo);
    }
  };

};

struct MimeTypeComparison : public std::unary_function<mitk::MimeType, bool>
{
  MimeTypeComparison(const std::string& mimeTypeName)
    : m_Name(mimeTypeName)
  {}

  bool operator()(const mitk::MimeType& mimeType) const
  {
    return mimeType.GetName() == m_Name;
  }

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
    foreach(const QString& extension, filterExtensions)
    {
      filter += "*." + extension + " ";
    }
    filter = filter.replace(filter.size()-1, 1, ')');
    filters += ";;" + filter;
  }
  filters.prepend("All (*)");
  return filters;
}

QList<mitk::BaseData::Pointer> QmitkIOUtil::Load(const QStringList& paths, QWidget* parent)
{
  std::vector<LoadInfo> loadInfos;
  foreach(const QString& file, paths)
  {
    loadInfos.push_back(LoadInfo(file.toStdString()));
  }

  Impl::ReaderOptionsDialogFunctor optionsCallback;
  std::string errMsg = Load(loadInfos, NULL, NULL, &optionsCallback);
  if (!errMsg.empty())
  {
    QMessageBox::warning(parent, "Error reading files", QString::fromStdString(errMsg));
    mitkThrow() << errMsg;
  }

  QList<mitk::BaseData::Pointer> qResult;
  for(std::vector<LoadInfo>::const_iterator iter = loadInfos.begin(), iterEnd = loadInfos.end();
      iter != iterEnd; ++iter)
  {
    for (const auto & elem : iter->m_Output)
    {
      qResult << elem;
    }
  }
  return qResult;
}

mitk::DataStorage::SetOfObjects::Pointer QmitkIOUtil::Load(const QStringList& paths, mitk::DataStorage& storage, QWidget* parent)
{
  std::vector<LoadInfo> loadInfos;
  foreach(const QString& file, paths)
  {
    loadInfos.push_back(LoadInfo(QFile::encodeName(file).constData()));
  }

  mitk::DataStorage::SetOfObjects::Pointer nodeResult = mitk::DataStorage::SetOfObjects::New();
  Impl::ReaderOptionsDialogFunctor optionsCallback;
  std::string errMsg = Load(loadInfos, nodeResult, &storage, &optionsCallback);
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
    SaveInfo saveInfo(*dataIter, mitk::MimeType(), std::string());

    SaveFilter filters(saveInfo);

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
    QString nextName =  QFileDialog::getSaveFileName(parent,
                                                     dialogTitle,
                                                     fileName,
                                                     filterString,
                                                     &selectedFilter);

    if (nextName.isEmpty())
    {
      // We stop asking for further file names, but we still save the
      // data where the user already confirmed the save dialog.
      break;
    }

    fileName = nextName;
    std::string stdFileName = QFile::encodeName(fileName).constData();
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
         mimeTypeIterEnd = filterMimeTypes.end(); mimeTypeIter != mimeTypeIterEnd; ++mimeTypeIter)
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
          stdFileName = QFile::encodeName(fileName).constData();
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
            if (QMessageBox::question(parent, "Replace File",
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
      QMessageBox::warning(parent,
                           "Saving not possible",
                           QString("No mime-type available which can handle \"%1\".")
                           .arg(fileName));
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
    std::string errMsg = Save(saveInfos, &optionsCallback);
    if (!errMsg.empty())
    {
      QMessageBox::warning(parent, "Error writing files", QString::fromStdString(errMsg));
      mitkThrow() << errMsg;
    }
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
  Impl(const mitk::IOUtil::SaveInfo& saveInfo)
    : m_SaveInfo(saveInfo)
  {
    // Add an artifical filter for "All"
    m_MimeTypes.push_back(ALL_MIMETYPE());
    m_FilterStrings.push_back("All (*.*)");

    // Get all writers and their mime types for the given base data type
    // (this is sorted already)
    std::vector<mitk::MimeType> mimeTypes = saveInfo.m_WriterSelector.GetMimeTypes();

    for (std::vector<mitk::MimeType>::const_reverse_iterator iter = mimeTypes.rbegin(),
         iterEnd = mimeTypes.rend(); iter != iterEnd; ++iter)
    {
      QList<QString> filterExtensions;
      mitk::MimeType mimeType = *iter;
      std::vector<std::string> extensions = mimeType.GetExtensions();
      for (auto & extension : extensions)
      {
        filterExtensions << QString::fromStdString(extension);
      }
      if (m_DefaultExtension.isEmpty())
      {
        m_DefaultExtension = QString::fromStdString(extensions.front());
      }

      QString filter = QString::fromStdString(mimeType.GetComment()) + " (";
      foreach(const QString& extension, filterExtensions)
      {
        filter += "*." + extension + " ";
      }
      filter = filter.replace(filter.size()-1, 1, ')');
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

QmitkIOUtil::SaveFilter::SaveFilter(const QmitkIOUtil::SaveFilter& other)
  : d(new Impl(*other.d))
{
}

QmitkIOUtil::SaveFilter::SaveFilter(const SaveInfo& saveInfo)
  : d(new Impl(saveInfo))
{
}

QmitkIOUtil::SaveFilter& QmitkIOUtil::SaveFilter::operator=(const QmitkIOUtil::SaveFilter& other)
{
  d.reset(new Impl(*other.d));
  return *this;
}

std::vector<mitk::MimeType> QmitkIOUtil::SaveFilter::GetMimeTypes() const
{
  return d->m_MimeTypes;
}

QString QmitkIOUtil::SaveFilter::GetFilterForMimeType(const std::string& mimeType) const
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

mitk::MimeType QmitkIOUtil::SaveFilter::GetMimeTypeForFilter(const QString& filter) const
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

bool QmitkIOUtil::SaveFilter::ContainsMimeType(const std::string& mimeType)
{
  return std::find_if(d->m_MimeTypes.begin(), d->m_MimeTypes.end(), MimeTypeComparison(mimeType)) != d->m_MimeTypes.end();
}
