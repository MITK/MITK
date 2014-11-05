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

#include <usGetModuleContext.h>
#include <usModuleContext.h>
#include <usLDAPProp.h>
#include <mitkStandaloneDataStorage.h>
#include <mitkIDataNodeReader.h>
#include <mitkProgressBar.h>
#include <mitkExceptionMacro.h>
#include <mitkCoreObjectFactory.h>
#include <mitkFileReaderRegistry.h>
#include <mitkFileWriterRegistry.h>
#include <mitkCoreServices.h>
#include <mitkIMimeTypeProvider.h>

//ITK
#include <itksys/SystemTools.hxx>

//VTK
#include <vtkPolyData.h>
#include <vtkTriangleFilter.h>
#include <vtkSmartPointer.h>

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

  FormatMessage(
    FORMAT_MESSAGE_ALLOCATE_BUFFER |
    FORMAT_MESSAGE_FROM_SYSTEM |
    FORMAT_MESSAGE_IGNORE_INSERTS,
    NULL,
    dw,
    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
    (LPTSTR) &lpMsgBuf,
    0, NULL );

  std::string errMsg((LPCTSTR)lpMsgBuf);

  LocalFree(lpMsgBuf);

  return errMsg;
#endif
}

#ifdef US_PLATFORM_WINDOWS

#include <io.h>
#include <direct.h>

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
static int mkstemps_compat(char* tmpl, int suffixlen)
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
  char* XXXXXX = &tmpl[len - 6 - suffixlen];

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
    unsigned long long randomTimeBits = ((static_cast<unsigned long long>(ftNow.dwHighDateTime) << 32)
                                         | static_cast<unsigned long long>(ftNow.dwLowDateTime));
    value = randomTimeBits ^ static_cast<unsigned long long>(GetCurrentThreadId());
  }
#else
  {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    unsigned long long randomTimeBits = ((static_cast<unsigned long long>(tv.tv_usec) << 32)
                                         | static_cast<unsigned long long>(tv.tv_sec));
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

    int fd = open (tmpl, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
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
static char* mkdtemps_compat(char* tmpl, int suffixlen)
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
    return NULL;
  }

  /* This is where the Xs start.  */
  char* XXXXXX = &tmpl[len - 6 - suffixlen];

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
      return NULL;
    }
    unsigned long long randomTimeBits = ((static_cast<unsigned long long>(ftNow.dwHighDateTime) << 32)
                                         | static_cast<unsigned long long>(ftNow.dwLowDateTime));
    value = randomTimeBits ^ static_cast<unsigned long long>(GetCurrentThreadId());
  }
#else
  {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    unsigned long long randomTimeBits = ((static_cast<unsigned long long>(tv.tv_usec) << 32)
                                         | static_cast<unsigned long long>(tv.tv_sec));
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
    int fd = _mkdir (tmpl); //, _S_IREAD | _S_IWRITE | _S_IEXEC);
#else
    int fd = mkdir (tmpl, S_IRUSR | S_IWUSR | S_IXUSR);
#endif
    if (fd >= 0)
    {
      errno = savedErrno;
      return tmpl;
    }
    else if (errno != EEXIST)
    {
      return NULL;
    }
  }

  /* We got out of the loop because we ran out of combinations to try.  */
  errno = EEXIST;
  return NULL;
}

//#endif

//**************************************************************
// mitk::IOUtil method definitions

namespace mitk {

const std::string IOUtil::DEFAULTIMAGEEXTENSION = ".nrrd";
const std::string IOUtil::DEFAULTSURFACEEXTENSION = ".stl";
const std::string IOUtil::DEFAULTPOINTSETEXTENSION = ".mps";

struct IOUtil::Impl
{
  struct FixedReaderOptionsFunctor : public ReaderOptionsFunctorBase
  {
    FixedReaderOptionsFunctor(const IFileReader::Options& options)
      : m_Options(options)
    {}

