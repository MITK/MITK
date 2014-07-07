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

// *****************************************************************
// Utility code

US_HASH_FUNCTION_NAMESPACE_BEGIN
US_HASH_FUNCTION_BEGIN(std::vector<std::string>)
  std::string imploded;
  imploded.reserve(256);
  for(std::vector<std::string>::const_iterator iter = arg.begin(),
      iterEnd = arg.end(); iter != iterEnd; ++iter)
  {
   imploded += *iter;
  }
  return US_HASH_FUNCTION(std::string, imploded);
US_HASH_FUNCTION_END
US_HASH_FUNCTION_NAMESPACE_END

namespace {

class FileSorter
{

public:

  typedef std::vector<std::string> MimeTypeVector;
  typedef std::vector<std::string> FileVector;

  FileSorter()
  {}

private:

  FileSorter(const FileSorter&);
  FileSorter& operator=(const FileSorter&);

  typedef US_UNORDERED_MAP_TYPE<MimeTypeVector, FileVector> HashMapType;

  HashMapType m_MimeTypesToFiles;

public:

  typedef HashMapType::const_iterator Iterator;

  void AddFile(const std::string& file, const std::string& mimeType)
  {
    MimeTypeVector mimeTypes;
    mimeTypes.push_back(mimeType);
    m_MimeTypesToFiles[mimeTypes].push_back(file);
  }

  void AddFile(const std::string& file, const MimeTypeVector& mimeTypes)
  {
    m_MimeTypesToFiles[mimeTypes].push_back(file);
  }

  Iterator Begin() const
  {
    return m_MimeTypesToFiles.begin();
  }

  Iterator End() const
  {
    return m_MimeTypesToFiles.end();
  }

};

}

//**************************************************************
// mitk::IOUtil method definitions

namespace mitk {

const std::string IOUtil::DEFAULTIMAGEEXTENSION = ".nrrd";
const std::string IOUtil::DEFAULTSURFACEEXTENSION = ".stl";
const std::string IOUtil::DEFAULTPOINTSETEXTENSION = ".mps";

struct IOUtil::Impl
{
  struct FixedOptionsFunctor : public OptionsFunctorBase
  {
    FixedOptionsFunctor(const IFileReader::Options& options)
      : m_Options(options)
    {}

    virtual bool operator()(const std::string& /*path*/, const std::vector<std::string>& /*readerLabels*/,
                            const std::vector<IFileReader*>& readers, IFileReader*& selectedReader)
    {
      selectedReader = readers.front();
      selectedReader->SetOptions(m_Options);
      return false;
    }

  private:
    const IFileReader::Options& m_Options;
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

  path += "/" + templateName;
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

