/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkIOUtil.h>

#include <mitkCoreServices.h>
#include <mitkIDataStorageService.h>
#include <mitkStorageThreadDispatcherBase.h>
#include <mitkExceptionMacro.h>
#include <mitkFileReaderRegistry.h>
#include <mitkFileWriterRegistry.h>
#include <mitkIMimeTypeProvider.h>
#include <mitkProgressTask.h>
#include <mitkStandaloneDataStorage.h>
#include <usGetModuleContext.h>
#include <usLDAPProp.h>
#include <usModuleContext.h>
#include <usModuleResource.h>
#include <usModuleResourceStream.h>
#include <mitkAbstractFileReader.h>
#include <mitkUtf8Util.h>
#include <mitkFileSystem.h>

// ITK
#include <itksys/SystemTools.hxx>

// VTK
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>
#include <vtkTriangleFilter.h>

#include <algorithm>
#include <cerrno>
#include <cstdlib>
#include <optional>

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

// make the posix flags point to the obsolete bsd types on windows
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

namespace
{
  /**
   * Resolution of a single file within its task. Readers and writers report
   * a fraction, and this is what that fraction is spread over.
   */
  constexpr unsigned int STEPS_PER_FILE = 100;

  /** Nesting depth of IOUtil::QuietProgress on this thread. */
  thread_local unsigned int s_QuietDepth = 0;

  /**
   * Runs the task on the thread that owns the data storage, if this is not
   * it. Writing to data that is on display has to happen where everything
   * else reads it, which is not the worker a save may be running on.
   */
  void RunWhereTheDataLives(const std::function<void()>& task)
  {
    mitk::CoreServicePointer<mitk::IDataStorageService> service(mitk::CoreServices::GetDataStorageService());

    auto* dispatcher = service
      ? service->GetDispatcher()
      : nullptr;

    if (nullptr == dispatcher || dispatcher->IsDispatchThread())
    {
      task();
      return;
    }

    dispatcher->Execute(task);
  }

  /**
   * Maps the progress of one reader or writer onto its share of a task, and
   * keeps the callback registered only while that file is being handled.
   *
   * Steps are reported relative rather than absolute, so that a task shared
   * with the caller keeps whatever progress the caller made itself.
   */
  class FileProgressForwarder final
  {
  public:
    FileProgressForwarder(mitk::IFileIO* fileIO, mitk::ProgressTask* task)
      : m_FileIO(nullptr != task ? fileIO : nullptr),
        m_Task(task),
        m_Reported(0),
        m_FileTask([this](float progress) { this->OnProgress(progress); })
    {
      if (nullptr != m_FileIO)
      {
        m_FileIO->AddProgressCallback(mitk::MessageDelegate1<FileProgressForwarder, float>(
          this, &FileProgressForwarder::OnProgress));

        // A reader that reports in steps of its own, or that drives further
        // reads, gets a task rather than a callback. It maps into this
        // file's share, so whatever it adds cannot grow the budget of the
        // operation around it.
        m_FileIO->SetProgressTask(&m_FileTask);
      }
    }

    ~FileProgressForwarder()
    {
      if (nullptr != m_FileIO)
      {
        m_FileIO->SetProgressTask(nullptr);
        m_FileIO->RemoveProgressCallback(mitk::MessageDelegate1<FileProgressForwarder, float>(
          this, &FileProgressForwarder::OnProgress));
      }

      // Whatever the file reported, its share of the work is over. Formats
      // that report nothing at all advance here in one go.
      this->Advance(STEPS_PER_FILE);
    }

    FileProgressForwarder(const FileProgressForwarder&) = delete;
    FileProgressForwarder& operator=(const FileProgressForwarder&) = delete;

  private:
    void OnProgress(float progress)
    {
      const auto clamped = std::clamp(progress, 0.0f, 1.0f);
      this->Advance(static_cast<unsigned int>(clamped * STEPS_PER_FILE));
    }

    void Advance(unsigned int reached)
    {
      if (nullptr != m_Task && reached > m_Reported)
      {
        m_Task->Progress(reached - m_Reported);
        m_Reported = reached;
      }
    }

    mitk::IFileIO* m_FileIO;
    mitk::ProgressTask* m_Task;
    unsigned int m_Reported;
    mitk::ProgressTask m_FileTask;
  };
}

namespace mitk
{
  IOUtil::QuietProgress::QuietProgress()
  {
    ++s_QuietDepth;
  }

  IOUtil::QuietProgress::~QuietProgress()
  {
    --s_QuietDepth;
  }

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