    virtual bool operator()(LoadInfo& loadInfo)
    {
      IFileReader* reader = loadInfo.m_ReaderSelector.GetSelected().GetReader();
      if (reader)
      {
        reader->SetOptions(m_Options);
      }
      return false;
    }

  private:
    const IFileReader::Options& m_Options;
  };

  struct FixedWriterOptionsFunctor : public WriterOptionsFunctorBase
  {
    FixedWriterOptionsFunctor(const IFileReader::Options& options)
      : m_Options(options)
    {}

    virtual bool operator()(SaveInfo& saveInfo)
    {
      IFileWriter* writer = saveInfo.m_WriterSelector.GetSelected().GetWriter();
      if (writer)
      {
        writer->SetOptions(m_Options);
      }
      return false;
    }

  private:
    const IFileWriter::Options& m_Options;
  };

  static BaseData::Pointer LoadBaseDataFromFile(const std::string& path);

  static void SetDefaultDataNodeProperties(mitk::DataNode* node, const std::string& filePath = std::string());
};

#ifdef US_PLATFORM_WINDOWS
std::string IOUtil::GetProgramPath()
{
  char path[512];
  std::size_t index = std::string(path, GetModuleFileName(NULL, path, 512)).find_last_of('\\');
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
    //const char* execPath = strPath.c_str();
    //mitk::StandardFileLocations::GetInstance()->AddDirectoryForSearch(execPath,false);
    return strPath;
  }
  return std::string();
}
#else
#include <sys/types.h>
#include <unistd.h>
#include <sstream>
std::string IOUtil::GetProgramPath()
{
  std::stringstream ss;
  ss << "/proc/" << getpid() << "/exe";
  char proc[512] = {0};
  ssize_t ch = readlink(ss.str().c_str(), proc, 512);
  if (ch == -1) return std::string();
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

std::string IOUtil::CreateTemporaryFile(const std::string& templateName, std::string path)
{
  ofstream tmpOutputStream;
  std::string returnValue = CreateTemporaryFile(tmpOutputStream,templateName,path);
  tmpOutputStream.close();
  return returnValue;
}

std::string IOUtil::CreateTemporaryFile(std::ofstream& f, const std::string& templateName, std::string path)
{
  return CreateTemporaryFile(f, std::ios_base::out | std::ios_base::trunc, templateName, path);
}

std::string IOUtil::CreateTemporaryFile(std::ofstream& f, std::ios_base::openmode mode, const std::string& templateName, std::string path)
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
  int firstNonX = firstX == std::string::npos ? - 1 : firstX - 1;
  while (lastX != std::string::npos && (lastX - firstNonX) < 6)
  {
    lastX = path.find_last_of('X', firstX);
    firstX = path.find_last_not_of('X', lastX);
    firstNonX = firstX == std::string::npos ? - 1 : firstX - 1;
  }
  std::size_t suffixlen = lastX == std::string::npos ? path.size() : path.size() - lastX  - 1;

  int fd = mkstemps_compat(&dst_path[0], suffixlen);
  if(fd != -1)
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

std::string IOUtil::CreateTemporaryDirectory(const std::string& templateName, std::string path)
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
  int firstNonX = firstX == std::string::npos ? - 1 : firstX - 1;
  while (lastX != std::string::npos && (lastX - firstNonX) < 6)
  {
    lastX = path.find_last_of('X', firstX);
    firstX = path.find_last_not_of('X', lastX);
    firstNonX = firstX == std::string::npos ? - 1 : firstX - 1;
  }
  std::size_t suffixlen = lastX == std::string::npos ? path.size() : path.size() - lastX  - 1;

  if(mkdtemps_compat(&dst_path[0], suffixlen) == NULL)
  {
    mitkThrow() << "Creating temporary directory " << &dst_path[0] << " failed: " << GetLastErrorStr();
  }

  path.assign(dst_path.begin(), dst_path.end() - 1);
  return path;
}

