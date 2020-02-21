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

// uncomment for learning more about the internal sorting mechanisms
//#define MBILOG_ENABLE_DEBUG

#ifndef _WIN32
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <signal.h>
#endif

#include <mutex>
#include <condition_variable>
#include <chrono>

#include <dcmtk/dcmdata/dctk.h>
#include <dcmtk/dcmdata/dcrledrg.h>
#include <dcmtk/dcmdata/dcistrmb.h>
#ifdef ssize_t //for compatibility with hdf5
#undef ssize_t
#endif

#include <mitkDicomSeriesReader.h>
#include <mitkImage.h>
#include <mitkImageCast.h>
#include <mitkLocaleSwitch.h>

#include <condition_variable>

#include <boost/tokenizer.hpp>
#include <boost/format.hpp>
#include <boost/locale.hpp>
#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/filesystem.hpp>

#include <itkGDCMSeriesFileNames.h>

#include <gdcmSorter.h>
#include <gdcmRAWCodec.h>
#include <gdcmAttribute.h>
#include <gdcmPixmapReader.h>
#include <gdcmStringFilter.h>
#include <gdcmDirectory.h>
#include <gdcmScanner.h>
#include <gdcmUIDs.h>

#include <ThreadPoolUtilities.h>
#include "mitkProperties.h"
#include "mitkDicomTagsList.h"