  fs::path IOUtil::GetAppBundlePath([[maybe_unused]] AppBundlePath path)
  {
    fs::path appPath = GetProgramPath();

#if defined(US_PLATFORM_APPLE)
    if (appPath.filename() == "MacOS")
    {
      if (const auto contentsPath = appPath.parent_path(); contentsPath.filename() == "Contents")
      {
        if (const auto bundlePath = contentsPath.parent_path(); bundlePath.extension() == ".app")
        {
          appPath = path == AppBundlePath::Parent
            ? bundlePath.parent_path()
            : bundlePath;
        }
      }
    }
#endif

    return appPath;
  }

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
    try
    {
      auto tmp = fs::temp_directory_path();

      if (tmp.has_filename() == false)
        tmp = tmp.parent_path(); // Remove trailing path separator

      return tmp.string();
    }
    catch (const fs::filesystem_error &e)
    {
      mitkThrow() << e.what();
    }
  }

  std::string IOUtil::CreateTemporaryFile(const std::string &templateName, const std::string &path)
  {
    std::ofstream tmpOutputStream;
    std::string returnValue = CreateTemporaryFile(tmpOutputStream, templateName, path);
    tmpOutputStream.close();
    return returnValue;
  }

  std::string IOUtil::CreateTemporaryFile(std::ofstream &f, const std::string &templateName, const std::string &path)
  {
    return CreateTemporaryFile(f, std::ios_base::out | std::ios_base::trunc, templateName, path);
  }

  std::string IOUtil::CreateTemporaryFile(std::ofstream &f,
                                          std::ios_base::openmode mode,
                                          const std::string &templateName,
                                          std::string path)
  {
    if (path.empty())
      path = GetTempPath();

    path = (fs::path(path) / templateName).string();

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
      path = GetTempPath();

    path = (fs::path(path) / templateName).string();

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
    for (const auto &loadInfo : paths)
    {
      loadInfos.emplace_back(loadInfo);
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
    for (const auto &loadInfo : paths)
    {
      loadInfos.emplace_back(loadInfo);
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
    const auto steps = static_cast<unsigned int>(STEPS_PER_FILE * filesToRead);

    std::optional<ProgressTask> ownTask;

    if (0 == s_QuietDepth)
      ownTask.emplace("Loading files", steps);

    auto* task = ownTask.has_value()
      ? &ownTask.value()
      : nullptr;

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
        if (!itksys::SystemTools::FileExists(Utf8Util::Local8BitToUtf8(loadInfo.m_Path).c_str()))
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

      reader->SetProperties(loadInfo.m_Properties);

      if (nullptr != task)
        task->SetName("Loading " + itksys::SystemTools::GetFilenameName(loadInfo.m_Path));

      // Reports whatever the reader tells it while the file is being read,
      // and covers the rest of the file's share when it goes out of scope.
      FileProgressForwarder fileProgress(reader, task);

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

          data->SetProperty("path", mitk::StringProperty::New(Utf8Util::Local8BitToUtf8(loadInfo.m_Path)));

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
        errMsg += "Exception occurred when reading file " + loadInfo.m_Path + ":\n" + e.what() + "\n\n";
      }
      --filesToRead;
    }

    if (!errMsg.empty())
    {
      MITK_ERROR << errMsg;
    }

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

  BaseData::Pointer IOUtil::Load(const std::string& path, const PropertyList* properties)
  {
    LoadInfo loadInfo(path);
    loadInfo.m_Properties = properties;

    std::vector<LoadInfo> loadInfos;
    loadInfos.push_back(loadInfo);

    auto errMsg = Load(loadInfos, nullptr, nullptr, nullptr);

    if (!errMsg.empty())
      mitkThrow() << errMsg;

    return loadInfos.front().m_Output.front();
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
      mitkThrow() << "BaseData cannot be null or empty for save methods in IOUtil.h.";

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

    std::string ext = Utf8Util::Utf8ToLocal8Bit(itksys::SystemTools::GetFilenameExtension(Utf8Util::Local8BitToUtf8(path)));

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
      saveInfo.m_Path += ext;
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
    std::optional<ProgressTask> ownTask;

    if (0 == s_QuietDepth)
      ownTask.emplace("Saving files", static_cast<unsigned int>(STEPS_PER_FILE * filesToWrite));

    auto* task = ownTask.has_value()
      ? &ownTask.value()
      : nullptr;

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

      if (nullptr != task)
        task->SetName("Saving " + itksys::SystemTools::GetFilenameName(saveInfo.m_Path));

      FileProgressForwarder fileProgress(writer, task);

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
      {
        RunWhereTheDataLives([&saveInfo]()
          {
            saveInfo.m_BaseData->GetPropertyList()->SetStringProperty(
              "path", Utf8Util::Local8BitToUtf8(saveInfo.m_Path).c_str());
          });
      }

      --filesToWrite;
    }

    if (!errMsg.empty())
    {
      MITK_ERROR << errMsg;
    }

    return errMsg;
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

  IOUtil::LoadInfo::LoadInfo(const std::string &path)
    : m_Path(path),
      m_ReaderSelector(path),
      m_Cancel(false),
      m_Properties(nullptr)
  {
  }
}