DataStorage::SetOfObjects::Pointer IOUtil::Load(const std::string& path, DataStorage& storage)
{
  std::vector<std::string> paths;
  paths.push_back(path);
  return Load(paths, storage);
}

DataStorage::SetOfObjects::Pointer IOUtil::Load(const std::string& path,
                                                const IFileReader::Options& options,
                                                DataStorage& storage)
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

std::vector<BaseData::Pointer> IOUtil::Load(const std::string& path)
{
  std::vector<std::string> paths;
  paths.push_back(path);
  return Load(paths);
}

std::vector<BaseData::Pointer> IOUtil::Load(const std::string& path, const IFileReader::Options& options)
{
  std::vector<LoadInfo> loadInfos;
  loadInfos.push_back(LoadInfo(path));
  Impl::FixedReaderOptionsFunctor optionsCallback(options);
  std::string errMsg = Load(loadInfos, NULL, NULL, &optionsCallback);
  if (!errMsg.empty())
  {
    mitkThrow() << errMsg;
  }
  return loadInfos.front().m_Output;
}

DataStorage::SetOfObjects::Pointer IOUtil::Load(const std::vector<std::string>& paths, DataStorage& storage)
{
  DataStorage::SetOfObjects::Pointer nodeResult = DataStorage::SetOfObjects::New();
  std::vector<LoadInfo> loadInfos;
  for (std::vector<std::string>::const_iterator iter = paths.begin(), iterEnd = paths.end();
       iter != iterEnd; ++iter)
  {
    LoadInfo loadInfo(*iter);
    loadInfos.push_back(loadInfo);
  }
  std::string errMsg = Load(loadInfos, nodeResult, &storage, NULL);
  if (!errMsg.empty())
  {
    mitkThrow() << errMsg;
  }
  return nodeResult;
}

std::vector<BaseData::Pointer> IOUtil::Load(const std::vector<std::string>& paths)
{
  std::vector<BaseData::Pointer> result;
  std::vector<LoadInfo> loadInfos;
  for (std::vector<std::string>::const_iterator iter = paths.begin(), iterEnd = paths.end();
       iter != iterEnd; ++iter)
  {
    LoadInfo loadInfo(*iter);
    loadInfos.push_back(loadInfo);
  }
  std::string errMsg = Load(loadInfos, NULL, NULL, NULL);
  if (!errMsg.empty())
  {
    mitkThrow() << errMsg;
  }

  for (std::vector<LoadInfo>::const_iterator iter = loadInfos.begin(), iterEnd = loadInfos.end();
       iter != iterEnd; ++iter)
  {
    result.insert(result.end(), iter->m_Output.begin(), iter->m_Output.end());
  }
  return result;
}

int IOUtil::LoadFiles(const std::vector<std::string> &fileNames, DataStorage& ds)
{
  return static_cast<int>(Load(fileNames, ds)->Size());
}

DataStorage::Pointer IOUtil::LoadFiles(const std::vector<std::string>& fileNames)
{
  mitk::StandaloneDataStorage::Pointer ds = mitk::StandaloneDataStorage::New();
  Load(fileNames, *ds);
  return ds.GetPointer();
}

BaseData::Pointer IOUtil::LoadBaseData(const std::string& path)
{
  return Impl::LoadBaseDataFromFile(path);
}

BaseData::Pointer IOUtil::Impl::LoadBaseDataFromFile(const std::string& path)
{
  std::vector<BaseData::Pointer> baseDataList = Load(path);

  // The Load(path) call above should throw an exception if nothing could be loaded
  assert(!baseDataList.empty());
  return baseDataList.front();
}

DataNode::Pointer IOUtil::LoadDataNode(const std::string& path)
{
  BaseData::Pointer baseData = Impl::LoadBaseDataFromFile(path);

  mitk::DataNode::Pointer node = mitk::DataNode::New();
  node->SetData(baseData);
  Impl::SetDefaultDataNodeProperties(node, path);

  return node;
}