namespace mitk
{

std::string DicomSeriesReader::ReaderImplementationLevelToString( const ReaderImplementationLevel& enumValue )
{
  switch (enumValue)
  {
    case ReaderImplementationLevel_Supported: return "Supported";
    case ReaderImplementationLevel_PartlySupported: return "PartlySupported";
    case ReaderImplementationLevel_Implemented: return "Implemented";
    case ReaderImplementationLevel_Unsupported: return "Unsupported";
    default: return "<unknown value of enum ReaderImplementationLevel>";
  };
}

std::string DicomSeriesReader::PixelSpacingInterpretationToString( const PixelSpacingInterpretation& enumValue )
{
  switch (enumValue)
  {
    case PixelSpacingInterpretation_SpacingInPatient: return "In Patient";
    case PixelSpacingInterpretation_SpacingAtDetector: return "At Detector";
    case PixelSpacingInterpretation_SpacingUnknown: return "Unknown spacing";
    default: return "<unknown value of enum PixelSpacingInterpretation>";
  };
}

DataNode::Pointer
DicomSeriesReader::LoadDicomSeries(const StringContainer &filenames, bool sort, bool check_4d, bool correctTilt, UpdateCallBackMethod callback, void *source, Image::Pointer preLoadedImageBlock)
{
  DataNode::Pointer node = DataNode::New();

  if (DicomSeriesReader::LoadDicomSeries(filenames, *node, sort, check_4d, correctTilt, callback, source, preLoadedImageBlock))
  {
    if( filenames.empty() )
    {
      return nullptr;
    }

    return node;
  }
  else
  {
    return nullptr;
  }
}

bool
DicomSeriesReader::LoadDicomSeries(const StringContainer &filenames,
    DataNode &node,
    bool sort,
    bool check_4d,
    bool correctTilt,
    UpdateCallBackMethod callback,
    void *source,
    itk::SmartPointer<Image> preLoadedImageBlock)
{
  if( filenames.empty() )
  {
    MITK_DEBUG << "Calling LoadDicomSeries with empty filename string container. Probably invalid application logic.";
    node.SetData(nullptr);
    return true; // this is not actually an error but the result is very simple
  }

  DcmIoType::Pointer io = DcmIoType::New();

  try
  {
    FileNamesGrouping descriptors;
    GetSeries(descriptors, filenames);
    if (!descriptors.empty() && io->CanReadFile(filenames.front().c_str()))
    {
      io->SetFileName(filenames.front().c_str());
      io->ReadImageInformation();

      if (io->GetPixelType() == itk::ImageIOBase::SCALAR ||
          io->GetPixelType() == itk::ImageIOBase::RGB)
      {
        LoadDicom(node, check_4d, correctTilt, callback, source, preLoadedImageBlock, *descriptors.begin()->second);
      }

      if (node.GetData())
      {
        return true;
      }
    }
  }
  catch(itk::MemoryAllocationError& e)
  {
    MITK_ERROR << "Out of memory. Cannot load DICOM series: " << e.what();
  }
  catch(std::exception& e)
  {
    MITK_ERROR << "Error encountered when loading DICOM series: " << e.what();
  }
  catch(...)
  {
    MITK_ERROR << "Unspecified error encountered when loading DICOM series.";
  }

  return false;
}

// TODO: REMOVE
bool
DicomSeriesReader::IsDicom(const std::string &filename)
{
  DcmIoType::Pointer io = DcmIoType::New();

  return io->CanReadFile(filename.c_str());
}

bool
DicomSeriesReader::ReadPhilips3DDicom(const std::string &filename, itk::SmartPointer<Image> output_image)
{
  // Now get PhilipsSpecific Tags

  gdcm::PixmapReader reader;
  reader.SetFileName(filename.c_str());
  reader.Read();
  gdcm::DataSet &data_set = reader.GetFile().GetDataSet();
  gdcm::StringFilter sf;
  sf.SetFile(reader.GetFile());

  gdcm::Attribute<0x0028,0x0011> dimTagX; // coloumns || sagittal
  gdcm::Attribute<0x3001,0x1001, gdcm::VR::UL, gdcm::VM::VM1> dimTagZ; //I have no idea what is VM1. // (Philips specific) // axial
  gdcm::Attribute<0x0028,0x0010> dimTagY; // rows || coronal
  gdcm::Attribute<0x0028,0x0008> dimTagT; // how many frames
  gdcm::Attribute<0x0018,0x602c> spaceTagX; // Spacing in X , unit is "physicalTagx" (usually centimeter)
  gdcm::Attribute<0x0018,0x602e> spaceTagY;
  gdcm::Attribute<0x3001,0x1003, gdcm::VR::FD, gdcm::VM::VM1> spaceTagZ; // (Philips specific)
  gdcm::Attribute<0x0018,0x6024> physicalTagX; // if 3, then spacing params are centimeter
  gdcm::Attribute<0x0018,0x6026> physicalTagY;
  gdcm::Attribute<0x3001,0x1002, gdcm::VR::US, gdcm::VM::VM1> physicalTagZ; // (Philips specific)

  dimTagX.Set(data_set);
  dimTagY.Set(data_set);
  dimTagZ.Set(data_set);
  dimTagT.Set(data_set);
  spaceTagX.Set(data_set);
  spaceTagY.Set(data_set);
  spaceTagZ.Set(data_set);
  physicalTagX.Set(data_set);
  physicalTagY.Set(data_set);
  physicalTagZ.Set(data_set);

  unsigned int
    dimX = dimTagX.GetValue(),
    dimY = dimTagY.GetValue(),
    dimZ = dimTagZ.GetValue(),
    dimT = dimTagT.GetValue(),
    physicalX = physicalTagX.GetValue(),
    physicalY = physicalTagY.GetValue(),
    physicalZ = physicalTagZ.GetValue();

  float
    spaceX = spaceTagX.GetValue(),
    spaceY = spaceTagY.GetValue(),
    spaceZ = spaceTagZ.GetValue();

  if (physicalX == 3) // spacing parameter in cm, have to convert it to mm.
    spaceX = spaceX * 10;

  if (physicalY == 3) // spacing parameter in cm, have to convert it to mm.
    spaceY = spaceY * 10;

  if (physicalZ == 3) // spacing parameter in cm, have to convert it to mm.
    spaceZ = spaceZ * 10;

  // Ok, got all necessary Tags!
  // Now read Pixeldata (7fe0,0010) X x Y x Z x T Elements

  const gdcm::Pixmap &pixels = reader.GetPixmap();
  gdcm::RAWCodec codec;

  codec.SetPhotometricInterpretation(gdcm::PhotometricInterpretation::MONOCHROME2);
  codec.SetPixelFormat(pixels.GetPixelFormat());
  codec.SetPlanarConfiguration(0);

  gdcm::DataElement out;
  codec.Decode(data_set.GetDataElement(gdcm::Tag(0x7fe0, 0x0010)), out);

  const gdcm::ByteValue *bv = out.GetByteValue();
  const char *new_pixels = bv->GetPointer();

  // Create MITK Image + Geometry
  typedef itk::Image<unsigned char, 4> ImageType;   //Pixeltype might be different sometimes? Maybe read it out from header
  ImageType::RegionType myRegion;
  ImageType::SizeType mySize;
  ImageType::IndexType myIndex;
  ImageType::SpacingType mySpacing;
  ImageType::Pointer imageItk = ImageType::New();

  mySpacing[0] = spaceX;
  mySpacing[1] = spaceY;
  mySpacing[2] = spaceZ;
  mySpacing[3] = 1;
  myIndex[0] = 0;
  myIndex[1] = 0;
  myIndex[2] = 0;
  myIndex[3] = 0;
  mySize[0] = dimX;
  mySize[1] = dimY;
  mySize[2] = dimZ;
  mySize[3] = dimT;
  myRegion.SetSize( mySize);
  myRegion.SetIndex( myIndex );
  imageItk->SetSpacing(mySpacing);
  imageItk->SetRegions( myRegion);
  imageItk->Allocate();
  imageItk->FillBuffer(0);

  itk::ImageRegionIterator<ImageType>  iterator(imageItk, imageItk->GetLargestPossibleRegion());
  iterator.GoToBegin();
  unsigned long pixCount = 0;
  unsigned long planeSize = dimX*dimY;
  unsigned long planeCount = 0;
  unsigned long timeCount = 0;
  unsigned long numberOfSlices = dimZ;

  while (!iterator.IsAtEnd())
  {
    unsigned long adressedPixel =
      pixCount
      + (numberOfSlices-1-planeCount)*planeSize // add offset to adress the first pixel of current plane
      + timeCount*numberOfSlices*planeSize; // add time offset

    iterator.Set( new_pixels[ adressedPixel ] );
    pixCount++;
    ++iterator;

    if (pixCount == planeSize)
    {
      pixCount = 0;
      planeCount++;
    }
    if (planeCount == numberOfSlices)
    {
      planeCount = 0;
      timeCount++;
    }
    if (timeCount == dimT)
    {
      break;
    }
  }
  mitk::CastToMitkImage(imageItk, output_image);
  return true; // actually never returns false yet.. but exception possible
}

std::string
DicomSeriesReader::ConstCharStarToString(const char* s)
{
  return s ?  std::string(s) : std::string();
}

bool
DicomSeriesReader::DICOMStringToSpacing(const std::string& s, ScalarType& spacingX, ScalarType& spacingY)
{
  bool successful = false;

  std::istringstream spacingReader(s);
  std::string spacing;
  if ( std::getline( spacingReader, spacing, '\\' ) )
  {
    spacingY = OFStandard::atof( spacing.c_str(), &successful );

    if ( successful && std::getline( spacingReader, spacing, '\\' ) )
    {
      spacingX = OFStandard::atof( spacing.c_str(), &successful );

    } else {
      successful = false;
    }
  }

  return successful;
}

Point3D
DicomSeriesReader::DICOMStringToPoint3D(const std::string& s, bool* successful)
{
  Point3D p;

  std::istringstream originReader(s);
  std::string coordinate;
  unsigned int dim(0);
  if(successful) {
    *successful = true;
  }
  while( std::getline( originReader, coordinate, '\\' ) && dim < 3)
  {
    p[dim++]= OFStandard::atof(coordinate.c_str(), successful);
    if (successful && !*successful) {
      break;
    }
  }

  if (dim != 3)
  {
    if (successful) {
      *successful = false;
    }
    if (dim) {
      MITK_ERROR << "Reader implementation made wrong assumption on tag (0020,0032). Found " << dim << " instead of 3 values.";
    }
    while (dim < 3) {
      p[dim++] = 0.0;
    }
  }

  return p;
}

bool
DicomSeriesReader::DICOMStringToOrientationVectors(const std::string& s, vnl_vector_fixed<double,3>& right, vnl_vector_fixed<double,3>& up)
{
  bool successful = true;

  std::istringstream orientationReader(s);
  std::string coordinate;
  unsigned int dim(0);
  while( std::getline( orientationReader, coordinate, '\\' ) && dim < 6 )
  {
    if (dim<3)
    {
      right[dim++] = OFStandard::atof(coordinate.c_str(), &successful);
    }
    else
    {
      up[dim++ - 3] = OFStandard::atof(coordinate.c_str(), &successful);
    }
    if (!successful)
    {
      break;
    }
  }

  if (!successful) {
    MITK_ERROR << "Tag ImageOrientationPatient(0020,0037) has wrong format! Default orientation will be used.";
  } else if (dim == 6) {
    // check for noncollinearity
    if (std::abs(up[0]*right[1]-up[1]*right[0]) >= mitk::eps || std::abs(up[1]*right[2]-up[2]*right[1]) >= mitk::eps || std::abs(up[0]*right[2]-up[2]*right[0]) >= mitk::eps) {
      return true;
    }
    MITK_ERROR << "Tag ImageOrientationPatient(0020,0037) contains collinear vectors, so it is incorrect! Default orientation will be used.";
  } else {
    if (dim) {
      MITK_ERROR << "Tag ImageOrientationPatient(0020,0037) contains only " << dim << " instead of 6 values. Default orientation will be used.";
    }
  }

  // Assign default to vectors for case when the return value will be ignored
  right.fill(0);
  right[0] = 1.0;
  up.fill(0);
  up[1] = 1.0;

  return false;
}
///
/// Postprocessing after all files was analyzed
///
/// The pre-sorted slices of each image be separated to blocks by different z-spacing and tilt.
/// Here also be setted flags for each group.
///
void DicomSeriesReader::FileNamesGrouping::postProcess()
{
  for (auto groupIter = begin(); groupIter != end(); ) {
    MITK_DEBUG << "Analyze group of " << groupIter->second->size() << " files";
    if (groupIter->second->size() == 0) {
      groupIter = erase(groupIter);
      continue;
    }
    if (groupIter->second->mayBeMultiOriented()) {
      ++groupIter;
      continue;
    }
    auto secondBlock = groupIter++->second->checkSliceDistance(); // here the image may be splitted onto first uniform part modified in place and the remaining part returned as the function value
    if (secondBlock) {
      groupIter = emplace(secondBlock->sortingKey(), secondBlock).first; // next we will try to split the new object again
    }
  }
}

std::shared_ptr<mitk::DicomSeriesReader::ImageBlockDescriptor> DicomSeriesReader::FileNamesGrouping::makeBlockDescriptorPtr(std::string file, DcmFileFormat& ff, unsigned long fileSize)
{
  return std::make_shared<ImageBlockDescriptor>(file, ff, fileSize);
}

DicomSeriesReader::FileNamesGrouping::iterator DicomSeriesReader::FileNamesGrouping::addFile(std::shared_ptr<ImageBlockDescriptor> descriptor, DcmFileFormat* ff)
{
  if (!descriptor->good()) {
    return end();
  }
  auto key = descriptor->sortingKey();

  std::unique_lock<std::mutex> lock(m_Mutex);
  auto it = find(key);
  while (it != end() && !descriptor->groupable()) {
    descriptor->m_BlockID--;
    key = descriptor->sortingKey();
    it = find(key);
  }
  if (it == end()) {
    it = emplace_hint(it, key, descriptor);
  } else {
    it->second->AddFile(*descriptor);
  }
  return it;
}

/// Input "stream" for reading DICOM file using file to memory mapping or simple system reading
///
class Stream {
public:
  /// Group of threaded tasks reading DICOM file using file to memory mapping
  /// Also guards them from memory access errors and optimizes file access order.
  ///
  class TaskGroup: public Utilities::TaskGroup {
  public:

