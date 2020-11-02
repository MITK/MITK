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

#include <dcmtk/dcmdata/dctk.h>
#include <dcmtk/dcmdata/dcrledrg.h>
#include <dcmtk/dcmdata/dcistrmb.h>
#include <dcmtk/dcmimage/diregist.h>     /* to support color images (under WIN)*/
#include <dcmtk/dcmimgle/dcmimage.h>
#include <dcmtk/dcmjpeg/djdecode.h>
#include <dcmtk/dcmjpls/djdecode.h>
#ifdef ssize_t //for compatibility with hdf5
#undef ssize_t
#endif

#include <mitkDicomSeriesReader.h>
#include <mitkImage.h>
#include <mitkStructuredReport.h>
#include <mitkPixelTypeMultiplex.h>
#include <vnl/vnl_cross.h>

#include <gdcmUIDs.h>
#include <gdcmImageReader.h>
#include <gdcmTagKeywords.h>
#include <gdcmGlobal.h>
#include <gdcmMediaStorage.h>
#include <gdcmStringFilter.h>

#include <mitkDicomTagsList.h>

static struct DCMTKCodecInitilizer {
  DCMTKCodecInitilizer()
  {
    DJDecoderRegistration::registerCodecs(EDC_never, EUC_never, EPC_colorByPixel, true, true);
    DJLSDecoderRegistration::registerCodecs(EJLSUC_never, EJLSPC_colorByPixel);
    DcmRLEDecoderRegistration::registerCodecs();
  }
} codecInitilizer;

namespace std {
  static inline string to_string(const string& s)
  {
    if (s.back() == 0 || s.back() == ' ') {
      return s.substr(0, s.size()-1);
    }
    return s;
  }
}