Image::Pointer IOUtil::LoadImage(const std::string& path)
{
  BaseData::Pointer baseData = Impl::LoadBaseDataFromFile(path);
  mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(baseData.GetPointer());
  if(image.IsNull())
  {
    mitkThrow() << path << " is not a mitk::Image but a " << baseData->GetNameOfClass();
  }
  return image;
}

Surface::Pointer IOUtil::LoadSurface(const std::string& path)
{
  BaseData::Pointer baseData = Impl::LoadBaseDataFromFile(path);
  mitk::Surface::Pointer surface = dynamic_cast<mitk::Surface*>(baseData.GetPointer());
  if(surface.IsNull())
  {
    mitkThrow() << path << " is not a mitk::Surface but a " << baseData->GetNameOfClass();
  }
  return surface;
}

PointSet::Pointer IOUtil::LoadPointSet(const std::string& path)
{
  BaseData::Pointer baseData = Impl::LoadBaseDataFromFile(path);
  mitk::PointSet::Pointer pointset = dynamic_cast<mitk::PointSet*>(baseData.GetPointer());
  if(pointset.IsNull())
  {
    mitkThrow() << path << " is not a mitk::PointSet but a " << baseData->GetNameOfClass();
  }
  return pointset;
}

std::string IOUtil::Load(std::vector<LoadInfo>& loadInfos,
                         DataStorage::SetOfObjects* nodeResult, DataStorage* ds,
                         ReaderOptionsFunctorBase* optionsCallback)
{
  if (loadInfos.empty())
  {
    return "No input files given";
  }

  int filesToRead = loadInfos.size();
  mitk::ProgressBar::GetInstance()->AddStepsToDo(2*filesToRead);

  std::string errMsg;

  std::map<std::string, FileReaderSelector::Item> usedReaderItems;

  for(std::vector<LoadInfo>::iterator infoIter = loadInfos.begin(),
      infoIterEnd = loadInfos.end(); infoIter != infoIterEnd; ++infoIter)
  {
    std::vector<FileReaderSelector::Item> readers = infoIter->m_ReaderSelector.Get();

    if (readers.empty())
    {
      if (!itksys::SystemTools::FileExists(infoIter->m_Path.c_str()))
      {
        errMsg += "File '" + infoIter->m_Path + "' does not exist\n";
      }
      else
      {
        errMsg += "No reader available for '" + infoIter->m_Path + "'\n";
      }
      continue;
    }

    bool callOptionsCallback = readers.size() > 1 || !readers.front().GetReader()->GetOptions().empty();

    // check if we already used a reader which should be re-used
    std::vector<MimeType> currMimeTypes = infoIter->m_ReaderSelector.GetMimeTypes();
    std::string selectedMimeType;
    for (std::vector<MimeType>::const_iterator mimeTypeIter = currMimeTypes.begin(),
         mimeTypeIterEnd = currMimeTypes.end(); mimeTypeIter != mimeTypeIterEnd; ++mimeTypeIter)
    {
      std::map<std::string, FileReaderSelector::Item>::const_iterator oldSelectedItemIter =
          usedReaderItems.find(mimeTypeIter->GetName());
      if (oldSelectedItemIter != usedReaderItems.end())
      {
        // we found an already used item for a mime-type which is contained
        // in the current reader set, check all current readers if there service
        // id equals the old reader
        for (std::vector<FileReaderSelector::Item>::const_iterator currReaderItem = readers.begin(),
             currReaderItemEnd = readers.end(); currReaderItem != currReaderItemEnd; ++currReaderItem)
        {
          if (currReaderItem->GetMimeType().GetName() == mimeTypeIter->GetName() &&
              currReaderItem->GetServiceId() == oldSelectedItemIter->second.GetServiceId() &&
              currReaderItem->GetConfidenceLevel() >= oldSelectedItemIter->second.GetConfidenceLevel())
          {
            // okay, we used the same reader already, re-use its options
            selectedMimeType = mimeTypeIter->GetName();
            callOptionsCallback = false;
            infoIter->m_ReaderSelector.Select(oldSelectedItemIter->second.GetServiceId());
            infoIter->m_ReaderSelector.GetSelected().GetReader()->SetOptions(
                  oldSelectedItemIter->second.GetReader()->GetOptions());
            break;
          }
        }
        if (!selectedMimeType.empty()) break;
      }
    }

    if (callOptionsCallback && optionsCallback)
    {
      callOptionsCallback = (*optionsCallback)(*infoIter);
      if (!callOptionsCallback && !infoIter->m_Cancel)
      {
        usedReaderItems.erase(selectedMimeType);
        FileReaderSelector::Item selectedItem = infoIter->m_ReaderSelector.GetSelected();
        usedReaderItems.insert(std::make_pair(selectedItem.GetMimeType().GetName(),
                                              selectedItem));
      }
    }

    if (infoIter->m_Cancel)
    {
      errMsg += "Reading operation(s) cancelled.";
      break;
    }

    IFileReader* reader = infoIter->m_ReaderSelector.GetSelected().GetReader();
    if (reader == NULL)
    {
      errMsg += "Unexpected NULL reader.";
      break;
    }

    // Do the actual reading
    try
    {
      DataStorage::SetOfObjects::Pointer nodes;
      if (ds != NULL)
      {
        nodes = reader->Read(*ds);
      }
      else
      {
        nodes = DataStorage::SetOfObjects::New();
        std::vector<mitk::BaseData::Pointer> baseData = reader->Read();
        for (std::vector<mitk::BaseData::Pointer>::iterator iter = baseData.begin();
             iter != baseData.end(); ++iter)
        {
          if (iter->IsNotNull())
          {
            mitk::DataNode::Pointer node = mitk::DataNode::New();
            node->SetData(*iter);
            nodes->InsertElement(nodes->Size(), node);
          }
        }
      }

      for (DataStorage::SetOfObjects::ConstIterator nodeIter = nodes->Begin(),
           nodeIterEnd = nodes->End(); nodeIter != nodeIterEnd; ++nodeIter)
      {
        const mitk::DataNode::Pointer& node = nodeIter->Value();
        mitk::BaseData::Pointer data = node->GetData();
        if (data.IsNull())
        {
          continue;
        }

        mitk::StringProperty::Pointer pathProp = mitk::StringProperty::New(infoIter->m_Path);
        data->SetProperty("path", pathProp);

        infoIter->m_Output.push_back(data);
        if (nodeResult)
        {
          nodeResult->push_back(nodeIter->Value());
        }
      }

      if (infoIter->m_Output.empty() || (nodeResult && nodeResult->Size() == 0))
      {
        errMsg += "Unknown read error occurred reading " + infoIter->m_Path;
      }
    }
    catch (const std::exception& e)
    {
      errMsg += "Exception occured when reading file " + infoIter->m_Path + ":\n" + e.what() + "\n\n";
    }
    mitk::ProgressBar::GetInstance()->Progress(2);
    --filesToRead;
  }

  if (!errMsg.empty())
  {
    MITK_ERROR << errMsg;
  }

  mitk::ProgressBar::GetInstance()->Progress(2*filesToRead);

  return errMsg;
}