  path += "/" + templateName;
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
  std::vector<std::string> paths;
  paths.push_back(path);
  DataStorage::SetOfObjects::Pointer nodeResult = DataStorage::SetOfObjects::New();
  Impl::FixedOptionsFunctor optionsCallback(options);
  std::string errMsg = Load(paths, NULL, nodeResult, &storage, &optionsCallback);
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
  std::vector<std::string> paths;
  paths.push_back(path);
  std::vector<BaseData::Pointer> result;
  Impl::FixedOptionsFunctor optionsCallback(options);
  std::string errMsg = Load(paths, &result, NULL, NULL, &optionsCallback);
  if (!errMsg.empty())
  {
    mitkThrow() << errMsg;
  }
  return result;
}

DataStorage::SetOfObjects::Pointer IOUtil::Load(const std::vector<std::string>& paths, DataStorage& storage)
{
  DataStorage::SetOfObjects::Pointer nodeResult = DataStorage::SetOfObjects::New();
  std::string errMsg = Load(paths, NULL, nodeResult, &storage, NULL);
  if (!errMsg.empty())
  {
    mitkThrow() << errMsg;
  }
  return nodeResult;
}

std::vector<BaseData::Pointer> IOUtil::Load(const std::vector<std::string>& paths)
{
  std::vector<BaseData::Pointer> result;
  std::string errMsg = Load(paths, &result, NULL, NULL, NULL);
  if (!errMsg.empty())
  {
    mitkThrow() << errMsg;
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

std::string IOUtil::Load(const std::vector<std::string>& paths, std::vector<BaseData::Pointer>* result,
                         DataStorage::SetOfObjects* nodeResult, DataStorage* ds,
                         OptionsFunctorBase* optionsCallback)
{
  if (paths.empty())
  {
    return "No input files given";
  }

  mitk::ProgressBar::GetInstance()->AddStepsToDo(2*paths.size());

  CoreServicePointer<IMimeTypeProvider> mimeTypeProvider(CoreServices::GetMimeTypeProvider());

  std::string errMsg;

  // group the selected files by mime-type(s)
  FileSorter fileSorter;
  std::string noMimeTypeError;
  for(std::vector<std::string>::const_iterator selectedFileIter = paths.begin(),
      iterEnd = paths.end(); selectedFileIter != iterEnd; ++selectedFileIter)
  {
    std::vector<std::string> mimeTypes = mimeTypeProvider->GetMimeTypesForFile(*selectedFileIter);
    if (!mimeTypes.empty())
    {
      fileSorter.AddFile(*selectedFileIter, mimeTypes);
    }
    else
    {
      noMimeTypeError += "  * " + *selectedFileIter + "\n";
    }
  }

  if (!noMimeTypeError.empty())
  {
    errMsg += "Unknown file format for\n\n" + noMimeTypeError + "\n\n";
  }

  FileReaderRegistry readerRegistry;

  for(FileSorter::Iterator iter = fileSorter.Begin(), endIter = fileSorter.End();
      iter != endIter; ++iter)
  {
    std::vector<mitk::IFileReader*> finalReaders;
    std::vector<std::string> finalReaderLabels;
    const FileSorter::FileVector& currentFiles = iter->second;
    const FileSorter::MimeTypeVector& fileMimeTypes = iter->first;
    for(FileSorter::MimeTypeVector::const_iterator mimeTypeIter = fileMimeTypes.begin(),
        mimeTypeIterEnd = fileMimeTypes.end(); mimeTypeIter != mimeTypeIterEnd; ++mimeTypeIter)
    {
      // Get all IFileReader references for the given mime-type
      std::vector<mitk::FileReaderRegistry::ReaderReference> readerRefs = readerRegistry.GetReferences(*mimeTypeIter);
      // Sort according to priority (ascending)
      std::sort(readerRefs.begin(), readerRefs.end());

      // Loop over all readers, starting at the highest priority, and check if it
      // really can read the file (well, the first in the list).
      for (std::vector<mitk::FileReaderRegistry::ReaderReference>::reverse_iterator readerRef = readerRefs.rbegin();
           readerRef != readerRefs.rend(); ++readerRef)
      {
        mitk::IFileReader* reader = readerRegistry.GetReader(*readerRef);
        try
        {
          if (reader->CanRead(currentFiles.front()))
          {
            finalReaders.push_back(reader);
            finalReaderLabels.push_back(readerRef->GetProperty(mitk::IFileReader::PROP_DESCRIPTION()).ToString());
          }
        }
        catch (const std::exception& e)
        {
          MITK_ERROR << "Calling CanRead('" << currentFiles.front() << "') on " << typeid(reader).name() << "failed: " << e.what();
        }
      }
    }

    if (finalReaders.empty())
    {
      errMsg += "No reader available for files\n\n";
      for(FileSorter::FileVector::const_iterator fileIter = currentFiles.begin(),
          fileIterEnd = currentFiles.end(); fileIter != fileIterEnd; ++fileIter)
      {
        errMsg += "  * " + *fileIter + "\n";
      }
    }
    else
    {
      // TODO check if the reader can read a series of files and if yes,
      //      pass the complete file list to one "Read" call.

      bool callOptionsCallback = finalReaders.size() > 1 || !finalReaders.front()->GetOptions().empty();
      mitk::IFileReader* selectedReader = finalReaders.front();
      MITK_INFO << "******* USING READER " << typeid(*selectedReader).name() << "*********";
      for(FileSorter::FileVector::const_iterator fileIter = currentFiles.begin(),
          fileIterEnd = currentFiles.end(); fileIter != fileIterEnd; ++fileIter)
      {
        if (callOptionsCallback && optionsCallback)
        {
          callOptionsCallback = (*optionsCallback)(*fileIter, finalReaderLabels, finalReaders, selectedReader);
        }
        if (selectedReader == NULL)
        {
          errMsg += "Reading operation(s) cancelled.";
          break;
        }

        // Do the actual reading
        try
        {
          // Correct conversion for File names.(BUG 12252)
          const std::string& stdFile = *fileIter;

          DataStorage::SetOfObjects::Pointer nodes;
          // Now do the actual reading
          if (ds != NULL)
          {
            nodes = selectedReader->Read(stdFile, *ds);
          }
          else
          {
            nodes = DataStorage::SetOfObjects::New();
            std::vector<mitk::BaseData::Pointer> baseData = selectedReader->Read(stdFile);
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

            mitk::StringProperty::Pointer pathProp = mitk::StringProperty::New(stdFile);
            data->SetProperty("path", pathProp);

            if (result)
            {
              result->push_back(data);
            }
            if (nodeResult)
            {
              nodeResult->push_back(nodeIter->Value());
            }
          }

          if ((result && result->empty()) || (nodeResult && nodeResult->Size() == 0))
          {
            errMsg += "Unknown read error occurred reading " + stdFile;
          }
        }
        catch (const std::exception& e)
        {
          errMsg += "Exception occured when reading file " + *fileIter + ":\n" + e.what() + "\n\n";
        }
        mitk::ProgressBar::GetInstance()->Progress(2);
      }
    }

    for(std::vector<mitk::IFileReader*>::const_iterator readerIter = finalReaders.begin(),
        readerIterEnd = finalReaders.end(); readerIter != readerIterEnd; ++readerIter)
    {
      readerRegistry.UngetReader(*readerIter);
    }
  }

  if (!errMsg.empty())
  {
    MITK_ERROR << errMsg;
  }

  return errMsg;
}

void IOUtil::Save(BaseData* data, const std::string& path)
{
  Save(data, path, IFileWriter::Options());
}

void IOUtil::Save(BaseData* data, const std::string& path, const IFileWriter::Options& options)
{
  if (data == NULL)
  {
    mitkThrow() << "Cannot write a NULL" << data->GetNameOfClass() << " object.";
  }
  if (path.empty())
  {
    mitkThrow() << "Cannot write a " << data->GetNameOfClass() << " object to an empty path.";
  }

  FileWriterRegistry writerRegistry;
  us::ServiceReference<IFileWriter> writerRef = writerRegistry.GetReference(data);
  // Throw exception if no compatible Writer was found
  if (!writerRef)
  {
    mitkThrow() << "No writer for " << data->GetNameOfClass() << " available.";
  }

//  std::string extension = itksys::SystemTools::GetFilenameExtension( path );
//  if(extension.empty())
//  {
//    std::string defaultExtension = writerRegistry.GetDefaultExtension(writerRef);
//    MITK_WARN << "Using default extension '" << defaultExtension << "' for writing to " << path;
//    extension = "." + defaultExtension;
//  }

  IFileWriter* writer = writerRegistry.GetWriter(writerRef);
  if (writer == NULL)
  {
    mitkThrow() << "Got a NULL IFileWriter service for writing " << data->GetNameOfClass();
  }

  try
  {
    if (!options.empty())
    {
      writer->SetOptions(options);
    }
    writer->Write(data, path);
  }
  catch(const std::exception& e)
  {
    mitkThrow() << " Writing a " << data->GetNameOfClass() << " to " << path << " failed: "
                << e.what();
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

}