namespace mitk
{

// Helpers using GDCM
template <typename Attr>
static std::string getTag(std::string& var, const gdcm::DataSet& ds)
{
  try {
    Attr attr;
    attr.Set(ds);
    var = std::to_string(attr.GetValue()); //TODO: locale depend!
    for (unsigned int i= 1; i < attr.GetNumberOfValues(); i++) {
      var += '\\';
      var += std::to_string(attr.GetValues()[i]);
    }
  } catch (std::exception&) {
    var.clear();
  }
  return var;
}

template <typename Attr, typename T, T badValue=0>
static inline unsigned getTag(T& var, const gdcm::DataSet& ds)
{
  try {
    Attr attr;
    attr.Set(ds);
    var = attr.GetValue();
  } catch (std::exception&) {
    var = badValue;
  }
  return var;
}

// Helpers using DCMTK

static std::string getTag(DcmTagKey tag, DcmItem* d1, DcmItem* d2 = nullptr)
{
  std::string r;
  d1->findAndGetOFStringArray(tag, r);
  if (r.empty() && d2) {
    d2->findAndGetOFStringArray(tag, r);
  }
  return r;
}

static double getDoubleTag(DcmTagKey tag, DcmItem* d1, DcmItem* d2 = nullptr)
{
  return OFStandard::atof(getTag(tag, d1, d2).c_str());
}

static long getIntTag(DcmTagKey tag, long defaultValue, DcmItem* d1, DcmItem* d2 = nullptr)
{
  try {
    return std::stol(getTag(tag, d1, d2));
  } catch (std::exception&) {
    return defaultValue;
  }
}

void DicomSeriesReader::fillSliceInfo(SliceInfo& info, DcmItem* ds, unsigned long fileSize)
{
  info.m_FileSize = fileSize;
  std::string s;
  ds->findAndGetOFStringArray(DCM_ImagePositionPatient, s);
  info.m_ImagePositionPatient = DICOMStringToPoint3D(s, &info.m_HasImagePositionPatient);
  info.m_InstanceNumber = getIntTag(DCM_InstanceNumber, std::numeric_limits<long>::max(), ds);
  info.m_AcquisitionNumber = getIntTag(DCM_AcquisitionNumber, std::numeric_limits<long>::max(), ds);
  info.m_TemporalPosition = getIntTag(DCM_TemporalPositionIndex, std::numeric_limits<long>::max(), ds);
  ds->findAndGetOFStringArray(DCM_SOPInstanceUID, info.m_SOPInstanceUID);
  if (info.m_SOPInstanceUID.empty()) {
    ds->findAndGetOFStringArray(DCM_ReferencedSOPInstanceUIDInFile, info.m_SOPInstanceUID);
  }
  ds->findAndGetOFStringArray(DCM_ImageOrientationPatient, s);
  info.m_HasOrientation = DICOMStringToOrientationVectors(s, info.m_Orientation[0], info.m_Orientation[1]);
  ds->findAndGetOFStringArray(DCM_GantryDetectorTilt, info.m_GantryTilt);
}

void DicomSeriesReader::ImageBlockDescriptor::fillSeriesInfo(DcmItem* d1, DcmItem* d2, DcmItem* d3)
{
  m_PatientID = getTag(DCM_PatientID, d1, d3);
  m_SeriesInstanceUID = getTag(DCM_SeriesInstanceUID, d1, d2);
  m_Modality = getTag(DCM_Modality, d1, d2);
  m_SOPClassUID = getTag(DCM_SOPClassUID, d1, d2);
  m_PixelSpacing = getTag(DCM_PixelSpacing, d1, d2);
  m_ImagerPixelSpacing = getTag(DCM_ImagerPixelSpacing, d1, d2);
  m_PhotometricInterpretation = getTag(DCM_PhotometricInterpretation, d1, d2);
  m_NumberOfFrames = getIntTag(DCM_NumberOfFrames, 0, d1, d2);
  m_Rows = getTag(DCM_Rows, d1, d2);
  m_Columns = getTag(DCM_Columns, d1, d2);
  std::string options = getTag(DCM_ScanOptions, d1, d2);
  if ( options.find("SCANOSCOPE") != std::string::npos || options.find("SCOUT") != std::string::npos
    || getTag(DCM_ImageType, d1, d2).find("LOCALIZER") != std::string::npos ) {
    m_SliceThickness = -1;
    m_SliceDistance = 1;
  } else {
    m_SliceDistance = m_SliceThickness = getDoubleTag(DCM_SliceThickness, d1, d2);
    if (m_SliceThickness == 0) {
      m_SliceThickness = -1;
      m_SliceDistance = 1;
    }
  }
}

DicomSeriesReader::ImageBlockDescriptor::ImageBlockDescriptor(std::string path, DcmItem* FileRecord, DcmItem* SeriesRecord, DcmItem* PatientRecord):
  m_Mutex(std::make_shared<std::mutex>()),
  m_Filenames({path}),
  m_SliceThickness(0),
  m_SliceDistance(1),
  m_BlockID(0),
  m_MultiOriented(false),
  m_HasGantryTiltCorrected(false), // This flags can be setted in checkSliceDistance() or directly
  m_HasMultipleTimePoints(false), //
  m_BadSlicingDistance(false),   //
  m_Philips3D(false)
{
  auto& info= m_SlicesInfo[path];

  fillSliceInfo(info, FileRecord);
  m_HasOrientation = info.m_HasOrientation;
  std::copy(info.m_Orientation, info.m_Orientation+2, m_Orientation);
  fillSeriesInfo(FileRecord, SeriesRecord, PatientRecord);
}
static std::string getTag(const gdcm::Tag tag, const gdcm::DataSet& ds)
{
  try {
    auto& de = ds.GetDataElement(tag);
    auto vr= de.GetVR();
    if (vr | gdcm::VR::VRASCII) {
      auto r= de.GetByteValue();
      if (!r) {
        throw std::exception();
      }
      return std::string(r->GetPointer(), r->GetLength());
    } else {
      std::ostringstream stream;
      stream << de;
      return stream.str();
    }
  } catch (std::exception&) {
    return std::string();
  }
}


template <class DataSet>
static double getDoubleTag(const gdcm::Tag tag, const DataSet& scanResult)
{
  return OFStandard::atof(getTag(tag, scanResult).c_str());
}

template <class DataSet>
static unsigned long getUIntTag(const gdcm::Tag tag, const DataSet& scanResult)
{
  std::string s;
  try {
    return std::stoul(getTag(tag, scanResult));
  } catch (std::exception&) {
    return 0;
  }
}

template<typename T>
static mitk::PixelType makePixelType(int numOfComponents)
{
  switch (numOfComponents) {
    case 1: return MakePixelType<T,T>(1);
    default: return MakePixelType<T, itk::Vector<T,3> >(numOfComponents);
  }
}

static mitk::PixelType makePixelType(const DicomImage& dcmImage)
{
   const DiPixel* interData = dcmImage.getInterData();
  int numOfComponents = interData->getPlanes();
  if (numOfComponents == 3) {
    switch (dcmImage.getPhotometricInterpretation()) {
    case EPI_Missing:
      if (interData->getRepresentation() != EPR_Uint8) {
        break;
      }
      // fall-through
    case EPI_PaletteColor:
    case EPI_RGB:
    case EPI_YBR_Full:
    case EPI_YBR_Full_422:
    case EPI_YBR_Partial_422:
      return MakePixelType<unsigned char, itk::RGBPixel<unsigned char> >(3);
    default:;
    }
  }
  double minValue, maxValue;
  if (dcmImage.getMinMaxValues(minValue, maxValue, 1)) {
    int bits = DicomImageClass::rangeToBits(minValue, maxValue);
    return minValue < 0 ? bits <= 16? bits <=8? makePixelType<char>(numOfComponents)
                                              : makePixelType<short>(numOfComponents)
                                    : makePixelType<int>(numOfComponents)
                        : bits <= 16? bits <=8? makePixelType<unsigned char>(numOfComponents)
                                              : makePixelType<unsigned short>(numOfComponents)
                                    : makePixelType<unsigned int>(numOfComponents);
  }
  switch (interData->getRepresentation()) {
  case EPR_Uint8:  return makePixelType<unsigned char>(numOfComponents);
  case EPR_Sint8:  return makePixelType<char>(numOfComponents);
  case EPR_Uint16: return makePixelType<unsigned short>(numOfComponents);
  case EPR_Sint16: return makePixelType<short>(numOfComponents);
  case EPR_Uint32: return makePixelType<unsigned int>(numOfComponents);
  case EPR_Sint32: return makePixelType<int>(numOfComponents);
  }
  assert(!"Unknown pixel type in DCMTK Image");
  return makePixelType<unsigned char>(1);
}

template<typename dstT, typename srcT>
struct Copyer
{
  static void copy(void* dst0, const void* src0, int planes, size_t count)
  {
    dstT* d = static_cast<dstT*>(dst0);
    const srcT **s= new const srcT*[planes];
    for (int i=0; i<planes; i++) {
      s[i] = static_cast<const srcT* const*>(src0)[i];
    }
    while (count--) {
      for (int i=0; i<planes; i++) {
        *d++ = *s[i]++;
      }
    }
    delete[] s;
  }
};

template<typename dstT, typename srcT>
struct PlaneCopyer
{
  static void copy(void* dst0, const void* src0, size_t count)
  {
    dstT* d = static_cast<dstT*>(dst0);
    const srcT* s = static_cast<const srcT* const*>(src0)[0];
    while (count--) {
      *d++ = *s++;
    }
  }
};
/*
template<typename T>
struct PlaneCopyer<T,T>
{
  static void copy(void* dst0, const void* src0, size_t count) // optimization for equal types
  {
    T* d = static_cast<T*>(dst0);
    const T* s = static_cast<const T* const*>(src0)[0];
    memcpy(d, s, count * sizeof (T));
  }
};
*/
template<typename dstT>
static void copyData(const mitk::PixelType&, void* dst0, DiPixel* interData)
{
  int planes = interData->getPlanes();
/*  if (planes == 1) {
    switch (interData->getRepresentation()) {
    case EPR_Uint8:  return PlaneCopyer<dstT, unsigned char>::copy(dst0, interData->getDataArrayPtr(), interData->getInputCount());
    case EPR_Sint8:  return PlaneCopyer<dstT, char>::copy(dst0, interData->getDataArrayPtr(), interData->getInputCount());
    case EPR_Uint16: return PlaneCopyer<dstT, unsigned short>::copy(dst0, interData->getDataArrayPtr(), interData->getInputCount());
    case EPR_Sint16: return PlaneCopyer<dstT, short>::copy(dst0, interData->getDataArrayPtr(), interData->getInputCount());
    case EPR_Uint32: return PlaneCopyer<dstT, unsigned int>::copy(dst0, interData->getDataArrayPtr(), interData->getInputCount());
    case EPR_Sint32: return PlaneCopyer<dstT, int>::copy(dst0, interData->getDataArrayPtr(), interData->getInputCount());
    }
  }*/
  switch (interData->getRepresentation()) {
  case EPR_Uint8:  return Copyer<dstT, unsigned char>::copy(dst0, interData->getDataArrayPtr(), planes, interData->getInputCount());
  case EPR_Sint8:  return Copyer<dstT, char>::copy(dst0, interData->getDataArrayPtr(), planes, interData->getInputCount());
  case EPR_Uint16: return Copyer<dstT, unsigned short>::copy(dst0, interData->getDataArrayPtr(), planes, interData->getInputCount());
  case EPR_Sint16: return Copyer<dstT, short>::copy(dst0, interData->getDataArrayPtr(), planes, interData->getInputCount());
  case EPR_Uint32: return Copyer<dstT, unsigned int>::copy(dst0, interData->getDataArrayPtr(), planes, interData->getInputCount());
  case EPR_Sint32: return Copyer<dstT, int>::copy(dst0, interData->getDataArrayPtr(), planes, interData->getInputCount());
  }
}

static void copyInterData(void* frame, const mitk::PixelType& dstPixelType, const DicomImage& dcmImage)
{
  mitkPixelTypeMultiplex2( copyData, dstPixelType, frame, const_cast<DiPixel*>(dcmImage.getInterData()) )
}

mitk::Matrix3D DicomSeriesReader::ImageBlockDescriptor::getMatrix(const Vector3D& spacing) const
{
  mitk::Matrix3D matrix;
  if (m_HasOrientation) {
    auto ey = vnl_vector<double>(m_Orientation[1]).normalize();
    auto ez = vnl_cross_3d(vnl_vector<double>(m_Orientation[0]), ey).normalize();
    auto ex = vnl_cross_3d(ey,ez).normalize();
    for (int j=0; j<3; ++j) {
      matrix[j][0] = ex[j]*spacing[0];
      matrix[j][1] = ey[j]*spacing[1];
      matrix[j][2] = ez[j]*spacing[2];
    }
  } else {
    for (int i=0; i<3; ++i) {
      for (int j=0; j<3; ++j) {
        matrix[i][j] = i==j? spacing[j] : 0;
      }
    }
  }
  return matrix;
}

DicomSeriesReader::ImageBlockDescriptor::ImageBlockDescriptor(std::string path, DcmFileFormat& ff, unsigned long fileSize):
  m_Mutex(std::make_shared<std::mutex>()),
  m_Filenames({path}),
  m_SliceThickness(0),
  m_SliceDistance(1),
  m_BlockID(0),
  m_MultiOriented(false),
  m_HasGantryTiltCorrected(false), // This flags can be setted in checkSliceDistance()
  m_HasMultipleTimePoints(false), //
  m_BadSlicingDistance(false)    //
{
  auto ds = ff.getDataset();
  auto mi = ff.getMetaInfo();
  // Checks if a specific file is a Philips3D ultrasound DICOM file.
  m_Philips3D = std::string(getTag(DcmTagKey(0x3001, 0x0020), ds)) == "Philips3D";

  auto& info= m_SlicesInfo[path];

  fillSliceInfo(info, ds, fileSize);
  if (info.m_SOPInstanceUID.empty()) {
    mi->findAndGetOFStringArray(DCM_MediaStorageSOPInstanceUID, info.m_SOPInstanceUID);
  }

  m_HasOrientation = info.m_HasOrientation;
  std::copy(info.m_Orientation, info.m_Orientation+2, m_Orientation);
  fillSeriesInfo(ds);

  if (m_SOPClassUID.empty()) {
    mi->findAndGetOFStringArray(DCM_MediaStorageSOPClassUID, m_SOPClassUID);
  }
}

bool DicomSeriesReader::ImageBlockDescriptor::loadStructeredReport(DcmFileFormat& ff, ImageBlockDescriptor* src)
{
  if (!src) {
    src = this;
  }

  std::unique_lock<std::mutex> lock(*m_Mutex);
  auto filename = src->m_Filenames[0];
  auto& info = m_SlicesInfo.at(filename);
  lock.unlock();

  ff.convertToUTF8();

  StructuredReport::Pointer report = StructuredReport::New();

  DcmDataset* fileData = ff.getDataset();
  report->reportData.read(*fileData);

  lock.lock();
  info.m_Data = report;
  lock.unlock();

  return true;
}

bool DicomSeriesReader::ImageBlockDescriptor::loadImage(DcmFileFormat& ff, ImageBlockDescriptor* src)
{
  if (!src) {
    src = this;
  }
  std::unique_lock<std::mutex> lock(*m_Mutex);
  auto filename = src->m_Filenames[0];
  auto& info= m_SlicesInfo.at(filename);
  lock.unlock();

  ff.convertToUTF8();

  //Load image
  if (!m_Philips3D) {
    DicomImage dcmImage(&ff, EXS_Unknown, /*CIF_AcrNemaCompatibility|CIF_WrongPaletteAttributeTags|CIF_IgnoreModalityTransformation|CIF_UseAbsolutePixelRange|*/CIF_UsePartialAccessToPixelData, 0, 1);
    if (dcmImage.getStatus() != EIS_Normal) {
      MITK_ERROR << "Can't load DICOM image \"" << filename << "\": " << DicomImage::getString(dcmImage.getStatus());
    }
    double max = std::numeric_limits<double>::max(), min = std::numeric_limits<double>::min();
    dcmImage.getMinMaxValues(min, max, 0);
    auto image = Image::New();
    { // Initialize mitk image
      const DiPixel* interData = dcmImage.getInterData();
      if (!interData) {
        return false; // not open this file
      }
      unsigned int d[3] = {dcmImage.getWidth(), dcmImage.getHeight(), m_NumberOfFrames>1? m_NumberOfFrames : 1};
      Vector3D spacing;
      GetDesiredMITKImagePixelSpacing(spacing[0], spacing[1]);
      spacing[2] = m_SliceDistance;
      image->Initialize(makePixelType(dcmImage), 3, d, 1, info.m_ImagePositionPatient, getMatrix(spacing), spacing);
    }
    // Fill image from file
    try {
      char* framePtr = (char*)image->GetVolumeData()->GetData();
      const auto bits = image->GetPixelType().GetBitsPerComponent();
      const auto frameSize = dcmImage.getOutputDataSize(bits);
      do {
        if (image->GetPixelType().GetPixelType() == itk::ImageIOBase::RGB) {
          dcmImage.getOutputData(framePtr, frameSize, bits);
        } else {
          copyInterData(framePtr, image->GetPixelType(), dcmImage);
        }
        framePtr += frameSize;
      } while (dcmImage.processNextFrames());
    } catch(mitk::Exception& e) {
      MITK_ERROR << "Error: cannot store DICOM image to mitk object (" << e.what() << ")";
      return false;
    }

    DcmDataset* fileData = ff.getDataset();
    DcmStack stack;

    auto& dictArray = image->AllocateMetaDataDictionaryArray();
    auto dict = dictArray[0];

    // For compatibility with private tags processing in itk::GDCMImageIO
    const gdcm::Dict& pubdict = gdcm::Global::GetInstance().GetDicts().GetPublicDict();

    while (fileData->nextObject(stack, OFTrue).good()) {
      auto elem = dynamic_cast<DcmElement *>(stack.top());
      if (!elem) {
        continue;
      }
      auto tag = elem->getTag();
      std::string value;
      if (tag.getGroup() == 0x7fe0 || !elem->valueLoaded()) {
        continue;
      }
      std::ostringstream key;
      key.fill('0');
      key.setf(std::ios::hex, std::ios::basefield);
      key << std::setw(4) << tag.getGroup() << '|' << std::setw(4) << tag.getElement();

      // Check for it is a binary tag known by gdcm
      gdcm::VR::VRType vrtype = pubdict.GetDictEntry(gdcm::Tag(tag.getGroup(), tag.getElement())).GetVR();
      if (vrtype & (gdcm::VR::OB_OW | gdcm::VR::OF | gdcm::VR::UN)) {
        std::vector<unsigned char> bin(elem->getLengthField());
        if (elem->getPartialValue(&bin[0], 0, bin.size()).good()) {
          OFStandard::encodeBase64(&bin[0], bin.size(), value);
          itk::EncapsulateMetaData(*dict, key.str(), value);
        }
      } else if (vrtype && elem->getOFStringArray(value).good() && !value.empty()) {
        itk::EncapsulateMetaData(*dict, key.str(), value);
      }
      // Other VRs may cause assertion fault in itk::GDCMImageIO::Write during export so we ignoring them
    }

    lock.lock();
    info.m_Data = image;
    lock.unlock();
  }
  return true;
}

void DicomSeriesReader::ImageBlockDescriptor::loadTags(DcmFileFormat& ff)
{
  std::unique_lock<std::mutex> lock(*m_Mutex);
  if (m_Filenames.empty()) {
    return;
  }
  auto image = m_SlicesInfo.at(m_Filenames[0]).m_Data;
  for (int i=1; i<m_Filenames.size() && !image; i++) {
    image = m_SlicesInfo.at(m_Filenames[i]).m_Data;
  }
  lock.unlock();

  if (!image) {
    return;
  }
  DcmDataset* fileData = ff.getDataset();
  DcmStack stack;

  while (fileData->nextObject(stack, OFTrue).good()) {
    auto elem = dynamic_cast<DcmElement *>(stack.top());
    if (!elem) {
      continue;
    }
    auto tag = elem->getTag();
    std::string value;
    if (tag.getGroup() == 0x7fe0 || !elem->valueLoaded() || elem->getOFStringArray(value).bad()) {
      continue;
    }
    std::ostringstream key;
    key.fill('0');
    key.setf(std::ios::hex, std::ios::basefield);
    key << std::setw(4) << tag.getGroup() << '|' << std::setw(4) << tag.getElement();
    auto valuePosition = tagToPropertyMap.find(key.str());
    if (valuePosition != tagToPropertyMap.end()) {
      std::string propertyKey = valuePosition->second;
      image->SetProperty(propertyKey.c_str(), StringProperty::New(value));
    }
  }
  m_PropertyList = image->GetPropertyList();
}

DicomSeriesReader::ImageBlockDescriptor::SortingKey DicomSeriesReader::ImageBlockDescriptor::sortingKey() const
{
  std::ostringstream key;
  std::unique_lock<std::mutex> lock(*m_Mutex);

  if (m_Filenames.empty())
  {
    mitkThrow() << "generating block UID failed because file list is empty!";
  }
  if (m_SeriesInstanceUID.empty())
  {
    mitkThrow() << "generating block UID for " << m_Filenames[0] << " failed because SeriesInstanceUID was not defined!";
  }

  key << m_PatientID;
  key << '\\' << m_SeriesInstanceUID;
  key << '\\' << m_Rows;
  key << '\\' << m_Columns;
  key << '\\' << m_PixelSpacing;
  key << '\\' << m_ImagerPixelSpacing;
  key.setf(std::ios::fixed, std::ios::floatfield);
  key.precision(5);
  key << '\\' << m_NumberOfFrames;

  // be a bit tolerant for orienatation, let only the first few digits matter (http://bugs.mitk.org/show_bug.cgi?id=12263)
  if (!mayBeMultiOriented())
  {
    key << '\\' << m_SliceThickness;
    key << '\\' << m_Orientation[0][0] << '\\' << m_Orientation[0][1] << '\\' << m_Orientation[0][2]
        << '\\' << m_Orientation[1][0] << '\\' << m_Orientation[1][1] << '\\' << m_Orientation[1][2];
  } else {
    key << ".\\" << (!IsMultiFrameImage() || m_SlicesInfo.empty()? m_SliceThickness: double(m_SlicesInfo.begin()->second.m_InstanceNumber));
    key << "\\.\\.\\.\\.\\.\\.";
  }
  if (!m_SlicesInfo.empty() && m_SlicesInfo.cbegin()->second.m_HasImagePositionPatient) {
    key << '*';
  }
  if (m_BlockID) {
    key << '.';
    key.width(4);
    key.fill('0');
    key << m_BlockID;
  }
  return key.str();
}

DicomSeriesReader::ImageBlockDescriptor::ImageBlockDescriptor():
  m_Mutex(std::make_shared<std::mutex>()),
  m_SliceThickness(0),
  m_SliceDistance(1),
  m_BlockID(0),
  m_MultiOriented(false),
  m_HasGantryTiltCorrected(false), // This flags can be setted in checkSliceDistance() or directly
  m_HasMultipleTimePoints(false), //
  m_BadSlicingDistance(false),   //
  m_Philips3D(false)
{
}

DicomSeriesReader::ImageBlockDescriptor::~ImageBlockDescriptor()
{
  // nothing
}

// TODO: REMOVE
DicomSeriesReader::ImageBlockDescriptor::ImageBlockDescriptor(const StringContainer& files):
  m_Mutex(std::make_shared<std::mutex>()),
  m_Filenames(files),
  m_SliceThickness(0),
  m_SliceDistance(1),
  m_BlockID(0),
  m_MultiOriented(false),
  m_HasGantryTiltCorrected(false), // This flags can be setted in checkSliceDistance() or directly
  m_HasMultipleTimePoints(false), //
  m_BadSlicingDistance(false),   //
  m_Philips3D(false)
{
}

/// Returns squared distance between two slices
/// or zero if they are from different times
///
double squaredStep(const DicomSeriesReader::SliceInfo& a, const DicomSeriesReader::SliceInfo& b, double prevStep = 1e10)
{
  if (!a.m_HasImagePositionPatient || !b.m_HasImagePositionPatient) {
    return 1;
  }
  auto d = (a.m_ImagePositionPatient - b.m_ImagePositionPatient).GetSquaredNorm();
  if (a.m_AcquisitionNumber != b.m_AcquisitionNumber || a.m_TemporalPosition != b.m_TemporalPosition) {
    return d < prevStep/4? 0 : d;
  }
  return d < 0.000001 ? 0 : d;
}

std::shared_ptr<DicomSeriesReader::ImageBlockDescriptor> DicomSeriesReader::ImageBlockDescriptor::checkSliceDistance(bool doSplit)
{
  std::unique_lock<std::mutex> lock(*m_Mutex);

  auto fileIter = m_Filenames.begin();
  auto sliceInfo = &(m_SlicesInfo.at(*fileIter));
  m_BadSlicingDistance = false;

  if (m_Filenames.size() < 3) {
    double d = 1;
    m_HasMultipleTimePoints =  m_Filenames.size() == 2 && (d = squaredStep(*sliceInfo, m_SlicesInfo.at(*++fileIter))) == 0;
    if (m_Filenames.size() == 2) {
      if (d > m_SliceThickness*m_SliceThickness) {
        m_SliceDistance = sqrt(d);
      }
    }
    return nullptr;
  }

  Point3D firstOrigin = sliceInfo->m_ImagePositionPatient;
  Point3D lastDifferentOrigin;
  Vector3D fromFirstToSecondOrigin;
  double step2;
  unsigned timeSteps = 0;
  auto referenceSlice = sliceInfo;

  do {
    if (++fileIter == m_Filenames.end()) {
      m_HasMultipleTimePoints = referenceSlice->m_HasImagePositionPatient && timeSteps > 1;
      return nullptr; // there is no second different point
    }
    ++timeSteps;
    referenceSlice = &(m_SlicesInfo.at(*fileIter));
    lastDifferentOrigin = referenceSlice->m_ImagePositionPatient;
    fromFirstToSecondOrigin = lastDifferentOrigin - firstOrigin;
    step2 = squaredStep(*referenceSlice, *sliceInfo);
  } while (step2 == 0);

  GantryTiltInformation tiltInfo(lastDifferentOrigin, firstOrigin, m_Orientation[0], m_Orientation[1], 1); // TODO: optimize GantryTiltInformation constructor or compute directly only necessary values
  m_HasGantryTiltCorrected = tiltInfo.IsSheared() && tiltInfo.IsRegularGantryTilt() && (  sliceInfo->m_GantryTilt.empty()
    || std::abs( OFStandard::atof(sliceInfo->m_GantryTilt.c_str()) - tiltInfo.GetTiltAngleInDegrees() ) <= 0.25  );

  double toleratedErrorWithWarning = step2*100;
  double toleratedError = 0.000025;
  unsigned timeSteps2 = 1;
  unsigned zSteps = 1;
  auto secondSlice = referenceSlice;
  auto preLastSlice = referenceSlice;
  while (++fileIter != m_Filenames.end()) {
    sliceInfo = &(m_SlicesInfo.at(*fileIter));
    if (squaredStep(*sliceInfo, *referenceSlice, step2) == 0) { // skip points from different time
      ++timeSteps2;
      continue;
    }
    if (timeSteps2 > timeSteps) {
      timeSteps = timeSteps2;
    }
    timeSteps2 = 1;

    Point3D assumedOrigin = lastDifferentOrigin + fromFirstToSecondOrigin;
    Point3D thisOrigin = sliceInfo->m_ImagePositionPatient;
    double squaredOriginError = (assumedOrigin - thisOrigin).GetSquaredNorm();
    if (doSplit && squaredOriginError > toleratedErrorWithWarning) {
      break;
    }
    if (squaredOriginError > toleratedError) {
      m_BadSlicingDistance = true;
    }
    lastDifferentOrigin = thisOrigin;
    preLastSlice = referenceSlice;
    referenceSlice = sliceInfo;
    ++zSteps;
  }
/*  if (timeSteps2 != timeSteps && timeSteps < m_Filenames.size()) {
    timeSteps = 1;
  }*/

  m_HasMultipleTimePoints = timeSteps > 1;
  m_MultiOriented = false;

  if (sliceInfo->m_Data) { // Correct spacing value to sqrt(step2)
    if (zSteps > 1) {
      auto last2 = squaredStep(*preLastSlice, *referenceSlice, step2);
      if (zSteps > 2) {
        auto mid2 = squaredStep(*secondSlice, *preLastSlice, step2) / ((zSteps-2)*(zSteps-2));
        if (mid2 < last2) {
          last2 = mid2;
        }
      }
      if (last2 < step2) {
        step2 = last2;
      }
    }
    m_SliceDistance = sqrt( step2 );
  }

  if (fileIter == m_Filenames.end()) {
    m_Filenames.shrink_to_fit();
    return nullptr;
  }

  std::shared_ptr<DicomSeriesReader::ImageBlockDescriptor> rest = clone();
  rest->m_Mutex = std::make_shared<std::mutex>();
  rest->m_Filenames.assign(fileIter, m_Filenames.end());
  rest->m_BlockID++;
  rest->m_BadSlicingDistance = false;

  m_Filenames.erase(fileIter, m_Filenames.end());
  m_Filenames.shrink_to_fit();
  // TODO: drop unused entries in splitted m_SlicesInfo-s
  return rest;
}

std::shared_ptr<DicomSeriesReader::ImageBlockDescriptor> DicomSeriesReader::ImageBlockDescriptor::clone()
{
  return std::make_shared<ImageBlockDescriptor>(*this);
}

void DicomSeriesReader::ImageBlockDescriptor::AddFile(ImageBlockDescriptor& file)
{
  std::unique_lock<std::mutex> lock(*m_Mutex);
  auto& newSliceInfo = m_SlicesInfo[file.m_Filenames[0]] = file.m_SlicesInfo.at(file.m_Filenames[0]);
  if ( !m_MultiOriented && (m_HasOrientation != newSliceInfo.m_HasOrientation
   || m_Filenames.size() && !std::equal(newSliceInfo.m_Orientation, newSliceInfo.m_Orientation+2, m_SlicesInfo.at(m_Filenames[0]).m_Orientation,
        [](const Vector3D& a, const Vector3D& b){ return (a-b).GetSquaredNorm() <= 0.00000001; })) ) {
    m_MultiOriented = true;
  }
  auto it= std::upper_bound(m_Filenames.begin(), m_Filenames.end(), file.m_Filenames[0], [this](const std::string& a, const std::string& b){ return m_SlicesInfo.at(a) < m_SlicesInfo.at(b); });
  if (m_MultiOriented && it == m_Filenames.begin()) {
    std::copy(file.m_Orientation, file.m_Orientation+2, m_Orientation);
  }
  m_Filenames.insert( it, file.m_Filenames[0] );
  file.DropImages();
}

void DicomSeriesReader::ImageBlockDescriptor::EraseFile(const ImageBlockDescriptor& file)
{
  std::unique_lock<std::mutex> lock(*m_Mutex);
  auto it = std::find(m_Filenames.begin(), m_Filenames.end(), file.m_Filenames[0]);
  if (it != m_Filenames.end()) {
    m_Filenames.erase(it);
  }
}

std::list<DicomSeriesReader::StringContainer> DicomSeriesReader::ImageBlockDescriptor::SortIntoBlocksFor3DplusT(bool& canLoadAs4D) const
{
  canLoadAs4D = true;
  std::list<StringContainer> imageBlocks;
  unsigned int maxSize = 1;

  if (m_MultiOriented) {
    canLoadAs4D = false;
    return {m_Filenames};
  }

  // loop files and sliceinfos
  for (StringContainer::const_iterator fileIter = m_Filenames.begin();
    fileIter != m_Filenames.end();
    ++fileIter)
  {
    auto sliceInfo = m_SlicesInfo.find(*fileIter);
    if (sliceInfo == m_SlicesInfo.end() || !sliceInfo->second.m_HasImagePositionPatient) {
      imageBlocks.clear();
      imageBlocks.push_back(m_Filenames);
      canLoadAs4D = false;
      break;
    }

    MITK_DEBUG << "  " << *fileIter << " at " << sliceInfo->second.m_ImagePositionPatient;
    auto bIt = std::find_if(imageBlocks.begin(), imageBlocks.end(), [sliceInfo, this](const StringContainer& x){
      return squaredStep(m_SlicesInfo.at(x.back()), sliceInfo->second, m_SliceDistance) != 0;
    });
    if (bIt == imageBlocks.end()) {
      bIt = imageBlocks.insert(bIt, {*fileIter});
    } else {
      bIt->push_back(*fileIter);
      if (maxSize < bIt->size()) {
        maxSize = bIt->size();
      }
    }
  }

  canLoadAs4D = imageBlocks.size()>1 && imageBlocks.size()*maxSize <= m_Filenames.size();
  if (!canLoadAs4D) {
    return {m_Filenames};
  }

  return imageBlocks;
}


Image::Pointer DicomSeriesReader::ImageBlockDescriptor::GetImage(size_t *slicesCnt)
{
  std::unique_lock<std::mutex> lock(*m_Mutex);
  StringLookupTable filesForSlices; //< for list of used files
  size_t sliceNo = 0;

  if (m_Filenames.empty()) {
    if (slicesCnt) {
      *slicesCnt = 0;
    }
    return Image::Pointer();
  }
  auto filename = m_Filenames[0];
  mitk::Image::Pointer firstImage = dynamic_cast<mitk::Image*>(m_SlicesInfo.at(filename).m_Data.GetPointer());
  for (int i=1; i<m_Filenames.size() && !firstImage; i++) {
    firstImage = dynamic_cast<mitk::Image*>(m_SlicesInfo.at(filename = m_Filenames[i]).m_Data.GetPointer());
  }
  if (m_Filenames.size()==1 || !firstImage) {
    if (slicesCnt) {
      *slicesCnt = !!firstImage;
    }
    if (firstImage) {
      filesForSlices.SetTableValue(0, filename);
      firstImage->SetProperty("files", StringLookupTableProperty::New(filesForSlices));
    }
    return firstImage;
  }

  bool canLoadAs4D= true;
  auto imageBlocks = SortIntoBlocksFor3DplusT(canLoadAs4D);
  Vector3D spacing;
  GetDesiredMITKImagePixelSpacing(spacing[0], spacing[1]);
  spacing[2] = m_SliceDistance;
  SetHasMultipleTimePoints(canLoadAs4D);

  lock.unlock();

  auto d0 = firstImage->GetDimensions();
  unsigned int d[4] = {d0[0], d0[1], (unsigned int)imageBlocks.front().size(), (unsigned int)imageBlocks.size()};
  auto slicedGeometry = firstImage->GetSlicedGeometry();
  auto origin = slicedGeometry->GetOrigin();

  auto image = Image::New();
  image->Initialize(firstImage->GetPixelType(), canLoadAs4D?4:3, d, 1, origin, getMatrix(spacing), spacing);
  auto& newDictArray = image->AllocateMetaDataDictionaryArray();
  auto dict = newDictArray.begin();

  const size_t sliceSize = d[0]*d[1]*(image->GetPixelType().GetBpe()/8);
  size_t slicesUsed = 0;
  try {
    char* dstPtr = (char*)image->GetVolumeData()->GetData();
    bool firstSlice = true;
    if (m_PropertyList) {
      image->SetPropertyList(m_PropertyList->Clone());
    }

    for (auto& block: imageBlocks) {
      for (auto& file: block) {
        lock.lock();
        auto& sliceInfo = m_SlicesInfo.at(file);
        mitk::Image::Pointer slice = dynamic_cast<mitk::Image*>(sliceInfo.m_Data.GetPointer());
        lock.unlock();
        if (slice) {
          filename = file;
        } else if (firstSlice) {// if the first image is not loaded we take the first of loaded
          slice = firstImage;
          slicesUsed--;
        }
        if (slice) {
          if (firstImage->GetDimension() != slice->GetDimension()) {
            throw mitk::Exception("Some slices has different dimensions");
          }
          if (image->GetPixelType().GetBpe() != slice->GetPixelType().GetBpe()) {
            throw mitk::Exception("Some slices has different pixel types");
          }
          const size_t sliceSize2 = slice->GetDimensions()[0]*slice->GetDimensions()[1]*(slice->GetPixelType().GetBpe()/8);
          if (sliceSize2 != sliceSize) {
            throw mitk::Exception("Some slices has different sizes");
          }
          memcpy(dstPtr, slice->GetVolumeData()->GetData(), sliceSize);
          if (!m_PropertyList || m_PropertyList->IsEmpty()) {
            // TODO: Copy mnemonic properties and merge hex properties with '|' to StringLookupTable-s
            auto props = slice->GetPropertyList();
            if (!props->IsEmpty()) {
              image->SetPropertyList(props->Clone());
            }
          }
          slicesUsed++;
          **dict = *slice->AllocateMetaDataDictionaryArray()[0];
        } else { // 1. Slice will be loaded later, but we know they number. 2. Slice can not be loaded
          // In this cases for not first slice we temporary copy the previous
          memcpy(dstPtr, dstPtr - sliceSize, sliceSize);
          **dict = *firstImage->AllocateMetaDataDictionaryArray()[0];
          if (sliceInfo.m_HasOrientation) {
            std::ostringstream value;
            value << sliceInfo.m_Orientation[0][0] << '\\' << sliceInfo.m_Orientation[0][1] << '\\' << sliceInfo.m_Orientation[0][2]
              << '\\' << sliceInfo.m_Orientation[1][0] << '\\' << sliceInfo.m_Orientation[1][1] << '\\' << sliceInfo.m_Orientation[1][2];
            itk::EncapsulateMetaData<std::string>(**dict, "0020|0037", value.str());
          }
          if (sliceInfo.m_HasImagePositionPatient) {
            std::ostringstream value;
            value << sliceInfo.m_ImagePositionPatient[0] << '\\' << sliceInfo.m_ImagePositionPatient[1] << '\\' << sliceInfo.m_ImagePositionPatient[2];
            itk::EncapsulateMetaData<std::string>(**dict, "0020|0032", value.str());
          }
        }
        filesForSlices.SetTableValue(sliceNo++, filename);
        assert(slicesUsed <= sliceNo);
        dstPtr += sliceSize;
        firstSlice = false;
        dict++;
      }
    }
    image->SetProperty("files", StringLookupTableProperty::New(filesForSlices));
  } catch(mitk::Exception& e) {
    MITK_ERROR << "Error: cannot build new mitk image (" << e.what() << ")";
    if (slicesCnt) {
      *slicesCnt = 0;
    }
    return Image::Pointer();
  }

  if (slicesCnt) {
    *slicesCnt = slicesUsed;
  }

  return image;
}

void DicomSeriesReader::ImageBlockDescriptor::DropImages()
{
  std::unique_lock<std::mutex> lock(*m_Mutex);
  for (auto& slice: m_SlicesInfo) {
    slice.second.m_Data = nullptr;
  }
}

DicomSeriesReader::StringContainer DicomSeriesReader::ImageBlockDescriptor::GetFilenames() const
{
  std::unique_lock<std::mutex> lock(*m_Mutex);
  return m_Filenames;
}

int DicomSeriesReader::ImageBlockDescriptor::GetImageBlockUID() const
{
  return m_BlockID;
}

std::string DicomSeriesReader::ImageBlockDescriptor::GetSeriesInstanceUID() const
{
  std::unique_lock<std::mutex> lock(*m_Mutex);
  return m_SeriesInstanceUID;
}

std::string DicomSeriesReader::ImageBlockDescriptor::GetModality() const
{
  std::unique_lock<std::mutex> lock(*m_Mutex);
  return m_Modality;
}

std::string DicomSeriesReader::ImageBlockDescriptor::GetSOPClassUIDAsString() const
{
  gdcm::UIDs uidKnowledge;
  uidKnowledge.SetFromUID( m_SOPClassUID.c_str() );
  return uidKnowledge.GetName();
}

std::string DicomSeriesReader::ImageBlockDescriptor::GetSOPClassUID() const
{
  std::unique_lock<std::mutex> lock(*m_Mutex);
  return m_SOPClassUID;
}

bool DicomSeriesReader::ImageBlockDescriptor::IsMultiFrameImage() const
{
  return m_NumberOfFrames > 1;
}

DicomSeriesReader::ReaderImplementationLevel DicomSeriesReader::ImageBlockDescriptor::GetReaderImplementationLevel() const
{
  if ( this->IsMultiFrameImage() )
    return ReaderImplementationLevel_Unsupported;

  gdcm::UIDs uidKnowledge;
  uidKnowledge.SetFromUID( m_SOPClassUID.c_str() );

  gdcm::UIDs::TSType uid = uidKnowledge;

  switch (uid)
  {
    case gdcm::UIDs::CTImageStorage:
    case gdcm::UIDs::MRImageStorage:
    case gdcm::UIDs::PositronEmissionTomographyImageStorage:
    case gdcm::UIDs::ComputedRadiographyImageStorage:
    case gdcm::UIDs::DigitalXRayImageStorageForPresentation:
    case gdcm::UIDs::DigitalXRayImageStorageForProcessing:
      return ReaderImplementationLevel_Supported;

    case gdcm::UIDs::NuclearMedicineImageStorage:
      return ReaderImplementationLevel_PartlySupported;

    case gdcm::UIDs::SecondaryCaptureImageStorage:
      return ReaderImplementationLevel_Implemented;

    default:
      return ReaderImplementationLevel_Unsupported;
  }
}

bool DicomSeriesReader::ImageBlockDescriptor::HasGantryTiltCorrected() const
{
  return m_HasGantryTiltCorrected;
}

/*
   PS defined      IPS defined     PS==IPS
        0               0                     --> UNKNOWN spacing, loader will invent
        0               1                     --> spacing as at detector surface
        1               0                     --> spacing as in patient
        1               1             0       --> detector surface spacing CORRECTED for geometrical magnifications: spacing as in patient
        1               1             1       --> detector surface spacing NOT corrected for geometrical magnifications: spacing as at detector
*/
DicomSeriesReader::PixelSpacingInterpretation DicomSeriesReader::ImageBlockDescriptor::GetPixelSpacingType() const
{
  if (m_PixelSpacing.empty())
  {
    if (m_ImagerPixelSpacing.empty())
    {
      return PixelSpacingInterpretation_SpacingUnknown;
    }
    else
    {
      return PixelSpacingInterpretation_SpacingAtDetector;
    }
  }
  else // Pixel Spacing defined
  {
    if (m_ImagerPixelSpacing.empty())
    {
      return PixelSpacingInterpretation_SpacingInPatient;
    }
    else if (m_PixelSpacing != m_ImagerPixelSpacing)
    {
      return PixelSpacingInterpretation_SpacingInPatient;
    }
    else
    {
      return PixelSpacingInterpretation_SpacingAtDetector;
    }
  }
}

bool DicomSeriesReader::ImageBlockDescriptor::PixelSpacingRelatesToPatient() const
{
  return  GetPixelSpacingType() == PixelSpacingInterpretation_SpacingInPatient;
}

bool DicomSeriesReader::ImageBlockDescriptor::PixelSpacingRelatesToDetector() const
{
  return  GetPixelSpacingType() == PixelSpacingInterpretation_SpacingAtDetector;
}

bool DicomSeriesReader::ImageBlockDescriptor::PixelSpacingIsUnknown() const
{
  return GetPixelSpacingType() == PixelSpacingInterpretation_SpacingUnknown;
}

void DicomSeriesReader::ImageBlockDescriptor::SetPixelSpacingInformation(const std::string& pixelSpacing, const std::string& imagerPixelSpacing)
{
  m_PixelSpacing = pixelSpacing;
  m_ImagerPixelSpacing = imagerPixelSpacing;
}

void DicomSeriesReader::ImageBlockDescriptor::GetDesiredMITKImagePixelSpacing( ScalarType& spacingX, ScalarType& spacingY) const
{
  // preference for "in patient" pixel spacing
  if ( !DICOMStringToSpacing( m_PixelSpacing, spacingX, spacingY ) )
  {
    // fallback to "on detector" spacing
    if ( !DICOMStringToSpacing( m_ImagerPixelSpacing, spacingX, spacingY ) )
    {
      // last resort: invent something
      spacingX = spacingY = 1.0;
    }
  }
}

bool DicomSeriesReader::ImageBlockDescriptor::HasMultipleTimePoints() const
{
  return m_HasMultipleTimePoints;
}

void DicomSeriesReader::ImageBlockDescriptor::SetImageBlockUID(int uid)
{
  m_BlockID = uid;
}

void DicomSeriesReader::ImageBlockDescriptor::SetSeriesInstanceUID(const std::string& uid)
{
  std::unique_lock<std::mutex> lock(*m_Mutex);
  m_SeriesInstanceUID = uid;
}

void DicomSeriesReader::ImageBlockDescriptor::SetModality(const std::string& modality)
{
  std::unique_lock<std::mutex> lock(*m_Mutex);
  m_Modality = modality;
}

void DicomSeriesReader::ImageBlockDescriptor::SetSOPClassUID(const std::string& sopClassUID)
{
  std::unique_lock<std::mutex> lock(*m_Mutex);
  m_SOPClassUID = sopClassUID;
}


void DicomSeriesReader::ImageBlockDescriptor::SetHasGantryTiltCorrected(bool on)
{
  m_HasGantryTiltCorrected = on;
}

void DicomSeriesReader::ImageBlockDescriptor::SetHasMultipleTimePoints(bool on)
{
  m_HasMultipleTimePoints = on;
}

const Vector3D* DicomSeriesReader::ImageBlockDescriptor::GetOrientation() const
{
  return m_Orientation;
}
bool DicomSeriesReader::ImageBlockDescriptor::HasOrientation() const
{
  return m_HasOrientation;
}

bool DicomSeriesReader::ImageBlockDescriptor::IsBadSlicingDistance() const
{
  return m_BadSlicingDistance;
}

void DicomSeriesReader::ImageBlockDescriptor::SetBadSlicingDistance(bool badSlicingDistance)
{
  m_BadSlicingDistance = badSlicingDistance;
}

std::string DicomSeriesReader::ImageBlockDescriptor::PhotometricInterpretation() const
{
  return m_PhotometricInterpretation;
}

void DicomSeriesReader::ImageBlockDescriptor::SetPhotometricInterpretation(std::string interpretation)
{
  m_PhotometricInterpretation = interpretation;
}

std::string DicomSeriesReader::ImageBlockDescriptor::GetPixelSpacing()
{
  return m_PixelSpacing;
}

std::string DicomSeriesReader::ImageBlockDescriptor::GetImagerPixelSpacing()
{
  return m_ImagerPixelSpacing;
}

unsigned long long DicomSeriesReader::ImageBlockDescriptor::filesSize() const
{
  unsigned long long r = 0;
  for (const auto& filename: m_Filenames) {
    r += m_SlicesInfo.at(filename).m_FileSize;
  }
  return r;
}

bool DicomSeriesReader::SliceInfo::operator<(const DicomSeriesReader::SliceInfo& b) const
{
  // This method MUST accept missing location and position information (and all else, too)
  // because we cannot rely on anything
  // (restriction on the sentence before: we have to provide consistent sorting, so we
  // rely on the minimum information all DICOM files need to provide: SOP Instance UID)

  /* we CAN expect a group of equal
     - series instance uid
     - image orientation
     - pixel spacing
     - imager pixel spacing
     - slice thickness
     - number of rows/columns
  */

  // TODO: move this parsing and distance calculation to ImageBlockDescriptor constructor
  const auto& imagePosition = m_ImagePositionPatient;
  const auto& bImagePosition = b.m_ImagePositionPatient;

  // See if we have Image Position and Orientation which is equal
  if ( m_HasImagePositionPatient && m_HasOrientation && b.m_HasImagePositionPatient && b.m_HasOrientation
    && (m_Orientation[0] - b.m_Orientation[0]).squared_magnitude() <= 0.00000001
    && (m_Orientation[1] - b.m_Orientation[1]).squared_magnitude() <= 0.00000001 ) {

    // Compute the distance from world origin (0,0,0) ALONG THE MEAN of the two NORMALS of the slices
    auto normal = (vnl_cross_3d(vnl_vector_fixed<double,3>(m_Orientation[0]), vnl_vector_fixed<double,3>(m_Orientation[1]))
               + vnl_cross_3d(vnl_vector_fixed<double,3>(b.m_Orientation[0]), vnl_vector_fixed<double,3>(b.m_Orientation[1])));

    double
      dist1 = 0.0,
      dist2 = 0.0;

    for (unsigned char i = 0u; i < 3u; ++i) {
      dist1 += normal[i] * imagePosition[i];
      dist2 += normal[i] * bImagePosition[i];
    }

    // We need to check more properties to distinguish slices
    if (fabs(dist1 - dist2) >= mitk::eps) {
      // Default: compare position
      return dist1 < dist2;
    }
  }
  // Try to sort by Acquisition Number
  if (m_AcquisitionNumber != b.m_AcquisitionNumber) {
    return m_AcquisitionNumber < b.m_AcquisitionNumber;
  }
  // Neither position nor acquisition number are good for sorting, so check more
  // Let's try temporal index
  if (m_TemporalPosition != b.m_TemporalPosition) {
    return m_TemporalPosition < b.m_TemporalPosition;
  }
  if (m_InstanceNumber != b.m_InstanceNumber) {
    return m_InstanceNumber < b.m_InstanceNumber;
  }

  // LAST RESORT: all valuable information for sorting is missing.
  // Sort by some meaningless but unique identifiers to satisfy the sort function
  if (!m_SOPInstanceUID.empty() && !b.m_SOPInstanceUID.empty()) {
    MITK_DEBUG << "Dicom images are missing attributes for a meaningful sorting, falling back to SOP instance UID comparison.";
    return m_SOPInstanceUID < b.m_SOPInstanceUID;
  }
  else {
    return false /*m_Filename < b.m_Filename*/;
  }
}

} // end namespace mitk