void IOUtil::Save(const BaseData* data, const std::string& path)
{
  Save(data, path, IFileWriter::Options());
}

void IOUtil::Save(const BaseData* data, const std::string& path, const IFileWriter::Options& options)
{
  Save(data, std::string(), path, options);
}

void IOUtil::Save(const BaseData* data, const std::string& mimeType, const std::string& path,
                  bool addExtension)
{
  Save(data, mimeType, path, IFileWriter::Options(), addExtension);
}

void IOUtil::Save(const BaseData* data, const std::string& mimeType, const std::string& path,
                  const IFileWriter::Options& options, bool addExtension)
{
  std::string errMsg;
  if (options.empty())
  {
    errMsg = Save(data, mimeType, path, NULL, addExtension);
  }
  else
  {
    Impl::FixedWriterOptionsFunctor optionsCallback(options);
    errMsg = Save(data, mimeType, path, &optionsCallback, addExtension);
  }

  if (!errMsg.empty())
  {
    mitkThrow() << errMsg;
  }
}

void IOUtil::Save(std::vector<IOUtil::SaveInfo>& saveInfos)
{
  std::string errMsg = Save(saveInfos, NULL);
  if (!errMsg.empty())
  {
    mitkThrow() << errMsg;
  }
}

bool IOUtil::SaveImage(mitk::Image::Pointer image, const std::string& path)
{
  Save(image, path);
  return true;
}

