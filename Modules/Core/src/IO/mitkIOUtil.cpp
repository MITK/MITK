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

#include "mitkIOUtil.h"

#include <mitkCoreObjectFactory.h>
#include <mitkCoreServices.h>
#include <mitkExceptionMacro.h>
#include <mitkFileReaderRegistry.h>
#include <mitkFileWriterRegistry.h>
#include <mitkIMimeTypeProvider.h>
#include <mitkProgressBar.h>
#include <mitkStandaloneDataStorage.h>
#include <usGetModuleContext.h>
#include <usLDAPProp.h>
#include <usModuleContext.h>
#include <usModuleResource.h>
#include <usModuleResourceStream.h>
#include <mitkAbstractFileReader.h>

// ITK
#include <itksys/SystemTools.hxx>

// VTK
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>
#include <vtkTriangleFilter.h>

#include <cerrno>
#include <cstdlib>

static std::string GetLastErrorStr()
{
#ifdef US_PLATFORM_POSIX
  return std::string(strerror(errno));
#else
  // Retrieve the system error message for the last-error code
  LPVOID lpMsgBuf;
  DWORD dw = GetLastError();

  FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                nullptr,
                dw,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPTSTR)&lpMsgBuf,
                0,
                nullptr);

  std::string errMsg((LPCTSTR)lpMsgBuf);

  LocalFree(lpMsgBuf);

  return errMsg;
#endif
}

#ifdef US_PLATFORM_WINDOWS

#include <direct.h>
#include <io.h>

// make the posix flags point to the obsolte bsd types on windows
#define S_IRUSR S_IREAD
#define S_IWUSR S_IWRITE

#else

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#endif

#include <fcntl.h>
#include <sys/stat.h>