    template <typename Path>
    void Enqueue(unsigned i, Path path, const std::function<void(Stream&)>& task, Utilities::TaskPriority priority = Utilities::TaskPriority::LOW);
    TaskGroup(volatile bool* interrupt = nullptr);
    static std::atomic<int> s_InCore;

#ifndef _WIN32
    ~TaskGroup();
  protected:
    static int s_ActiveCnt;
    static std::mutex s_Mutex;
    static void sigBusHandler(int);
    static struct sigaction m_OldSigBus;
#endif

  protected:

    std::mutex m_FileReadMutex;
    std::condition_variable m_FileReadCondition;
    std::set<unsigned> m_Running;
    volatile bool* m_Interrupt;
  };

  // Interface to read file data
  size_t size() const
  {
    return m_Size;
  }
  const char* data() const
  {
    return m_Data;
  }
  static bool incore() ///< true if the file is most likely in system cache (RAM)
  {// Under Linux monitoring the file fetching time proved more effective than the use of a system call mincore()
    return  TaskGroup::s_InCore;
  }

  Stream(): m_Data(nullptr), m_MMaped(false) {}

  /// Open and load the entire file into memory
  template<typename Path> void open(const Path& path)
  {
    close();
    boost::iostreams::file_descriptor_source fd;
    fd.open(path, std::ios_base::in|std::ios_base::binary);
    m_Size = boost::filesystem::file_size(path);
    // Prefetch the file and measure the reading time
    auto startTime = std::chrono::steady_clock::now();

#ifndef _WIN32
    if (m_Size >= (1<<16)) {
      m_Data = (char*)mmap(NULL, m_Size, PROT_READ, MAP_PRIVATE|MAP_POPULATE|MAP_LOCKED, fd.handle(), 0);
      if (m_Data == MAP_FAILED && errno == EAGAIN) {
        m_Data = (char*)mmap(NULL, m_Size, PROT_READ, MAP_PRIVATE|MAP_POPULATE, fd.handle(), 0);
      }
      if (m_Data != MAP_FAILED) {
        m_MMaped = true;
      } else {
        m_Data = nullptr;
        throw 1;
      }
      // prefetch is not need since MAP_LOCKED or MAP_POPULATE is specified
      // othervise the next (commented) code
      /*
      for (size_t i = 0, pgSize = getpagesize(); i < m_Size; i += pgSize) {
         volatile char k = m_Data[i];
      }
      */
    } else // for small files or under Windows read() is quicker
#endif
    {
      m_Data = new char[m_Size];
      size_t s = 0;
      do {
        auto r = fd.read(m_Data + s, m_Size);
        if (r<0) {
          m_Size = s;
          if (s == 0) {
            throw 1;
          }
          break;
        }
        s += r;
      } while (s < m_Size);
    }
    if (m_Size >= 4096) {
      auto byteTime = (std::chrono::duration<double>(std::chrono::steady_clock::now() - startTime).count())/((m_Size+4095)&~4095);
      TaskGroup::s_InCore = byteTime < 5e-9; //< experimentally selected criterion
    }

  }