bool IOUtil::SaveSurface(Surface::Pointer surface, const std::string& path)
{
  Save(surface, path);
  return true;
}

bool IOUtil::SavePointSet(PointSet::Pointer pointset, const std::string& path)
{
  Save(pointset, path);
  return true;
}

bool IOUtil::SaveBaseData( mitk::BaseData* data, const std::string& path)
{
  Save(data, path);
  return true;
}

std::string IOUtil::Save(const BaseData* data, const std::string& mimeTypeName, const std::string& path,
                         WriterOptionsFunctorBase* optionsCallback, bool addExtension)
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
    ext = saveInfo.m_MimeType.GetExtensions().empty() ? std::string() : "." + saveInfo.m_MimeType.GetExtensions().front();
  }

  std::vector<SaveInfo> infos;
  infos.push_back(saveInfo);
  return Save(infos, optionsCallback);
}

std::string IOUtil::Save(std::vector<SaveInfo>& saveInfos, WriterOptionsFunctorBase* optionsCallback)
{
  if (saveInfos.empty())
  {
    return "No data for saving available";
  }

  int filesToWrite = saveInfos.size();
  mitk::ProgressBar::GetInstance()->AddStepsToDo(2*filesToWrite);

  std::string errMsg;

  std::set<SaveInfo> usedSaveInfos;

  for (std::vector<SaveInfo>::iterator infoIter = saveInfos.begin(),
       infoIterEnd = saveInfos.end(); infoIter != infoIterEnd; ++infoIter)
  {
    const std::string baseDataType = infoIter->m_BaseData->GetNameOfClass();

    std::vector<FileWriterSelector::Item> writers = infoIter->m_WriterSelector.Get();

    // Error out if no compatible Writer was found
    if (writers.empty())
    {
      errMsg += std::string("No writer available for ") + baseDataType + " data.\n";
      continue;
    }

    bool callOptionsCallback = writers.size() > 1 || !writers[0].GetWriter()->GetOptions().empty();

    // check if we already used a writer for this base data type
    // which should be re-used
    std::set<SaveInfo>::const_iterator oldSaveInfoIter = usedSaveInfos.find(*infoIter);
    if (oldSaveInfoIter != usedSaveInfos.end())
    {
      // we previously saved a base data object of the same data with the same mime-type,
      // check if the same writer is contained in the current writer set and if the
      // confidence level matches
      FileWriterSelector::Item oldSelectedItem =
          oldSaveInfoIter->m_WriterSelector.Get(oldSaveInfoIter->m_WriterSelector.GetSelectedId());
      for (std::vector<FileWriterSelector::Item>::const_iterator currWriterItem = writers.begin(),
           currWriterItemEnd = writers.end(); currWriterItem != currWriterItemEnd; ++currWriterItem)
      {
        if (currWriterItem->GetServiceId() == oldSelectedItem.GetServiceId() &&
            currWriterItem->GetConfidenceLevel() >= oldSelectedItem.GetConfidenceLevel())
        {
          // okay, we used the same writer already, re-use its options
          callOptionsCallback = false;
          infoIter->m_WriterSelector.Select(oldSaveInfoIter->m_WriterSelector.GetSelectedId());
          infoIter->m_WriterSelector.GetSelected().GetWriter()->SetOptions(
                oldSelectedItem.GetWriter()->GetOptions());
          break;
        }
      }
    }

    if (callOptionsCallback && optionsCallback)
    {
      callOptionsCallback = (*optionsCallback)(*infoIter);
      if (!callOptionsCallback && !infoIter->m_Cancel)
      {
        usedSaveInfos.erase(*infoIter);
        usedSaveInfos.insert(*infoIter);
      }
    }

    if (infoIter->m_Cancel)
    {
      errMsg += "Writing operation(s) cancelled.";
      break;
    }

    IFileWriter* writer = infoIter->m_WriterSelector.GetSelected().GetWriter();
    if (writer == NULL)
    {
      errMsg += "Unexpected NULL writer.";
      break;
    }

    // Do the actual writing
    try
    {
      writer->SetOutputLocation(infoIter->m_Path);
      writer->Write();
    }
    catch(const std::exception& e)
    {
      errMsg += std::string("Exception occurred when writing to ") + infoIter->m_Path + ":\n" + e.what() + "\n";
    }
    mitk::ProgressBar::GetInstance()->Progress(2);
    --filesToWrite;
  }

  if (!errMsg.empty())
  {
    MITK_ERROR << errMsg;
  }

  mitk::ProgressBar::GetInstance()->Progress(2*filesToWrite);

  return errMsg;
}