static const char validLetters[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

// A cross-platform version of the mkstemps function
static int mkstemps_compat(char *tmpl, int suffixlen)
{
  static unsigned long long value = 0;
  int savedErrno = errno;

// Lower bound on the number of temporary files to attempt to generate.
#define ATTEMPTS_MIN (62 * 62 * 62)

/* The number of times to attempt to generate a temporary file.  To
   conform to POSIX, this must be no smaller than TMP_MAX.  */
#if ATTEMPTS_MIN < TMP_MAX
  const unsigned int attempts = TMP_MAX;
#else
  const unsigned int attempts = ATTEMPTS_MIN;
#endif

  const int len = strlen(tmpl);
  if ((len - suffixlen) < 6 || strncmp(&tmpl[len - 6 - suffixlen], "XXXXXX", 6))
  {
    errno = EINVAL;
    return -1;
  }

  /* This is where the Xs start.  */
  char *XXXXXX = &tmpl[len - 6 - suffixlen];

/* Get some more or less random data.  */
#ifdef US_PLATFORM_WINDOWS
  {
    SYSTEMTIME stNow;
    FILETIME ftNow;

    // get system time
    GetSystemTime(&stNow);
    stNow.wMilliseconds = 500;
    if (!SystemTimeToFileTime(&stNow, &ftNow))
    {
      errno = -1;
      return -1;
    }
    unsigned long long randomTimeBits = ((static_cast<unsigned long long>(ftNow.dwHighDateTime) << 32) |
                                         static_cast<unsigned long long>(ftNow.dwLowDateTime));
    value = randomTimeBits ^ static_cast<unsigned long long>(GetCurrentThreadId());
  }
#else
  {
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    unsigned long long randomTimeBits =
      ((static_cast<unsigned long long>(tv.tv_usec) << 32) | static_cast<unsigned long long>(tv.tv_sec));
    value = randomTimeBits ^ static_cast<unsigned long long>(getpid());
  }
#endif

  for (unsigned int count = 0; count < attempts; value += 7777, ++count)
  {
    unsigned long long v = value;

    /* Fill in the random bits.  */
    XXXXXX[0] = validLetters[v % 62];
    v /= 62;
    XXXXXX[1] = validLetters[v % 62];
    v /= 62;
    XXXXXX[2] = validLetters[v % 62];
    v /= 62;
    XXXXXX[3] = validLetters[v % 62];
    v /= 62;
    XXXXXX[4] = validLetters[v % 62];
    v /= 62;
    XXXXXX[5] = validLetters[v % 62];

    int fd = open(tmpl, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
    if (fd >= 0)
    {
      errno = savedErrno;
      return fd;
    }
    else if (errno != EEXIST)
    {
      return -1;
    }
  }

  /* We got out of the loop because we ran out of combinations to try.  */
  errno = EEXIST;
  return -1;
}

// A cross-platform version of the POSIX mkdtemp function
static char *mkdtemps_compat(char *tmpl, int suffixlen)
{
  static unsigned long long value = 0;
  int savedErrno = errno;

// Lower bound on the number of temporary dirs to attempt to generate.
#define ATTEMPTS_MIN (62 * 62 * 62)

/* The number of times to attempt to generate a temporary dir.  To
   conform to POSIX, this must be no smaller than TMP_MAX.  */
#if ATTEMPTS_MIN < TMP_MAX
  const unsigned int attempts = TMP_MAX;
#else
  const unsigned int attempts = ATTEMPTS_MIN;
#endif

  const int len = strlen(tmpl);
  if ((len - suffixlen) < 6 || strncmp(&tmpl[len - 6 - suffixlen], "XXXXXX", 6))
  {
    errno = EINVAL;
    return nullptr;
  }

  /* This is where the Xs start.  */
  char *XXXXXX = &tmpl[len - 6 - suffixlen];

/* Get some more or less random data.  */
#ifdef US_PLATFORM_WINDOWS
  {
    SYSTEMTIME stNow;
    FILETIME ftNow;

    // get system time
    GetSystemTime(&stNow);
    stNow.wMilliseconds = 500;
    if (!SystemTimeToFileTime(&stNow, &ftNow))
    {
      errno = -1;
      return nullptr;
    }
    unsigned long long randomTimeBits = ((static_cast<unsigned long long>(ftNow.dwHighDateTime) << 32) |
                                         static_cast<unsigned long long>(ftNow.dwLowDateTime));
    value = randomTimeBits ^ static_cast<unsigned long long>(GetCurrentThreadId());
  }
#else
  {
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    unsigned long long randomTimeBits =
      ((static_cast<unsigned long long>(tv.tv_usec) << 32) | static_cast<unsigned long long>(tv.tv_sec));
    value = randomTimeBits ^ static_cast<unsigned long long>(getpid());
  }
#endif

  unsigned int count = 0;
  for (; count < attempts; value += 7777, ++count)
  {
    unsigned long long v = value;

    /* Fill in the random bits.  */
    XXXXXX[0] = validLetters[v % 62];
    v /= 62;
    XXXXXX[1] = validLetters[v % 62];
    v /= 62;
    XXXXXX[2] = validLetters[v % 62];
    v /= 62;
    XXXXXX[3] = validLetters[v % 62];
    v /= 62;
    XXXXXX[4] = validLetters[v % 62];
    v /= 62;
    XXXXXX[5] = validLetters[v % 62];

#ifdef US_PLATFORM_WINDOWS
    int fd = _mkdir(tmpl); //, _S_IREAD | _S_IWRITE | _S_IEXEC);
#else
    int fd = mkdir(tmpl, S_IRUSR | S_IWUSR | S_IXUSR);
#endif
    if (fd >= 0)
    {
      errno = savedErrno;
      return tmpl;
    }
    else if (errno != EEXIST)
    {
      return nullptr;
    }
  }

  /* We got out of the loop because we ran out of combinations to try.  */
  errno = EEXIST;
  return nullptr;
}

//#endif

//**************************************************************
// mitk::IOUtil method definitions

namespace mitk
{
  struct IOUtil::Impl
  {
    struct FixedReaderOptionsFunctor : public ReaderOptionsFunctorBase
    {
      FixedReaderOptionsFunctor(const IFileReader::Options &options) : m_Options(options) {}
      bool operator()(LoadInfo &loadInfo) const override
      {
        IFileReader *reader = loadInfo.m_ReaderSelector.GetSelected().GetReader();
        if (reader)
        {
          reader->SetOptions(m_Options);
        }
        return false;
      }

    private:
      const IFileReader::Options &m_Options;
    };

    struct FixedWriterOptionsFunctor : public WriterOptionsFunctorBase
    {
      FixedWriterOptionsFunctor(const IFileReader::Options &options) : m_Options(options) {}
      bool operator()(SaveInfo &saveInfo) const override
      {
        IFileWriter *writer = saveInfo.m_WriterSelector.GetSelected().GetWriter();
        if (writer)
        {
          writer->SetOptions(m_Options);
        }
        return false;
      }

    private:
      const IFileWriter::Options &m_Options;
    };

    static BaseData::Pointer LoadBaseDataFromFile(const std::string &path, const ReaderOptionsFunctorBase* optionsCallback = nullptr);

    static void SetDefaultDataNodeProperties(mitk::DataNode *node, const std::string &filePath = std::string());
  };

  BaseData::Pointer IOUtil::Impl::LoadBaseDataFromFile(const std::string &path,
                                                       const ReaderOptionsFunctorBase *optionsCallback)
  {
    std::vector<BaseData::Pointer> baseDataList = Load(path, optionsCallback);

    // The Load(path) call above should throw an exception if nothing could be loaded
    assert(!baseDataList.empty());
    return baseDataList.front();
  }

#ifdef US_PLATFORM_WINDOWS
  std::string IOUtil::GetProgramPath()
  {
    char path[512];
    std::size_t index = std::string(path, GetModuleFileName(nullptr, path, 512)).find_last_of('\\');
    return std::string(path, index);
  }
#elif defined(US_PLATFORM_APPLE)
#include <mach-o/dyld.h>
  std::string IOUtil::GetProgramPath()
  {
    char path[512];
    uint32_t size = sizeof(path);
    if (_NSGetExecutablePath(path, &size) == 0)
    {
      std::size_t index = std::string(path).find_last_of('/');
      std::string strPath = std::string(path, index);
      // const char* execPath = strPath.c_str();
      // mitk::StandardFileLocations::GetInstance()->AddDirectoryForSearch(execPath,false);
      return strPath;
    }
    return std::string();
  }
#else
#include <sstream>
#include <sys/types.h>
#include <unistd.h>
  std::string IOUtil::GetProgramPath()
  {
    std::stringstream ss;
    ss << "/proc/" << getpid() << "/exe";
    char proc[512] = {0};
    ssize_t ch = readlink(ss.str().c_str(), proc, 512);
    if (ch == -1)
      return std::string();
    std::size_t index = std::string(proc).find_last_of('/');
    return std::string(proc, index);
  }
#endif

  char IOUtil::GetDirectorySeparator()
  {
#ifdef US_PLATFORM_WINDOWS
    return '\\';
#else
    return '/';
#endif
  }

  std::string IOUtil::GetTempPath()
  {
    static std::string result;
    if (result.empty())
    {
#ifdef US_PLATFORM_WINDOWS
      char tempPathTestBuffer[1];
      DWORD bufferLength = ::GetTempPath(1, tempPathTestBuffer);
      if (bufferLength == 0)
      {
        mitkThrow() << GetLastErrorStr();
      }
      std::vector<char> tempPath(bufferLength);
      bufferLength = ::GetTempPath(bufferLength, &tempPath[0]);
      if (bufferLength == 0)
      {
        mitkThrow() << GetLastErrorStr();
      }
      result.assign(tempPath.begin(), tempPath.begin() + static_cast<std::size_t>(bufferLength));
#else
      result = "/tmp/";
#endif
    }

    return result;
  }

  std::string IOUtil::CreateTemporaryFile(const std::string &templateName, std::string path)
  {
    ofstream tmpOutputStream;
    std::string returnValue = CreateTemporaryFile(tmpOutputStream, templateName, path);
    tmpOutputStream.close();
    return returnValue;
  }

  std::string IOUtil::CreateTemporaryFile(std::ofstream &f, const std::string &templateName, std::string path)
  {
    return CreateTemporaryFile(f, std::ios_base::out | std::ios_base::trunc, templateName, path);
  }

  std::string IOUtil::CreateTemporaryFile(std::ofstream &f,
                                          std::ios_base::openmode mode,
                                          const std::string &templateName,
                                          std::string path)
  {
    if (path.empty())
    {
      path = GetTempPath();
    }

    path += templateName;

    std::vector<char> dst_path(path.begin(), path.end());
    dst_path.push_back('\0');

    std::size_t lastX = path.find_last_of('X');
    std::size_t firstX = path.find_last_not_of('X', lastX);
    int firstNonX = firstX == std::string::npos ? -1 : firstX - 1;
    while (lastX != std::string::npos && (lastX - firstNonX) < 6)
    {
      lastX = path.find_last_of('X', firstX);
      firstX = path.find_last_not_of('X', lastX);
      firstNonX = firstX == std::string::npos ? -1 : firstX - 1;
    }
    std::size_t suffixlen = lastX == std::string::npos ? path.size() : path.size() - lastX - 1;

    int fd = mkstemps_compat(&dst_path[0], suffixlen);
    if (fd != -1)
    {
      path.assign(dst_path.begin(), dst_path.end() - 1);
      f.open(path.c_str(), mode | std::ios_base::out | std::ios_base::trunc);
      close(fd);
    }
    else
    {
      mitkThrow() << "Creating temporary file " << &dst_path[0] << " failed: " << GetLastErrorStr();
    }
    return path;
  }

  std::string IOUtil::CreateTemporaryDirectory(const std::string &templateName, std::string path)
  {
    if (path.empty())
    {
      path = GetTempPath();
    }

    path += GetDirectorySeparator() + templateName;
    std::vector<char> dst_path(path.begin(), path.end());
    dst_path.push_back('\0');

    std::size_t lastX = path.find_last_of('X');
    std::size_t firstX = path.find_last_not_of('X', lastX);
    int firstNonX = firstX == std::string::npos ? -1 : firstX - 1;
    while (lastX != std::string::npos && (lastX - firstNonX) < 6)
    {
      lastX = path.find_last_of('X', firstX);
      firstX = path.find_last_not_of('X', lastX);
      firstNonX = firstX == std::string::npos ? -1 : firstX - 1;
    }
    std::size_t suffixlen = lastX == std::string::npos ? path.size() : path.size() - lastX - 1;

    if (mkdtemps_compat(&dst_path[0], suffixlen) == nullptr)
    {
      mitkThrow() << "Creating temporary directory " << &dst_path[0] << " failed: " << GetLastErrorStr();
    }

    path.assign(dst_path.begin(), dst_path.end() - 1);
    return path;
  }

  DataStorage::SetOfObjects::Pointer IOUtil::Load(const std::string &path, DataStorage &storage, const ReaderOptionsFunctorBase *optionsCallback)
  {
    std::vector<std::string> paths;
    paths.push_back(path);
    return Load(paths, storage, optionsCallback);
  }

  DataStorage::SetOfObjects::Pointer IOUtil::Load(const std::string &path,
                                                  const IFileReader::Options &options,
                                                  DataStorage &storage)
  {
    std::vector<LoadInfo> loadInfos;
    loadInfos.push_back(LoadInfo(path));
    DataStorage::SetOfObjects::Pointer nodeResult = DataStorage::SetOfObjects::New();
    Impl::FixedReaderOptionsFunctor optionsCallback(options);
    std::string errMsg = Load(loadInfos, nodeResult, &storage, &optionsCallback);
    if (!errMsg.empty())
    {
      mitkThrow() << errMsg;
    }
    return nodeResult;
  }

  std::vector<BaseData::Pointer> IOUtil::Load(const std::string &path, const ReaderOptionsFunctorBase *optionsCallback)
  {
    std::vector<std::string> paths;
    paths.push_back(path);
    return Load(paths, optionsCallback);
  }

  std::vector<BaseData::Pointer> IOUtil::Load(const std::string &path, const IFileReader::Options &options)
  {
    std::vector<LoadInfo> loadInfos;
    loadInfos.push_back(LoadInfo(path));
    Impl::FixedReaderOptionsFunctor optionsCallback(options);
    std::string errMsg = Load(loadInfos, nullptr, nullptr, &optionsCallback);
    if (!errMsg.empty())
    {
      mitkThrow() << errMsg;
    }
    return loadInfos.front().m_Output;
  }

  DataStorage::SetOfObjects::Pointer IOUtil::Load(const std::vector<std::string> &paths, DataStorage &storage, const ReaderOptionsFunctorBase *optionsCallback)
  {
    DataStorage::SetOfObjects::Pointer nodeResult = DataStorage::SetOfObjects::New();
    std::vector<LoadInfo> loadInfos;
    for (auto loadInfo : paths)
    {
      loadInfos.push_back(loadInfo);
    }
    std::string errMsg = Load(loadInfos, nodeResult, &storage, optionsCallback);
    if (!errMsg.empty())
    {
      mitkThrow() << errMsg;
    }
    return nodeResult;
  }

  std::vector<BaseData::Pointer> IOUtil::Load(const std::vector<std::string> &paths, const ReaderOptionsFunctorBase *optionsCallback)
  {
    std::vector<BaseData::Pointer> result;
    std::vector<LoadInfo> loadInfos;
    for (auto loadInfo : paths)
    {
      loadInfos.push_back(loadInfo);
    }
    std::string errMsg = Load(loadInfos, nullptr, nullptr, optionsCallback);
    if (!errMsg.empty())
    {
      mitkThrow() << errMsg;
    }

    for (std::vector<LoadInfo>::const_iterator iter = loadInfos.begin(), iterEnd = loadInfos.end(); iter != iterEnd;
         ++iter)
    {
      result.insert(result.end(), iter->m_Output.begin(), iter->m_Output.end());
    }
    return result;
  }

  std::string IOUtil::Load(std::vector<LoadInfo> &loadInfos,
                           DataStorage::SetOfObjects *nodeResult,
                           DataStorage *ds,
                           const ReaderOptionsFunctorBase *optionsCallback)
  {
    if (loadInfos.empty())
    {
      return "No input files given";
    }

    int filesToRead = loadInfos.size();
    mitk::ProgressBar::GetInstance()->AddStepsToDo(2 * filesToRead);

    std::string errMsg;

    std::map<std::string, FileReaderSelector::Item> usedReaderItems;

    std::vector< std::string > read_files;
    for (auto &loadInfo : loadInfos)
    {
      if(std::find(read_files.begin(), read_files.end(), loadInfo.m_Path) != read_files.end())
        continue;

      std::vector<FileReaderSelector::Item> readers = loadInfo.m_ReaderSelector.Get();

      if (readers.empty())
      {
        if (!itksys::SystemTools::FileExists(loadInfo.m_Path.c_str()))
        {
          errMsg += "File '" + loadInfo.m_Path + "' does not exist\n";
        }
        else
        {
          errMsg += "No reader available for '" + loadInfo.m_Path + "'\n";
        }
        continue;
      }

      bool callOptionsCallback = readers.size() > 1 || !readers.front().GetReader()->GetOptions().empty();

      // check if we already used a reader which should be re-used
      std::vector<MimeType> currMimeTypes = loadInfo.m_ReaderSelector.GetMimeTypes();
      std::string selectedMimeType;
      for (std::vector<MimeType>::const_iterator mimeTypeIter = currMimeTypes.begin(),
                                                 mimeTypeIterEnd = currMimeTypes.end();
           mimeTypeIter != mimeTypeIterEnd;
           ++mimeTypeIter)
      {
        std::map<std::string, FileReaderSelector::Item>::const_iterator oldSelectedItemIter =
          usedReaderItems.find(mimeTypeIter->GetName());
        if (oldSelectedItemIter != usedReaderItems.end())
        {
          // we found an already used item for a mime-type which is contained
          // in the current reader set, check all current readers if there service
          // id equals the old reader
          for (std::vector<FileReaderSelector::Item>::const_iterator currReaderItem = readers.begin(),
                                                                     currReaderItemEnd = readers.end();
               currReaderItem != currReaderItemEnd;
               ++currReaderItem)
          {
            if (currReaderItem->GetMimeType().GetName() == mimeTypeIter->GetName() &&
                currReaderItem->GetServiceId() == oldSelectedItemIter->second.GetServiceId() &&
                currReaderItem->GetConfidenceLevel() >= oldSelectedItemIter->second.GetConfidenceLevel())
            {
              // okay, we used the same reader already, re-use its options
              selectedMimeType = mimeTypeIter->GetName();
              callOptionsCallback = false;
              loadInfo.m_ReaderSelector.Select(oldSelectedItemIter->second.GetServiceId());
              loadInfo.m_ReaderSelector.GetSelected().GetReader()->SetOptions(
                oldSelectedItemIter->second.GetReader()->GetOptions());
              break;
            }
          }
          if (!selectedMimeType.empty())
            break;
        }
      }

      if (callOptionsCallback && optionsCallback)
      {
        callOptionsCallback = (*optionsCallback)(loadInfo);
        if (!callOptionsCallback && !loadInfo.m_Cancel)
        {
          usedReaderItems.erase(selectedMimeType);
          FileReaderSelector::Item selectedItem = loadInfo.m_ReaderSelector.GetSelected();
          usedReaderItems.insert(std::make_pair(selectedItem.GetMimeType().GetName(), selectedItem));
        }
      }

      if (loadInfo.m_Cancel)
      {
        errMsg += "Reading operation(s) cancelled.";
        break;
      }

      IFileReader *reader = loadInfo.m_ReaderSelector.GetSelected().GetReader();
      if (reader == nullptr)
      {
        errMsg += "Unexpected nullptr reader.";
        break;
      }

      // Do the actual reading
      try
      {
        DataStorage::SetOfObjects::Pointer nodes;
        if (ds != nullptr)
        {
          nodes = reader->Read(*ds);

          std::vector< std::string > new_files =  reader->GetReadFiles();
          read_files.insert( read_files.end(), new_files.begin(), new_files.end() );
        }
        else
        {
          nodes = DataStorage::SetOfObjects::New();
          std::vector<mitk::BaseData::Pointer> baseData = reader->Read();
          for (auto iter = baseData.begin(); iter != baseData.end(); ++iter)
          {
            if (iter->IsNotNull())
            {
              mitk::DataNode::Pointer node = mitk::DataNode::New();
              node->SetData(*iter);
              nodes->InsertElement(nodes->Size(), node);
            }
          }

          std::vector< std::string > new_files =  reader->GetReadFiles();
          read_files.insert( read_files.end(), new_files.begin(), new_files.end() );
        }

        for (DataStorage::SetOfObjects::ConstIterator nodeIter = nodes->Begin(), nodeIterEnd = nodes->End();
             nodeIter != nodeIterEnd;
             ++nodeIter)
        {
          const mitk::DataNode::Pointer &node = nodeIter->Value();
          mitk::BaseData::Pointer data = node->GetData();
          if (data.IsNull())
          {
            continue;
          }

          mitk::StringProperty::Pointer pathProp = mitk::StringProperty::New(loadInfo.m_Path);
          data->SetProperty("path", pathProp);

          loadInfo.m_Output.push_back(data);
          if (nodeResult)
          {
            nodeResult->push_back(nodeIter->Value());
          }
        }

        if (loadInfo.m_Output.empty() || (nodeResult && nodeResult->Size() == 0))
        {
          errMsg += "Unknown read error occurred reading " + loadInfo.m_Path;
        }
      }
      catch (const std::exception &e)
      {
        errMsg += "Exception occured when reading file " + loadInfo.m_Path + ":\n" + e.what() + "\n\n";
      }
      mitk::ProgressBar::GetInstance()->Progress(2);
      --filesToRead;
    }

    if (!errMsg.empty())
    {
      MITK_ERROR << errMsg;
    }

    mitk::ProgressBar::GetInstance()->Progress(2 * filesToRead);

    return errMsg;
  }

  std::vector<BaseData::Pointer> IOUtil::Load(const us::ModuleResource &usResource, std::ios_base::openmode mode)
  {
    us::ModuleResourceStream resStream(usResource, mode);

    mitk::CoreServicePointer<mitk::IMimeTypeProvider> mimeTypeProvider(mitk::CoreServices::GetMimeTypeProvider());
    std::vector<MimeType> mimetypes = mimeTypeProvider->GetMimeTypesForFile(usResource.GetResourcePath());

    std::vector<mitk::BaseData::Pointer> data;
    if (mimetypes.empty())
    {
      mitkThrow() << "No mimetype for resource stream: " << usResource.GetResourcePath();
      return data;
    }

    mitk::FileReaderRegistry fileReaderRegistry;
    std::vector<us::ServiceReference<IFileReader>> refs = fileReaderRegistry.GetReferences(mimetypes[0]);
    if (refs.empty())
    {
      mitkThrow() << "No reader available for resource stream: " << usResource.GetResourcePath();
      return data;
    }

    mitk::IFileReader *reader = fileReaderRegistry.GetReader(refs[0]);
    reader->SetInput(usResource.GetResourcePath(), &resStream);
    data = reader->Read();

    return data;
  }

  void IOUtil::Save(const BaseData *data, const std::string &path, bool setPathProperty) { Save(data, path, IFileWriter::Options(), setPathProperty); }
  void IOUtil::Save(const BaseData *data, const std::string &path, const IFileWriter::Options &options, bool setPathProperty)
  {
    Save(data, std::string(), path, options, setPathProperty);
  }

  void IOUtil::Save(const BaseData *data, const std::string &mimeType, const std::string &path, bool addExtension, bool setPathProperty)
  {
    Save(data, mimeType, path, IFileWriter::Options(), addExtension, setPathProperty);
  }

  void IOUtil::Save(const BaseData *data,
                    const std::string &mimeType,
                    const std::string &path,
                    const IFileWriter::Options &options,
                    bool addExtension,
                    bool setPathProperty)
  {
    if ((data == nullptr) || (data->IsEmpty()))
      mitkThrow() << "BaseData cannotbe null or empty for save methods in IOUtil.h.";

    std::string errMsg;
    if (options.empty())
    {
      errMsg = Save(data, mimeType, path, nullptr, addExtension, setPathProperty);
    }
    else
    {
      Impl::FixedWriterOptionsFunctor optionsCallback(options);
      errMsg = Save(data, mimeType, path, &optionsCallback, addExtension, setPathProperty);
    }

    if (!errMsg.empty())
    {
      mitkThrow() << errMsg;
    }
  }

  void IOUtil::Save(std::vector<IOUtil::SaveInfo> &saveInfos, bool setPathProperty)
  {
    std::string errMsg = Save(saveInfos, nullptr, setPathProperty);
    if (!errMsg.empty())
    {
      mitkThrow() << errMsg;
    }
  }

  std::string IOUtil::Save(const BaseData *data,
                           const std::string &mimeTypeName,
                           const std::string &path,
                           WriterOptionsFunctorBase *optionsCallback,
                           bool addExtension,
                           bool setPathProperty)
  {
    if (path.empty())
    {
      return "No output filename given";
    }

    mitk::CoreServicePointer<mitk::IMimeTypeProvider> mimeTypeProvider(mitk::CoreServices::GetMimeTypeProvider());

    MimeType mimeType = mimeTypeProvider->GetMimeTypeForName(mimeTypeName);

    SaveInfo saveInfo(data, mimeType, path);

    std::string ext = itksys::SystemTools::GetFilenameExtension(path);

    if (saveInfo.m_WriterSelector.IsEmpty())
    {
      return std::string("No suitable writer found for the current data of type ") + data->GetNameOfClass() +
             (mimeType.IsValid() ? (std::string(" and mime-type ") + mimeType.GetName()) : std::string()) +
             (ext.empty() ? std::string() : (std::string(" with extension ") + ext));
    }

    // Add an extension if not already specified
    if (ext.empty() && addExtension)
    {
      saveInfo.m_MimeType.GetExtensions().empty() ? std::string() : "." + saveInfo.m_MimeType.GetExtensions().front();
    }

    std::vector<SaveInfo> infos;
    infos.push_back(saveInfo);
    return Save(infos, optionsCallback, setPathProperty);
  }

  std::string IOUtil::Save(std::vector<SaveInfo> &saveInfos, WriterOptionsFunctorBase *optionsCallback, bool setPathProperty)
  {
    if (saveInfos.empty())
    {
      return "No data for saving available";
    }

    int filesToWrite = saveInfos.size();
    mitk::ProgressBar::GetInstance()->AddStepsToDo(2 * filesToWrite);

    std::string errMsg;

    std::set<SaveInfo> usedSaveInfos;

    for (auto &saveInfo : saveInfos)
    {
      const std::string baseDataType = saveInfo.m_BaseData->GetNameOfClass();

      std::vector<FileWriterSelector::Item> writers = saveInfo.m_WriterSelector.Get();

      // Error out if no compatible Writer was found
      if (writers.empty())
      {
        errMsg += std::string("No writer available for ") + baseDataType + " data.\n";
        continue;
      }

      bool callOptionsCallback = writers.size() > 1 || !writers[0].GetWriter()->GetOptions().empty();

      // check if we already used a writer for this base data type
      // which should be re-used
      auto oldSaveInfoIter = usedSaveInfos.find(saveInfo);
      if (oldSaveInfoIter != usedSaveInfos.end())
      {
        // we previously saved a base data object of the same data with the same mime-type,
        // check if the same writer is contained in the current writer set and if the
        // confidence level matches
        FileWriterSelector::Item oldSelectedItem =
          oldSaveInfoIter->m_WriterSelector.Get(oldSaveInfoIter->m_WriterSelector.GetSelectedId());
        for (std::vector<FileWriterSelector::Item>::const_iterator currWriterItem = writers.begin(),
                                                                   currWriterItemEnd = writers.end();
             currWriterItem != currWriterItemEnd;
             ++currWriterItem)
        {
          if (currWriterItem->GetServiceId() == oldSelectedItem.GetServiceId() &&
              currWriterItem->GetConfidenceLevel() >= oldSelectedItem.GetConfidenceLevel())
          {
            // okay, we used the same writer already, re-use its options
            callOptionsCallback = false;
            saveInfo.m_WriterSelector.Select(oldSaveInfoIter->m_WriterSelector.GetSelectedId());
            saveInfo.m_WriterSelector.GetSelected().GetWriter()->SetOptions(oldSelectedItem.GetWriter()->GetOptions());
            break;
          }
        }
      }

      if (callOptionsCallback && optionsCallback)
      {
        callOptionsCallback = (*optionsCallback)(saveInfo);
        if (!callOptionsCallback && !saveInfo.m_Cancel)
        {
          usedSaveInfos.erase(saveInfo);
          usedSaveInfos.insert(saveInfo);
        }
      }

      if (saveInfo.m_Cancel)
      {
        errMsg += "Writing operation(s) cancelled.";
        break;
      }

      IFileWriter *writer = saveInfo.m_WriterSelector.GetSelected().GetWriter();
      if (writer == nullptr)
      {
        errMsg += "Unexpected nullptr writer.";
        break;
      }

      // Do the actual writing
      try
      {
        writer->SetOutputLocation(saveInfo.m_Path);
        writer->Write();
      }
      catch (const std::exception &e)
      {
        errMsg += std::string("Exception occurred when writing to ") + saveInfo.m_Path + ":\n" + e.what() + "\n";
      }

      if (setPathProperty)
        saveInfo.m_BaseData->GetPropertyList()->SetStringProperty("path", saveInfo.m_Path.c_str());

      mitk::ProgressBar::GetInstance()->Progress(2);
      --filesToWrite;
    }

    if (!errMsg.empty())
    {
      MITK_ERROR << errMsg;
    }

    mitk::ProgressBar::GetInstance()->Progress(2 * filesToWrite);

    return errMsg;
  }

  // This method can be removed after the deprecated LoadDataNode() method was removed
  void IOUtil::Impl::SetDefaultDataNodeProperties(DataNode *node, const std::string &filePath)
  {
    // path
    mitk::StringProperty::Pointer pathProp = mitk::StringProperty::New(itksys::SystemTools::GetFilenamePath(filePath));
    node->SetProperty(StringProperty::PATH, pathProp);

    // name already defined?
    mitk::StringProperty::Pointer nameProp = dynamic_cast<mitk::StringProperty *>(node->GetProperty("name"));
    if (nameProp.IsNull() || nameProp->GetValue() == DataNode::NO_NAME_VALUE())
    {
      // name already defined in BaseData
      mitk::StringProperty::Pointer baseDataNameProp =
        dynamic_cast<mitk::StringProperty *>(node->GetData()->GetProperty("name").GetPointer());
      if (baseDataNameProp.IsNull() || baseDataNameProp->GetValue() == DataNode::NO_NAME_VALUE())
      {
        // name neither defined in node, nor in BaseData -> name = filename
        nameProp = mitk::StringProperty::New(itksys::SystemTools::GetFilenameWithoutExtension(filePath));
        node->SetProperty("name", nameProp);
      }
      else
      {
        // name defined in BaseData!
        nameProp = mitk::StringProperty::New(baseDataNameProp->GetValue());
        node->SetProperty("name", nameProp);
      }
    }

    // visibility
    if (!node->GetProperty("visible"))
    {
      node->SetVisibility(true);
    }
  }

  IOUtil::SaveInfo::SaveInfo(const BaseData *baseData, const MimeType &mimeType, const std::string &path)
    : m_BaseData(baseData),
      m_WriterSelector(baseData, mimeType.GetName(), path),
      m_MimeType(mimeType.IsValid() ? mimeType // use the original mime-type
                                      :
                                      (m_WriterSelector.IsEmpty() ?
                                         mimeType // no writer found, use the original invalid mime-type
                                         :
                                         m_WriterSelector.GetDefault().GetMimeType() // use the found default mime-type
                                       )),
      m_Path(path),
      m_Cancel(false)
  {
  }

  bool IOUtil::SaveInfo::operator<(const IOUtil::SaveInfo &other) const
  {
    int r = strcmp(m_BaseData->GetNameOfClass(), other.m_BaseData->GetNameOfClass());
    if (r == 0)
    {
      return m_WriterSelector.GetSelected().GetMimeType() < other.m_WriterSelector.GetSelected().GetMimeType();
    }
    return r < 0;
  }

  IOUtil::LoadInfo::LoadInfo(const std::string &path) : m_Path(path), m_ReaderSelector(path), m_Cancel(false) {}
}