  /// free memory
  void close()
  {
#ifndef _WIN32
    if (m_Data && m_MMaped) {
      munmap(m_Data, m_Size);
      m_MMaped = false;
    } else
#endif
    {
      delete[] m_Data;
    }
    m_Data = nullptr;
  }
  ~Stream() {
    close();
  }

protected:
  size_t m_Size;
  char* m_Data;
  bool m_MMaped;
};

#if 0
#ifdef _WIN32
/// This class is to transform Windows system exceptions caused by memory page access when the media is absent
/// But file to memory mapping under Windows turned out slower than usual reding
//
static inline size_t getpagesize(void)
{
  SYSTEM_INFO system_info;
  GetSystemInfo (&system_info);
  return system_info.dwPageSize;
}
static void ThrowPageException(const std::function<void()>& protectedAction) {
  __try {
    protectedAction();
  }
  __except (GetExceptionCode() == EXCEPTION_IN_PAGE_ERROR? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
    throw 1;
  }
}
#endif
#endif

template <typename Path>
void Stream::TaskGroup::Enqueue(unsigned i, Path path, const std::function<void(Stream&)>& task, Utilities::TaskPriority priority) {
  std::unique_lock<std::mutex> lock(m_FileReadMutex);
  if (i==0) {
    TaskGroup::s_InCore = 1;
  }
  m_Running.insert(i);
  Utilities::TaskGroup::Enqueue(
    [this, i, path, task]
    {
      try {
#ifdef _WIN32
//      ThrowPageException([&] {
#endif
          if (m_Interrupt && *m_Interrupt) {
            throw 0;
          }

          Stream fd;

          std::unique_lock<std::mutex> lock(m_FileReadMutex);
          bool cached = false;
          while (!(cached = incore()) && i > *m_Running.begin()) {
            if (m_Interrupt && *m_Interrupt) {
              throw 0;
            }
            m_FileReadCondition.wait(lock);
          }
          if (cached) {
            auto i0 = *m_Running.begin();
            m_Running.erase(i);
            if (i == i0) {
              m_FileReadCondition.notify_all();
            }
          }

          lock.unlock();

          fd.open(path); //< throws std::exception on error

          lock.lock();

          if (!m_Running.empty()) {
            auto i0 = *m_Running.begin();
            m_Running.erase(i);
            if (i == i0) {
              m_FileReadCondition.notify_all();
            }
          }

          lock.unlock();

          task(fd);

#ifdef _WIN32
//      });
#endif
      } catch (const std::exception& e) {
        MITK_ERROR << "Readig DICOM file \"" << path << "\" failed: "<< e.what();
      } catch (int e) {
        if (e == 1) {
          // Mapped file memory area becomed unaccesable
          MITK_ERROR << "File \"" << path << "\" is unaccessible (may be truncated or media ejected)";
        } else if (e) {
          // Unattended int exception
          MITK_ERROR << "File \"" << path << "\": exception " << e << " during parse";
        }
        // Else interrupted by user
      }

      std::unique_lock<std::mutex> lock(m_FileReadMutex);
      if (!m_Running.empty()) {
        auto i0 = *m_Running.begin();
        m_Running.erase(i);
        if (i == i0) {
          m_FileReadCondition.notify_all();
        }
      }
    },
  priority);
}

/// Save interruption flag in the object
/// On Linux install handler for SIGBUS can be caused due to disk ejecting or file truncating
///
Stream::TaskGroup::TaskGroup(volatile bool* interrupt): m_Interrupt(interrupt) {
#ifndef _WIN32
  std::unique_lock<std::mutex> lock(s_Mutex);
  if (!s_ActiveCnt++) {
    struct sigaction sigBus;
    if (sigaction(SIGBUS, 0, &sigBus)) {
      MITK_ERROR << "get sigaction error: " << strerror(errno);
    }

    sigBus.sa_handler = &sigBusHandler;
    sigBus.sa_flags = SA_NODEFER;

    if (sigaction(SIGBUS, &sigBus, &m_OldSigBus)) {
      MITK_ERROR << "set sigaction error: " << strerror(errno);
    }
  }
#endif
}

std::atomic<int> Stream::TaskGroup::s_InCore;

#ifndef _WIN32
/// Counter for active reading guard objects
int  Stream::TaskGroup::s_ActiveCnt = 0;
/// Mutex for the counter and initializators
std::mutex Stream::TaskGroup::s_Mutex;
struct sigaction Stream::TaskGroup::m_OldSigBus;

/// Restore previous handler state for SIGBUS
///
Stream::TaskGroup::~TaskGroup() {
  std::unique_lock<std::mutex> lock(s_Mutex);
  if (!--s_ActiveCnt) {
    if (sigaction(SIGBUS, &m_OldSigBus, 0)) {
      MITK_ERROR << "restore sigaction error: " << strerror(errno);
    }
  }
}

/// Linux specific.
/// Generate exception on memory access error
///
void Stream::TaskGroup::sigBusHandler(int)
{
  // Note: may be it is better to use sigsetjmp/siglongjmp just before accessing the mapped area
  // and that code parts must not have dynamic memory operations, aquaring/relesing some resources etc.

  throw 1;
}
#endif

void DicomSeriesReader::GetSeries(DicomSeriesReader::FileNamesGrouping& result, const StringContainer& files, volatile bool* interrupt)
{
  /**
    assumption about this method:
      returns a map of uid-like-key --> list(BlockDescriptor(filenames))
      each entry should contain BlockDescriptor that have images of same
        - series instance uid
        - 0020,0037 image orientation (patient)
        - 0028,0030 pixel spacing (x,y)
        - 0018,0050 slice thickness
  */

  // PART 1: parse and sort images by relevant DICOM tags,
  //         separate images that differ in any of those attributes
  //         assigning each parsed files to one of image blocks sorting slices spatially
  //         (or at least consistently if this is NOT possible, see SliceInfo::operator<)

  // Reading files in the disk order decreases reading time for noncached DVD

  auto taskLimit = boost::thread::hardware_concurrency()*4;
  if (taskLimit > files.size()) {
    taskLimit = files.size();
  }
  Stream::TaskGroup getSeries(interrupt);

  for (unsigned i=0; i < files.size(); i++) {
    if (interrupt && *interrupt) {
      break;
    }
    if (i >= taskLimit) {
      getSeries.WaitFirst();
    }

    auto file = files[i];

    getSeries.Enqueue(i, file, [&result, file] (Stream& fd) {
      // Read file and add data to new or existed image
      DcmInputBufferStream fileStream;
      fileStream.setBuffer(fd.data(), fd.size());
      fileStream.setEos();

      auto r = fileStream.status();
      DcmFileFormat ff;

      if (r.good()) {
        ff.transferInit();
        r = ff.readUntilTag(fileStream,  EXS_Unknown, EGL_noChange, 250);
        ff.transferEnd();
      } else {
        //MITK_ERROR << "DICOM file \"" << file << "\" open failed: " << fileStream.status().text();
      }

      if(r.good()) {
        result.addFile(result.makeBlockDescriptorPtr(file, ff, fd.size()), &ff);
      } else {
        //MITK_ERROR << "DICOM file \"" << file << "\" parse failed: "<< r.text();
      }

    });
  }
  getSeries.WaitAll();

  // PART 2: split blocks with large spatial gaps

  result.postProcess();
}

DicomSeriesReader::FileNamesGrouping
DicomSeriesReader::GetSeries(const std::string &dir)
{
  gdcm::Directory directoryLister;
  directoryLister.Load( dir.c_str(), false ); // non-recursive
  DicomSeriesReader::FileNamesGrouping result;
  GetSeries(result, directoryLister.GetFilenames());
  return result;
}

std::string DicomSeriesReader::GetConfigurationString()
{
  std::stringstream configuration;
  configuration << "MITK_USE_GDCMIO: ";
  configuration << "true";
  configuration << "\n";

  configuration << "GDCM_VERSION: ";
#ifdef GDCM_MAJOR_VERSION
  configuration << GDCM_VERSION;
#endif
  //configuration << "\n";

  return configuration.str();
}

void DicomSeriesReader::copyMetaDataForSerieToImageProperties(Image* image, DcmIoType* io, const ImageBlockDescriptor& blockInfo)
{
  // Copy tags for series, study, patient level (leave interpretation to application).
  // These properties will be copied to the DataNode by DicomSeriesReader.

  // tags for the series (we just use the one that ITK copied to its dictionary (proably that of the last slice)
  const itk::MetaDataDictionary& dict = io->GetMetaDataDictionary();

  auto dictIter = dict.Begin();
  while (dictIter != dict.End()) {
    //MITK_DEBUG << "Key " << dictIter->first;
    std::string value;
    if (itk::ExposeMetaData<std::string>(dict, dictIter->first, value)) {
      //MITK_DEBUG << "Value " << value;

      auto valuePosition = tagToPropertyMap.find(dictIter->first);
      if (valuePosition != tagToPropertyMap.end()) {
        std::string propertyKey = valuePosition->second;
        //MITK_DEBUG << "--> " << propertyKey;

        image->SetProperty(propertyKey.c_str(), StringProperty::New(value));
      }
    } else {
      MITK_WARN << "Tag " << dictIter->first << " not read as string as expected. Ignoring...";
    }
    ++dictIter;
  }

  // copy imageblockdescriptor as properties
  image->SetProperty("dicomseriesreader.SOPClass", StringProperty::New(blockInfo.GetSOPClassUIDAsString()));
  image->SetProperty("dicomseriesreader.ReaderImplementationLevelString", StringProperty::New(ReaderImplementationLevelToString(blockInfo.GetReaderImplementationLevel())));
  image->SetProperty("dicomseriesreader.ReaderImplementationLevel", IntProperty::New(blockInfo.GetReaderImplementationLevel()));
  image->SetProperty("dicomseriesreader.PixelSpacingInterpretationString", StringProperty::New(PixelSpacingInterpretationToString(blockInfo.GetPixelSpacingType())));
  image->SetProperty("dicomseriesreader.PixelSpacingInterpretation", IntProperty::New(blockInfo.GetPixelSpacingType()));
  image->SetProperty("dicomseriesreader.MultiFrameImage", BoolProperty::New(blockInfo.IsMultiFrameImage()));
  image->SetProperty("dicomseriesreader.GantyTiltCorrected", BoolProperty::New(blockInfo.HasGantryTiltCorrected()));
  image->SetProperty("dicomseriesreader.3D+t", BoolProperty::New(blockInfo.HasMultipleTimePoints()));
}

void DicomSeriesReader::CopyMetaDataToImageProperties(DcmIoType *io, const ImageBlockDescriptor& blockInfo, Image *image)
{
  std::list<StringContainer> imageBlock;
  imageBlock.push_back(blockInfo.GetFilenames());
  CopyMetaDataToImageProperties(imageBlock, io, blockInfo, image);
}

void DicomSeriesReader::CopyMetaDataToImageProperties(std::list<StringContainer> imageBlock, DcmIoType* io, const ImageBlockDescriptor& blockInfo, Image* image)
{
  if (!io || !image) {
    return;
  }

  StringLookupTable filesForSlices;
  StringLookupTable sliceLocationForSlices;
  StringLookupTable instanceNumberForSlices;
  StringLookupTable SOPInstanceNumberForSlices;

  unsigned int timeStep(0);

  std::string propertyKeySliceLocation = "dicom.image.0020.1041";
  std::string propertyKeyInstanceNumber = "dicom.image.0020.0013";
  std::string propertyKeySOPInstanceNumber = "dicom.image.0008.0018";

  auto slicesInfo = blockInfo.GetSlicesInfo();
  for (auto i = imageBlock.begin(); i != imageBlock.end(); i++, timeStep++) {
    const StringContainer& files = (*i);
    unsigned int slice(0);
    for (auto fIter = files.begin(); fIter != files.end(); ++fIter, ++slice) {
      filesForSlices.SetTableValue(slice, *fIter);
      if (slicesInfo.find(*fIter) != slicesInfo.end()) {

        if (slicesInfo[*fIter].m_HasImagePositionPatient) {
          sliceLocationForSlices.SetTableValue(slice, std::to_string(slicesInfo[*fIter].m_ImagePositionPatient[2]));
        }

        if (slicesInfo[*fIter].m_InstanceNumber != std::numeric_limits<long>::max()) {
          instanceNumberForSlices.SetTableValue(slice, std::to_string(slicesInfo[*fIter].m_InstanceNumber));
        }

        if (!slicesInfo[*fIter].m_SOPInstanceUID.empty()) {
          SOPInstanceNumberForSlices.SetTableValue(slice, slicesInfo[*fIter].m_SOPInstanceUID);
        }
      }
    }

    image->SetProperty("files", StringLookupTableProperty::New(filesForSlices));

    //If more than one time step add postfix ".t" + timestep
    if (timeStep != 0) {
      std::ostringstream postfix;
      postfix << ".t" << timeStep;

      propertyKeySliceLocation.append(postfix.str());
      propertyKeyInstanceNumber.append(postfix.str());
      propertyKeySOPInstanceNumber.append(postfix.str());
    }
    image->SetProperty(propertyKeySliceLocation.c_str(), StringLookupTableProperty::New(sliceLocationForSlices));
    image->SetProperty(propertyKeyInstanceNumber.c_str(), StringLookupTableProperty::New(instanceNumberForSlices));
    image->SetProperty(propertyKeySOPInstanceNumber.c_str(), StringLookupTableProperty::New(SOPInstanceNumberForSlices));
  }

  copyMetaDataForSerieToImageProperties(image, io, blockInfo);
}
void DicomSeriesReader::FixSpacingInformation( mitk::Image* image, const ImageBlockDescriptor& imageBlockDescriptor )
{
  // spacing provided by ITK/GDCM
  Vector3D imageSpacing = image->GetGeometry()->GetSpacing();
  ScalarType imageSpacingX = imageSpacing[0];
  ScalarType imageSpacingY = imageSpacing[1];

  // spacing as desired by MITK (preference for "in patient", else "on detector", or "1.0/1.0")
  ScalarType desiredSpacingX = imageSpacingX;
  ScalarType desiredSpacingY = imageSpacingY;
  imageBlockDescriptor.GetDesiredMITKImagePixelSpacing( desiredSpacingX, desiredSpacingY );

  MITK_DEBUG << "Loaded spacing: " << imageSpacingX << "/" << imageSpacingY;
  MITK_DEBUG << "Corrected spacing: " << desiredSpacingX << "/" << desiredSpacingY;

  imageSpacing[0] = desiredSpacingX;
  imageSpacing[1] = desiredSpacingY;
  image->GetGeometry()->SetSpacing( imageSpacing );
}

void DicomSeriesReader::FixMetaDataCharset( Image* image )
{
  auto charsetPtr = image->GetProperty("dicom.SpecificCharacterSet");
  if (charsetPtr == nullptr) {
    return;
  }

  std::string charset = charsetPtr->GetValueAsString();
  if (charset == "") {
    return;
  }

  // Trim Charset
  charset.erase(charset.begin(), std::find_if(charset.begin(), charset.end(), [](int ch) {
    return !std::isspace(ch);
  }));
  charset.erase(std::find_if(charset.rbegin(), charset.rend(), [](int ch) {
    return !std::isspace(ch);
  }).base(), charset.end());

  charset = GetStandardCharSet(charset);

#ifdef _WIN32
  std::string locString = boost::locale::util::get_system_locale();
  boost::locale::generator gen;
  std::locale loc = gen(locString);
#endif

  for (const auto& prop : propertiesToLocalize) {
    auto propToLocalizePtr = image->GetProperty(prop.c_str());
    if (propToLocalizePtr == nullptr) {
      continue;
    }
    try {
      std::string utf8String = boost::locale::conv::to_utf<char>(propToLocalizePtr->GetValueAsString(), charset);
#ifdef _WIN32
      std::string locString = boost::locale::conv::from_utf<char>(utf8String, loc);
#else
      std::string locString = boost::locale::conv::from_utf<char>(utf8String, "CP1251");
#endif
      image->SetProperty(prop.c_str(), StringProperty::New(locString));
    }
    catch (boost::locale::conv::invalid_charset_error& e) {
      MITK_WARN << "Error while localizing dicom properties: " << e.what() << '\n';
      return;
    }
  }
}

std::string DicomSeriesReader::GetStandardCharSet(std::string dicomCharset)
{
  if (dicomCharset == "ISO 2022 IR 6")             // ASCII
    return "ASCII";
  else if (dicomCharset == "ISO 2022 IR 100")      // Latin alphabet No. 1
    return "ISO-8859-1";
  else if (dicomCharset == "ISO 2022 IR 101")      // Latin alphabet No. 2
    return "ISO-8859-2";
  else if (dicomCharset == "ISO 2022 IR 109")      // Latin alphabet No. 3
    return "ISO-8859-3";
  else if (dicomCharset == "ISO 2022 IR 110")      // Latin alphabet No. 4
    return "ISO-8859-4";
  else if (dicomCharset == "ISO 2022 IR 144")      // Cyrillic
    return "ISO-8859-5";
  else if (dicomCharset == "ISO 2022 IR 127")      // Arabic
    return "ISO-8859-6";
  else if (dicomCharset == "ISO 2022 IR 126")      // Greek
    return "ISO-8859-7";
  else if (dicomCharset == "ISO 2022 IR 138")      // Hebrew
    return "ISO-8859-8";
  else if (dicomCharset == "ISO 2022 IR 148")      // Latin alphabet No. 5
    return "ISO-8859-9";
  else if (dicomCharset == "ISO 2022 IR 13")       // Japanese
#if DCMTK_ENABLE_CHARSET_CONVERSION == DCMTK_CHARSET_CONVERSION_ICONV
    return "JIS_X0201";                 // - the name "ISO-IR-13" is not supported by libiconv
#else
    return "Shift_JIS";                 // - ICU and stdlibc iconv only know "Shift_JIS" (is this mapping correct?)
#endif
  else if (dicomCharset == "ISO 2022 IR 166")      // Thai
#if DCMTK_ENABLE_CHARSET_CONVERSION == DCMTK_CHARSET_CONVERSION_ICU
    return "TIS-620";                   // - "ISO-IR-166" is not supported by ICU
#else
    return "ISO-IR-166";
#endif
  else if (dicomCharset == "ISO 2022 IR 87")       // Japanese (multi-byte)
    return "ISO-IR-87";                 // - this might generate an error since "ISO-IR-87" is not supported by ICU and stdlibc iconv
  else if (dicomCharset == "ISO 2022 IR 159")      // Japanese (multi-byte)
    return "ISO-IR-159";                // - this might generate an error since "ISO-IR-159" is not supported by ICU and stdlibc iconv
  else if (dicomCharset == "ISO 2022 IR 149")      // Korean (multi-byte)
    return "EUC-KR";                    // - is this mapping really correct?
  else if (dicomCharset == "ISO 2022 IR 58")       // Simplified Chinese (multi-byte)
    return "GB2312";                    // - should work, but not tested yet!
  else if (dicomCharset == "ISO_IR 6")           // ASCII
    return "ASCII";
  else if (dicomCharset == "ISO_IR 100")    // Latin alphabet No. 1
    return "ISO-8859-1";
  else if (dicomCharset == "ISO_IR 101")    // Latin alphabet No. 2
    return "ISO-8859-2";
  else if (dicomCharset == "ISO_IR 109")    // Latin alphabet No. 3
    return "ISO-8859-3";
  else if (dicomCharset == "ISO_IR 110")    // Latin alphabet No. 4
    return "ISO-8859-4";
  else if (dicomCharset == "ISO_IR 144")    // Cyrillic
    return "ISO-8859-5";
  else if (dicomCharset == "ISO_IR 127")    // Arabic
    return "ISO-8859-6";
  else if (dicomCharset == "ISO_IR 126")    // Greek
    return "ISO-8859-7";
  else if (dicomCharset == "ISO_IR 138")    // Hebrew
    return "ISO-8859-8";
  else if (dicomCharset == "ISO_IR 148")    // Latin alphabet No. 5
    return "ISO-8859-9";
  else if (dicomCharset == "ISO_IR 13")     // Japanese
#if DCMTK_ENABLE_CHARSET_CONVERSION == DCMTK_CHARSET_CONVERSION_ICONV
    return "JIS_X0201";                 // - the name "ISO-IR-13" is not supported by libiconv
#else
    return "Shift_JIS";                 // - ICU and stdlibc iconv only know "Shift_JIS" (is this mapping correct?)
#endif
  else if (dicomCharset == "ISO_IR 166")    // Thai
#if DCMTK_ENABLE_CHARSET_CONVERSION == DCMTK_CHARSET_CONVERSION_ICU
    return "TIS-620";                   // - the name "ISO-IR-166" is not supported by ICU
#else
    return "ISO-IR-166";
#endif
  else if (dicomCharset == "ISO_IR 192")    // Unicode in UTF-8 (multi-byte)
    return "UTF-8";
  else if (dicomCharset == "GB18030")       // Chinese (multi-byte)
    return "GB18030";
  else if (dicomCharset == "GBK")           // Chinese (multi-byte, subset of "GB 18030")
    return "GBK";

  return dicomCharset;
}

void DicomSeriesReader::LoadDicom(DataNode &node, bool load4D, bool correctTilt, UpdateCallBackMethod callback, void *source, Image::Pointer preLoadedImageBlock, DicomSeriesReader::ImageBlockDescriptor& descriptor)
{
  mitk::LocaleSwitch localeSwitch("C");
  struct LocaleSwitcher {
    std::locale previousCppLocale;
    LocaleSwitcher() : previousCppLocale(std::cin.getloc())
    {
      std::locale l( "C" );
      std::cin.imbue(l);
    }
    ~LocaleSwitcher()
    {
      std::cin.imbue(previousCppLocale);
    }
  } localeSwitcher;

  try
  {
    Image::Pointer image = preLoadedImageBlock.IsNull() ? Image::New() : preLoadedImageBlock;
    CallbackCommand *command = callback ? new CallbackCommand(callback, source) : nullptr;
    bool initialize_node = false;

    const StringContainer& filenames = descriptor.GetFilenames();
    /* special case for Philips 3D+t ultrasound images */
    if ( descriptor.m_Philips3D )
    {
      // TODO what about imageBlockDescriptor?
      // TODO what about preLoadedImageBlock?
      ReadPhilips3DDicom(filenames.front().c_str(), image);
      initialize_node = true;
    }
    else
    {
      std::list<StringContainer> imageBlocks;
      /* default case: assume "normal" image blocks, possibly 3D+t */
      bool canLoadAs4D(true);

      imageBlocks = descriptor.SortIntoBlocksFor3DplusT(canLoadAs4D);

      unsigned int volume_count = imageBlocks.size();

      GantryTiltInformation tiltInfo;

      // check possibility of a single slice with many timesteps. In this case, don't check for tilt, no second slice possible
      if ( !imageBlocks.empty() && imageBlocks.front().size() > 1 && correctTilt)
      {
        // check tiltedness here, potentially fixup ITK's loading result by shifting slice contents
        // check first and last position slice from tags, make some calculations to detect tilt

        std::string firstFilename(imageBlocks.front().front());
        // calculate from first and last slice to minimize rounding errors
        std::string secondFilename(imageBlocks.front().back());

        auto slicesInfo = descriptor.GetSlicesInfo();
        auto orientation = descriptor.GetOrientation();

        tiltInfo = GantryTiltInformation ( slicesInfo[firstFilename].m_ImagePositionPatient, slicesInfo[secondFilename].m_ImagePositionPatient, orientation[0], orientation[1], filenames.size()-1 );
        correctTilt = tiltInfo.IsSheared() && tiltInfo.IsRegularGantryTilt();
      }
      else
      {
        correctTilt = false; // we CANNOT do that
      }

      descriptor.SetHasGantryTiltCorrected( correctTilt );

      image = descriptor.GetImage();
      descriptor.DropImages();
      if (volume_count == 1 || !canLoadAs4D || !load4D)
      {
        descriptor.SetHasMultipleTimePoints( false );

        if (!image) {
          DcmIoType::Pointer io;
          image = MultiplexLoadDICOMByITK( descriptor, correctTilt, tiltInfo, io, command, preLoadedImageBlock ); // load single 3D block
          CopyMetaDataToImageProperties(io, descriptor, image);
        }

        FixSpacingInformation( image, descriptor );
        FixMetaDataCharset( image );

        initialize_node = true;
      }
      else if (volume_count > 1)
      {
        if (!image) {
          descriptor.SetHasMultipleTimePoints( true );
          DcmIoType::Pointer io;
          image = MultiplexLoadDICOMByITK4D( imageBlocks, descriptor, correctTilt, tiltInfo, io, command, preLoadedImageBlock );
        }

        initialize_node = true;
      }
    }

    if (initialize_node)
    {
      // forward some image properties to node
      node.GetPropertyList()->ConcatenatePropertyList( image->GetPropertyList(), true );

      std::string patientName = "NoName";
      if(node.GetProperty("dicom.patient.PatientsName"))
        patientName = node.GetProperty("dicom.patient.PatientsName")->GetValueAsString();

      node.SetData(image);
      node.SetName(patientName);
    }

    MITK_DEBUG << "--------------------------------------------------------------------------------";
    MITK_DEBUG << "DICOM files loaded (from series UID " << descriptor.GetSeriesInstanceUID() << "):";
    MITK_DEBUG << "  " << descriptor.size() << " '" << descriptor.GetModality() << "' files (" << descriptor.GetSOPClassUIDAsString() << ") loaded into 1 mitk::Image";
    MITK_DEBUG << "  multi-frame: " << (descriptor.IsMultiFrameImage()?"Yes":"No");
    MITK_DEBUG << "  reader support: " << ReaderImplementationLevelToString(descriptor.GetReaderImplementationLevel());
    MITK_DEBUG << "  pixel spacing type: " << PixelSpacingInterpretationToString( descriptor.GetPixelSpacingType() ) << " " << image->GetGeometry()->GetSpacing()[0] << "/" << image->GetGeometry()->GetSpacing()[0];
    MITK_DEBUG << "  gantry tilt corrected: " << (descriptor.HasGantryTiltCorrected()?"Yes":"No");
    MITK_DEBUG << "  3D+t: " << (descriptor.HasMultipleTimePoints()?"Yes":"No");
    MITK_DEBUG << "--------------------------------------------------------------------------------";
  }
  catch (std::exception& e)
  {
    //reset locale then throw up
    //std::cin.imbue(previousCppLocale);

    MITK_ERROR << "Caught exception in DicomSeriesReader::LoadDicom " << e.what();

    //throw e;
  }
}

void
DicomSeriesReader::ScanForSliceInformation(const StringContainer &filenames, gdcm::Scanner& scanner)
{
  const gdcm::Tag tagImagePositionPatient(0x0020,0x0032); //Image position (Patient)
  scanner.AddTag(tagImagePositionPatient);

  const gdcm::Tag tagSeriesInstanceUID(0x0020, 0x000e); // Series Instance UID
  scanner.AddTag(tagSeriesInstanceUID);

  const gdcm::Tag tagImageOrientation(0x0020,0x0037); //Image orientation
  scanner.AddTag(tagImageOrientation);

  const gdcm::Tag tagSliceLocation(0x0020, 0x1041); // slice location -
  scanner.AddTag( tagSliceLocation );

  const gdcm::Tag tagInstanceNumber(0x0020, 0x0013); // (image) instance number
  scanner.AddTag( tagInstanceNumber );

  const gdcm::Tag tagSOPInstanceNumber(0x0008, 0x0018); // SOP instance number
  scanner.AddTag( tagSOPInstanceNumber );

  const gdcm::Tag tagPixelSpacing(0x0028, 0x0030); // Pixel Spacing
  scanner.AddTag( tagPixelSpacing );

  const gdcm::Tag tagImagerPixelSpacing(0x0018, 0x1164); // Imager Pixel Spacing
  scanner.AddTag( tagImagerPixelSpacing );

  const gdcm::Tag tagModality(0x0008, 0x0060); // Modality
  scanner.AddTag( tagModality );

  const gdcm::Tag tagSOPClassUID(0x0008, 0x0016); // SOP Class UID
  scanner.AddTag( tagSOPClassUID );

  const gdcm::Tag tagNumberOfFrames(0x0028, 0x0008); // number of frames
    scanner.AddTag( tagNumberOfFrames );

  scanner.Scan(filenames); // make available image information for each file
}

namespace
{
enum class DICOMType
{
  NONE,
  SCALAR,
  RGB,
  SCALAR_4D,
  RGB_4D
};

class LoadSync
{
public:
  LoadSync();