// This method can be removed after the deprecated LoadDataNode() method was removed
void IOUtil::Impl::SetDefaultDataNodeProperties(DataNode* node, const std::string& filePath)
{
  // path
  mitk::StringProperty::Pointer pathProp = mitk::StringProperty::New( itksys::SystemTools::GetFilenamePath(filePath) );
  node->SetProperty(StringProperty::PATH, pathProp);

  // name already defined?
  mitk::StringProperty::Pointer nameProp = dynamic_cast<mitk::StringProperty*>(node->GetProperty("name"));
  if(nameProp.IsNull() || (strcmp(nameProp->GetValue(),"No Name!")==0))
  {
    // name already defined in BaseData
    mitk::StringProperty::Pointer baseDataNameProp = dynamic_cast<mitk::StringProperty*>(node->GetData()->GetProperty("name").GetPointer() );
    if(baseDataNameProp.IsNull() || (strcmp(baseDataNameProp->GetValue(),"No Name!")==0))
    {
      // name neither defined in node, nor in BaseData -> name = filename
      nameProp = mitk::StringProperty::New( itksys::SystemTools::GetFilenameWithoutExtension(filePath));
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
  if(!node->GetProperty("visible"))
  {
    node->SetVisibility(true);
  }
}

IOUtil::SaveInfo::SaveInfo(const BaseData* baseData, const MimeType& mimeType,
                           const std::string& path)
  : m_BaseData(baseData)
  , m_WriterSelector(baseData, mimeType.GetName(), path)
  , m_MimeType(mimeType.IsValid() ? mimeType // use the original mime-type
                                  : (m_WriterSelector.IsEmpty() ? mimeType // no writer found, use the original invalid mime-type
                                                                : m_WriterSelector.GetDefault().GetMimeType() // use the found default mime-type
                                                                  )
                                    )
  , m_Path(path)
  , m_Cancel(false)
{
}

bool IOUtil::SaveInfo::operator<(const IOUtil::SaveInfo& other) const
{
  int r = strcmp(m_BaseData->GetNameOfClass(), other.m_BaseData->GetNameOfClass());
  if (r == 0)
  {
    return m_WriterSelector.GetSelected().GetMimeType() <
        other.m_WriterSelector.GetSelected().GetMimeType();
  }
  return r < 0;
}

IOUtil::LoadInfo::LoadInfo(const std::string& path)
  : m_Path(path)
  , m_ReaderSelector(path)
  , m_Cancel(false)
{
}

}
