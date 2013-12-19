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

    int fd = open (tmpl, O_RDWR | O_CREAT | O_EXCL, S_IREAD | S_IWRITE);
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

namespace mitk {

const std::string IOUtil::DEFAULTIMAGEEXTENSION = ".nrrd";
const std::string IOUtil::DEFAULTSURFACEEXTENSION = ".stl";
const std::string IOUtil::DEFAULTPOINTSETEXTENSION = ".mps";

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

int IOUtil::LoadFiles(const std::vector<std::string> &fileNames, DataStorage &ds)
{
    // Get the set of registered mitk::IDataNodeReader services
    us::ModuleContext* context = us::GetModuleContext();
    const std::vector<us::ServiceReference<IDataNodeReader> > refs = context->GetServiceReferences<IDataNodeReader>();
    std::vector<IDataNodeReader*> services;
    services.reserve(refs.size());
    for (std::vector<us::ServiceReference<IDataNodeReader> >::const_iterator i = refs.begin();
         i != refs.end(); ++i)
    {
        IDataNodeReader* s = context->GetService(*i);
        if (s != 0)
        {
            services.push_back(s);
        }
    }

    mitk::ProgressBar::GetInstance()->AddStepsToDo(2*fileNames.size());

    // Iterate over all file names and use the IDataNodeReader services
    // to load them.
    int nodesRead = 0;
    for (std::vector<std::string>::const_iterator i = fileNames.begin();
         i != fileNames.end(); ++i)
    {
        for (std::vector<IDataNodeReader*>::const_iterator readerIt = services.begin();
             readerIt != services.end(); ++readerIt)
        {
            try
            {
                int n = (*readerIt)->Read(*i, ds);
                nodesRead += n;
                if (n > 0) break;
            }
            catch (const std::exception& e)
            {
                MITK_WARN << e.what();
            }
        }
        mitk::ProgressBar::GetInstance()->Progress(2);
    }

    for (std::vector<us::ServiceReference<IDataNodeReader> >::const_iterator i = refs.begin();
         i != refs.end(); ++i)
    {
        context->UngetService(*i);
    }

    return nodesRead;
}

DataStorage::Pointer IOUtil::LoadFiles(const std::vector<std::string>& fileNames)
{
    mitk::StandaloneDataStorage::Pointer ds = mitk::StandaloneDataStorage::New();
    LoadFiles(fileNames, *ds);
    return ds.GetPointer();
}

DataNode::Pointer IOUtil::LoadDataNode(const std::string path)
{
  try
  {
    mitk::FileReaderRegistry readerRegistry;
    std::vector<BaseData::Pointer> baseDataList = readerRegistry.Read(path);

    BaseData::Pointer baseData;
    if (!baseDataList.empty()) baseData = baseDataList.front();

    if(baseData.IsNull())
    {
      MITK_ERROR << "Could not find data '" << path << "'";
      mitkThrow() << "An exception occured during loading the file " << path << ". Exception says could not find data.";
    }

    mitk::DataNode::Pointer node = mitk::DataNode::New();
    node->SetData(baseData);
    SetDefaultDataNodeProperties(node, path);

    return node;
  }
  catch (const std::exception& e)
  {
    MITK_ERROR << "Exception occured during load data of '" << path << "': Exception: " << e.what();
    mitkThrow() << "An exception occured during loading the file " << path << ". Exception says: " << e.what();
  }
}

Image::Pointer IOUtil::LoadImage(const std::string path)
{
    mitk::DataNode::Pointer node = LoadDataNode(path);
    mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(node->GetData());
    if(image.IsNull())
    {
        MITK_ERROR << "Image is NULL '" << path << "'";
        mitkThrow() << "An exception occured during loading the image " << path << ". Exception says: Image is NULL.";
    }
    return image;
}

Surface::Pointer IOUtil::LoadSurface(const std::string path)
{
    mitk::DataNode::Pointer node = LoadDataNode(path);
    mitk::Surface::Pointer surface = dynamic_cast<mitk::Surface*>(node->GetData());
    if(surface.IsNull())
    {
        MITK_ERROR << "Surface is NULL '" << path << "'";
        mitkThrow() << "An exception occured during loading the file " << path << ". Exception says: Surface is NULL.";
    }
    return surface;
}

PointSet::Pointer IOUtil::LoadPointSet(const std::string path)
{
    mitk::DataNode::Pointer node = LoadDataNode(path);
    mitk::PointSet::Pointer pointset = dynamic_cast<mitk::PointSet*>(node->GetData());
    if(pointset.IsNull())
    {
        MITK_ERROR << "PointSet is NULL '" << path << "'";
        mitkThrow() << "An exception occured during loading the file " << path << ". Exception says: Pointset is NULL.";
    }
    return pointset;
}

bool IOUtil::SaveImage(mitk::Image::Pointer image, const std::string path)
{
  return SaveBaseData(image, path);
  /*
    std::string dir = itksys::SystemTools::GetFilenamePath( path );
    std::string baseFilename = itksys::SystemTools::GetFilenameWithoutExtension( path );
    std::string extension = itksys::SystemTools::GetFilenameExtension( path );
    if (dir == "")
      dir = ".";
    std::string finalFileName = dir + "/" + baseFilename;

    mitk::ImageWriter::Pointer imageWriter = mitk::ImageWriter::New();
    //check if an extension is given, else use the defaul extension
    if( extension == "" )
    {
        MITK_WARN << extension << " extension is not set. Extension set to default: " << finalFileName
                  << DEFAULTIMAGEEXTENSION;
        extension = DEFAULTIMAGEEXTENSION;
    }

    // check if extension is suitable for writing image data
    if (!imageWriter->IsExtensionValid(extension))
    {
        MITK_WARN << extension << " extension is unknown. Extension set to default: " << finalFileName
                  << DEFAULTIMAGEEXTENSION;
        extension = DEFAULTIMAGEEXTENSION;
    }

    try
    {
        //write the data
        imageWriter->SetInput(image);
        imageWriter->SetFileName(finalFileName.c_str());
        imageWriter->SetExtension(extension.c_str());
        imageWriter->Write();
    }
    catch ( std::exception& e )
    {
        MITK_ERROR << " during attempt to write '" << finalFileName + extension << "' Exception says:";
        MITK_ERROR << e.what();
        mitkThrow() << "An exception occured during writing the file " << finalFileName << ". Exception says " << e.what();
    }
    return true;
    */
}

bool IOUtil::SaveSurface(Surface::Pointer surface, const std::string path)
{
  return SaveBaseData(surface, path);
  /*
    std::string dir = itksys::SystemTools::GetFilenamePath( path );
    std::string baseFilename = itksys::SystemTools::GetFilenameWithoutLastExtension( path );
    std::string extension = itksys::SystemTools::GetFilenameLastExtension( path );
    if (dir == "")
      dir = ".";
    std::string finalFileName = dir + "/" + baseFilename;

    if (extension == "") // if no extension has been set we use the default extension
    {
        MITK_WARN << extension << " extension is not set. Extension set to default: " << finalFileName
                  << DEFAULTSURFACEEXTENSION;
        extension = DEFAULTSURFACEEXTENSION;
    }
    try
    {
        finalFileName += extension;
        if(extension == ".stl" )
        {
            mitk::SurfaceVtkWriter<vtkSTLWriter>::Pointer surfaceWriter = mitk::SurfaceVtkWriter<vtkSTLWriter>::New();

            // check if surface actually consists of triangles; if not, the writer will not do anything; so, convert to triangles...
            vtkPolyData* polys = surface->GetVtkPolyData();
            if( polys->GetNumberOfStrips() > 0 )
            {
                vtkSmartPointer<vtkTriangleFilter> triangleFilter = vtkSmartPointer<vtkTriangleFilter>::New();
                triangleFilter->SetInput(polys);
                triangleFilter->Update();
                polys = triangleFilter->GetOutput();
                polys->Register(NULL);
                surface->SetVtkPolyData(polys);
            }
            surfaceWriter->SetInput( surface );
            surfaceWriter->SetFileName( finalFileName.c_str() );
            surfaceWriter->GetVtkWriter()->SetFileTypeToBinary();
            surfaceWriter->Write();
        }
        else if(extension == ".vtp")
        {
            mitk::SurfaceVtkWriter<vtkXMLPolyDataWriter>::Pointer surfaceWriter = mitk::SurfaceVtkWriter<vtkXMLPolyDataWriter>::New();
            surfaceWriter->SetInput( surface );
            surfaceWriter->SetFileName( finalFileName.c_str() );
            surfaceWriter->GetVtkWriter()->SetDataModeToBinary();
            surfaceWriter->Write();
        }
        else if(extension == ".vtk")
        {
            mitk::SurfaceVtkWriter<vtkPolyDataWriter>::Pointer surfaceWriter = mitk::SurfaceVtkWriter<vtkPolyDataWriter>::New();
            surfaceWriter->SetInput( surface );
            surfaceWriter->SetFileName( finalFileName.c_str() );
            surfaceWriter->Write();
        }
        else
        {
            // file extension not suitable for writing specified data type
            MITK_ERROR << "File extension is not suitable for writing'" << finalFileName;
            mitkThrow() << "An exception occured during writing the file " << finalFileName <<
                           ". File extension " << extension << " is not suitable for writing.";
        }
    }
    catch(std::exception& e)
    {
        MITK_ERROR << " during attempt to write '" << finalFileName << "' Exception says:";
        MITK_ERROR << e.what();
        mitkThrow() << "An exception occured during writing the file " << finalFileName << ". Exception says " << e.what();
    }
    return true;
    */
}

bool IOUtil::SavePointSet(PointSet::Pointer pointset, const std::string path)
{
  return SaveBaseData(pointset, path);
  /*
    mitk::PointSetWriter::Pointer pointSetWriter = mitk::PointSetWriter::New();

    std::string dir = itksys::SystemTools::GetFilenamePath( path );
    std::string baseFilename = itksys::SystemTools::GetFilenameWithoutLastExtension( path );
    std::string extension = itksys::SystemTools::GetFilenameLastExtension( path );
    if (dir == "")
      dir = ".";
    std::string finalFileName = dir + "/" + baseFilename;

    if (extension == "") // if no extension has been entered manually into the filename
    {
        MITK_WARN << extension << " extension is not set. Extension set to default: " << finalFileName
                  << DEFAULTPOINTSETEXTENSION;
        extension = DEFAULTPOINTSETEXTENSION;
    }

    // check if extension is valid
    if (!pointSetWriter->IsExtensionValid(extension))
    {
        MITK_WARN << extension << " extension is unknown. Extension set to default: " << finalFileName
                  << DEFAULTPOINTSETEXTENSION;
        extension = DEFAULTPOINTSETEXTENSION;
    }
    try
    {
        pointSetWriter->SetInput( pointset );
        finalFileName += extension;
        pointSetWriter->SetFileName( finalFileName.c_str() );
        pointSetWriter->Update();
    }
    catch( std::exception& e )
    {
        MITK_ERROR << " during attempt to write '" << finalFileName << "' Exception says:";
        MITK_ERROR << e.what();
        mitkThrow() << "An exception occured during writing the file " << finalFileName << ". Exception says " << e.what();
    }
    return true;
    */
}

bool IOUtil::SaveBaseData( mitk::BaseData* data, const std::string& path )
{
  if (data == NULL || path.empty()) return false;

  try
  {
    mitk::FileWriterRegistry::Write(data, path);
  }
  catch(const std::exception& e)
  {
    MITK_ERROR << " Writing a " << data->GetNameOfClass() << " to " << path << " failed: "
               << e.what();
    throw e;
  }

  return true;
}

void IOUtil::SetDefaultDataNodeProperties(DataNode* node, const std::string& filePath)
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