  void Lock(const DICOMType type);
  void Unlock(const DICOMType type);

private:
  typedef std::unique_lock<std::recursive_mutex> ScopedLock;

  DICOMType m_type;
  size_t m_count;
  std::recursive_mutex m_mutex;
  std::condition_variable_any m_event;
};

LoadSync::LoadSync()
  : m_type(DICOMType::NONE)
  , m_count(0)
{
}

void LoadSync::Lock(const DICOMType type)
{
  ScopedLock lock(m_mutex);
  while (DICOMType::NONE != m_type && type != m_type) {
    m_event.wait(lock);
  }
  m_type = type;
  ++m_count;
}

void LoadSync::Unlock(const DICOMType type)
{
  ScopedLock lock(m_mutex);
  if (!--m_count) {
    m_type = DICOMType::NONE;
  }
  m_event.notify_all();
}

class LoadSyncGuard
{
public:
  explicit LoadSyncGuard(const DICOMType type);
  ~LoadSyncGuard();

private:
  const DICOMType m_type;

  static LoadSync s_guard;
};

LoadSync LoadSyncGuard::s_guard;

LoadSyncGuard::LoadSyncGuard(const DICOMType type)
  : m_type(type)
{
  s_guard.Lock(m_type);
}

LoadSyncGuard::~LoadSyncGuard()
{
  s_guard.Unlock(m_type);
}
}

Image::Pointer
DicomSeriesReader
::MultiplexLoadDICOMByITK(const ImageBlockDescriptor& descriptor, bool correctTilt, const GantryTiltInformation& tiltInfo, DcmIoType::Pointer& io, CallbackCommand* command, Image::Pointer preLoadedImageBlock)
{
  const auto& filenames = descriptor.m_Filenames;
  io = DcmIoType::New();
  io->SetFileName(filenames.front().c_str());
  io->ReadImageInformation();

  if (io->GetPixelType() == itk::ImageIOBase::SCALAR)
  {
    const LoadSyncGuard guard(DICOMType::SCALAR);
    return MultiplexLoadDICOMByITKScalar(filenames, correctTilt, tiltInfo, io, command ,preLoadedImageBlock);
  }
  else if (io->GetPixelType() == itk::ImageIOBase::RGB)
  {
    const LoadSyncGuard guard(DICOMType::RGB);
    return MultiplexLoadDICOMByITKRGBPixel(filenames, correctTilt, tiltInfo, io, command ,preLoadedImageBlock);
  }
  else
  {
    return nullptr;
  }
}

Image::Pointer
DicomSeriesReader
::MultiplexLoadDICOMByITK4D( std::list<StringContainer>& imageBlocks, ImageBlockDescriptor imageBlockDescriptor, bool correctTilt, const GantryTiltInformation& tiltInfo, DcmIoType::Pointer& io, CallbackCommand* command, Image::Pointer preLoadedImageBlock)
{
  io = DcmIoType::New();
  io->SetFileName(imageBlocks.front().front().c_str());
  io->ReadImageInformation();

  if (io->GetPixelType() == itk::ImageIOBase::SCALAR)
  {
    const LoadSyncGuard guard(DICOMType::SCALAR_4D);
    return MultiplexLoadDICOMByITK4DScalar(imageBlocks, imageBlockDescriptor, correctTilt, tiltInfo, io, command ,preLoadedImageBlock);
  }
  else if (io->GetPixelType() == itk::ImageIOBase::RGB)
  {
    const LoadSyncGuard guard(DICOMType::RGB_4D);
    return MultiplexLoadDICOMByITK4DRGBPixel(imageBlocks, imageBlockDescriptor, correctTilt, tiltInfo, io, command ,preLoadedImageBlock);
  }
  else
  {
    return nullptr;
  }
}
} // end namespace mitk
