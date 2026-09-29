/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

//#define MBILOG_ENABLE_DEBUG

#include <dcmtk/dcmdata/dcvrdt.h>
#include <dcmtk/ofstd/ofstd.h>

#include <mitkITKDICOMSeriesReaderHelper.h>
#include <mitkITKDICOMSeriesReaderHelper.tpp>

#include <mitkDICOMGDCMTagScanner.h>
#include <mitkDICOMTimeUtil.h>
#include <mitkArbitraryTimeGeometry.h>
#include <mitkImageReadAccessor.h>
#include <mitkImageWriteAccessor.h>

#include <dcmtk/dcmdata/dcvrda.h>

#include <itkMetaDataObject.h>

#include <gdcmRescaler.h>

#include <algorithm>
#include <cmath>
#include <sstream>
#include <type_traits>
#include <vector>


const mitk::DICOMTag mitk::ITKDICOMSeriesReaderHelper::AcquisitionDateTag = mitk::DICOMTag( 0x0008, 0x0022 );
const mitk::DICOMTag mitk::ITKDICOMSeriesReaderHelper::AcquisitionTimeTag = mitk::DICOMTag( 0x0008, 0x0032 );
const mitk::DICOMTag mitk::ITKDICOMSeriesReaderHelper::TriggerTimeTag = mitk::DICOMTag( 0x0018, 0x1060 );

#define switchTypeCase(Dim, IOType, T) \
  case IOType:                    \
    return LoadDICOMByITK<T, Dim>( filenames, correctTilt, tiltInfo, io );

namespace
{
  using Rescale = mitk::DICOMFrameLayout::Rescale;

  gdcm::PixelFormat::ScalarType ToGDCMScalarType(itk::IOComponentEnum component)
  {
    switch (component)
    {
      case itk::IOComponentEnum::UCHAR:  return gdcm::PixelFormat::UINT8;
      case itk::IOComponentEnum::CHAR:   return gdcm::PixelFormat::INT8;
      case itk::IOComponentEnum::USHORT: return gdcm::PixelFormat::UINT16;
      case itk::IOComponentEnum::SHORT:  return gdcm::PixelFormat::INT16;
      case itk::IOComponentEnum::UINT:   return gdcm::PixelFormat::UINT32;
      case itk::IOComponentEnum::INT:    return gdcm::PixelFormat::INT32;
      default:                           return gdcm::PixelFormat::FLOAT64;
    }
  }

  mitk::PixelType ToMitkPixelType(gdcm::PixelFormat::ScalarType type)
  {
    switch (type)
    {
      case gdcm::PixelFormat::UINT8:  return mitk::MakeScalarPixelType<unsigned char>();
      case gdcm::PixelFormat::INT8:   return mitk::MakeScalarPixelType<signed char>();
      case gdcm::PixelFormat::UINT16: return mitk::MakeScalarPixelType<unsigned short>();
      case gdcm::PixelFormat::INT16:  return mitk::MakeScalarPixelType<short>();
      case gdcm::PixelFormat::UINT32: return mitk::MakeScalarPixelType<unsigned int>();
      case gdcm::PixelFormat::INT32:  return mitk::MakeScalarPixelType<int>();
      case gdcm::PixelFormat::FLOAT32: return mitk::MakeScalarPixelType<float>();
      default:                        return mitk::MakeScalarPixelType<double>();
    }
  }

  /** A Pixel Module attribute as ITK's GDCM IO records it in its dictionary. */
  bool ReadPixelModuleAttribute(const itk::MetaDataDictionary& dictionary, const char* key, unsigned short& value)
  {
    std::string text;
    if (!itk::ExposeMetaData<std::string>(dictionary, key, text))
    {
      return false;
    }

    std::istringstream stream(text);
    return static_cast<bool>(stream >> value);
  }

  /**
   * The stored format as the Pixel Module declares it.
   *
   * GDCM sizes its rescale output by Bits Stored, so the same declaration has
   * to go into the per-frame rule, or a file storing 12 bits in 16 would get a
   * wider type here than GDCM gives it for a uniform transformation. Float
   * stored data and a dictionary without a usable declaration fall back to the
   * IO's component type.
   */
  gdcm::PixelFormat StoredPixelFormat(const itk::GDCMImageIO& io)
  {
    const auto component = io.GetInternalComponentType();
    const gdcm::PixelFormat fallback(ToGDCMScalarType(component));

    if (itk::IOComponentEnum::FLOAT == component || itk::IOComponentEnum::DOUBLE == component)
    {
      return fallback;
    }

    const auto& dictionary = io.GetMetaDataDictionary();
    unsigned short bitsAllocated = 0;
    unsigned short bitsStored = 0;
    unsigned short highBit = 0;
    unsigned short pixelRepresentation = 0;
    if (!ReadPixelModuleAttribute(dictionary, "0028|0100", bitsAllocated)
        || !ReadPixelModuleAttribute(dictionary, "0028|0101", bitsStored)
        || !ReadPixelModuleAttribute(dictionary, "0028|0102", highBit)
        || !ReadPixelModuleAttribute(dictionary, "0028|0103", pixelRepresentation)
        || 0 == bitsStored || bitsStored > bitsAllocated || bitsAllocated > 32 || pixelRepresentation > 1)
    {
      return fallback;
    }

    return gdcm::PixelFormat(1, bitsAllocated, bitsStored, highBit, pixelRepresentation);
  }

  /** GDCM's own output-type rule for one pair. */
  gdcm::PixelFormat::ScalarType RescaledType(const gdcm::PixelFormat& stored, const Rescale& rescale)
  {
    // GDCM's rule sizes an integer output by the stored range, and a float
    // format has none; asking for it asserts inside GDCM.
    const auto storedType = stored.GetScalarType();
    if (gdcm::PixelFormat::FLOAT16 == storedType || gdcm::PixelFormat::FLOAT32 == storedType
        || gdcm::PixelFormat::FLOAT64 == storedType)
    {
      return gdcm::PixelFormat::FLOAT64;
    }

    gdcm::Rescaler rescaler;
    rescaler.SetPixelFormat(stored);
    rescaler.SetSlope(rescale.slope);
    rescaler.SetIntercept(rescale.intercept);

    return rescaler.ComputeInterceptSlopePixelType();
  }

  /** The narrowest of GDCM's types that holds both, so that a uniform and a
      varying file of the same kind load with the same type. */
  gdcm::PixelFormat::ScalarType Widest(gdcm::PixelFormat::ScalarType left, gdcm::PixelFormat::ScalarType right)
  {
    if (left == right)
    {
      return left;
    }

    if (gdcm::PixelFormat::FLOAT64 == left || gdcm::PixelFormat::FLOAT64 == right
        || gdcm::PixelFormat::FLOAT32 == left || gdcm::PixelFormat::FLOAT32 == right)
    {
      return gdcm::PixelFormat::FLOAT64;
    }

    const double lowest = std::min(gdcm::PixelFormat(left).GetMin(), gdcm::PixelFormat(right).GetMin());
    const double highest = std::max(gdcm::PixelFormat(left).GetMax(), gdcm::PixelFormat(right).GetMax());

    for (const auto candidate : { gdcm::PixelFormat::UINT8, gdcm::PixelFormat::INT8,
                                  gdcm::PixelFormat::UINT16, gdcm::PixelFormat::INT16,
                                  gdcm::PixelFormat::UINT32, gdcm::PixelFormat::INT32 })
    {
      if (gdcm::PixelFormat(candidate).GetMin() <= lowest && highest <= gdcm::PixelFormat(candidate).GetMax())
      {
        return candidate;
      }
    }

    return gdcm::PixelFormat::FLOAT64;
  }

  template <typename TPixel>
  void ToDouble(const void* source, double* target, std::size_t count)
  {
    const auto* typed = static_cast<const TPixel*>(source);
    for (std::size_t i = 0; i < count; ++i)
    {
      target[i] = static_cast<double>(typed[i]);
    }
  }

  /**
   * The recovery can leave an integral target a few ULPs below the whole
   * value that the frame's own pair yields; truncating that would cost one
   * slope step.
   */
  template <typename TPixel>
  void FromDouble(const double* source, void* target, std::size_t count)
  {
    auto* typed = static_cast<TPixel*>(target);
    for (std::size_t i = 0; i < count; ++i)
    {
      if constexpr (std::is_integral_v<TPixel>)
      {
        typed[i] = static_cast<TPixel>(std::round(source[i]));
      }
      else
      {
        typed[i] = static_cast<TPixel>(source[i]);
      }
    }
  }

#define mitkDicomRescaleDispatch(function, componentType, first, second, count)      \
  switch (componentType)                                                             \
  {                                                                                  \
    case itk::IOComponentEnum::UCHAR:  function<unsigned char>(first, second, count); break;  \
    case itk::IOComponentEnum::CHAR:   function<signed char>(first, second, count); break;    \
    case itk::IOComponentEnum::USHORT: function<unsigned short>(first, second, count); break; \
    case itk::IOComponentEnum::SHORT:  function<short>(first, second, count); break;          \
    case itk::IOComponentEnum::UINT:   function<unsigned int>(first, second, count); break;   \
    case itk::IOComponentEnum::INT:    function<int>(first, second, count); break;            \
    case itk::IOComponentEnum::ULONG:  function<unsigned long>(first, second, count); break;  \
    case itk::IOComponentEnum::LONG:   function<long>(first, second, count); break;           \
    case itk::IOComponentEnum::FLOAT:  function<float>(first, second, count); break;          \
    default:                           function<double>(first, second, count); break;         \
  }

  /**
   * Replaces GDCM's single Pixel Value Transformation by each frame's own.
   *
   * gdcm::Image holds one slope and one intercept for the whole buffer. For the
   * enhanced SOP classes GDCM knows, that pair comes from the shared functional
   * group, or else from the first per-frame item; for any other class it is the
   * top-level pair. Which one GDCM chose does not matter here, because the
   * applied pair is read back from the IO. Undoing it and applying the frame's
   * pair is exact when GDCM's output type is integral, because GDCM chose a
   * type that holds stored * m + b exactly, and within double rounding
   * otherwise. Real World Value Mapping is deliberately not applied; GDCM does
   * not apply it either and it stays a consumer concern.
   */
  mitk::Image::Pointer ApplyPerFrameRescale(mitk::Image* loaded,
                                            const itk::GDCMImageIO& io,
                                            const mitk::DICOMFrameLayout& layout)
  {
    if (nullptr == loaded || layout.perFrameRescale.empty() || loaded->GetDimension() < 3)
    {
      return loaded;
    }

    const unsigned int sliceCount = loaded->GetDimension(2);
    if (sliceCount < 2)
    {
      return loaded;
    }

    const auto effective = mitk::EffectivePerFrameRescale(layout);
    if (effective.size() != sliceCount)
    {
      MITK_WARN << "The frame layout describes " << effective.size() << " frames but the image has "
                << sliceCount << " slices. Leaving the Pixel Value Transformation as GDCM applied it.";
      return loaded;
    }

    const Rescale applied{ io.GetRescaleSlope(), io.GetRescaleIntercept() };
    if (0.0 == applied.slope)
    {
      MITK_WARN << "GDCM reports a rescale slope of zero; the applied transformation cannot be undone.";
      return loaded;
    }

    if (std::all_of(effective.cbegin(), effective.cend(),
                    [&applied](const Rescale& rescale) { return mitk::SameRescale(rescale, applied); }))
    {
      return loaded;
    }

    if (loaded->GetPixelType().GetNumberOfComponents() > 1)
    {
      MITK_WARN << "The per-frame Pixel Value Transformation is not applied to multi-component "
                << "(for example colour) images.";
      return loaded;
    }

    const auto storedFormat = StoredPixelFormat(io);
    auto targetType = RescaledType(storedFormat, effective.front());
    for (const auto& rescale : effective)
    {
      targetType = Widest(targetType, RescaledType(storedFormat, rescale));
    }

    const mitk::PixelType targetPixelType = ToMitkPixelType(targetType);
    const auto sourceComponent = loaded->GetPixelType().GetComponentType();
    const auto targetComponent = targetPixelType.GetComponentType();

    const std::size_t pixelsPerSlice =
      static_cast<std::size_t>(loaded->GetDimension(0)) * loaded->GetDimension(1);
    const std::size_t sourceStride = pixelsPerSlice * loaded->GetPixelType().GetSize();
    const std::size_t targetStride = pixelsPerSlice * targetPixelType.GetSize();

    // One slice of doubles rather than the whole volume: an Enhanced CT of 500
    // frames at 512 squared would otherwise need a gigabyte of them.
    std::vector<double> values(pixelsPerSlice);
    unsigned int corrected = 0;

    const auto RescaleSlice = [&](const void* source, void* target, unsigned int z)
    {
      mitkDicomRescaleDispatch(ToDouble, sourceComponent, source, values.data(), pixelsPerSlice);

      if (!mitk::SameRescale(effective[z], applied))
      {
        for (std::size_t i = 0; i < pixelsPerSlice; ++i)
        {
          values[i] =
            (values[i] - applied.intercept) / applied.slope * effective[z].slope + effective[z].intercept;
        }
        ++corrected;
      }

      mitkDicomRescaleDispatch(FromDouble, targetComponent, values.data(), target, pixelsPerSlice);
    };

    mitk::Image::Pointer result = loaded;

    if (targetComponent == sourceComponent)
    {
      // Same width in and out, so the correction is written back over the pixels
      // it read. A write accessor serves both directions; taking a read and a
      // write accessor on one image at the same time would block.
      mitk::ImageWriteAccessor writer(loaded, loaded->GetVolumeData(0));
      auto* volume = static_cast<unsigned char*>(writer.GetData());
      for (unsigned int z = 0; z < sliceCount; ++z)
      {
        RescaleSlice(volume + z * sourceStride, volume + z * targetStride, z);
      }
    }
    else
    {
      result = mitk::Image::New();
      result->Initialize(targetPixelType, *loaded->GetTimeGeometry(), 1, 1);

      mitk::ImageWriteAccessor writer(result, result->GetVolumeData(0));
      auto* target = static_cast<unsigned char*>(writer.GetData());
      mitk::ImageReadAccessor reader(loaded, loaded->GetVolumeData(0));
      const auto* volume = static_cast<const unsigned char*>(reader.GetData());
      for (unsigned int z = 0; z < sliceCount; ++z)
      {
        RescaleSlice(volume + z * sourceStride, target + z * targetStride, z);
      }
      result->Modified();
    }

    MITK_INFO << "Applied the per-frame Pixel Value Transformation to " << corrected << " of "
              << effective.size() << " frames.";

    return result;
  }
}

bool mitk::ITKDICOMSeriesReaderHelper::CanHandleFile( const std::string& filename )
{
  MITK_DEBUG << "ITKDICOMSeriesReaderHelper::CanHandleFile " << filename;
  itk::GDCMImageIO::Pointer tester = itk::GDCMImageIO::New();
  return tester->CanReadFile( filename.c_str() );
}

template<unsigned int TDim>
mitk::Image::Pointer
mitk::ITKDICOMSeriesReaderHelper::LoadByTypeDispatch(const StringContainer& filenames,
  bool correctTilt,
  const GantryTiltInformation& tiltInfo,
  itk::GDCMImageIO::Pointer& io)
{
  if (io->GetPixelType() == itk::IOPixelEnum::SCALAR)
  {
    switch (io->GetComponentType())
    {
      switchTypeCase( TDim, itk::IOComponentEnum::UCHAR, unsigned char)
      switchTypeCase( TDim, itk::IOComponentEnum::CHAR, char)
      switchTypeCase( TDim, itk::IOComponentEnum::USHORT, unsigned short)
      switchTypeCase( TDim, itk::IOComponentEnum::SHORT, short)
      switchTypeCase( TDim, itk::IOComponentEnum::UINT, unsigned int)
      switchTypeCase( TDim, itk::IOComponentEnum::INT, int)
      switchTypeCase( TDim, itk::IOComponentEnum::ULONG, long unsigned int)
      switchTypeCase( TDim, itk::IOComponentEnum::LONG, long int)
      switchTypeCase( TDim, itk::IOComponentEnum::FLOAT, float)
      switchTypeCase( TDim, itk::IOComponentEnum::DOUBLE, double)
      default: MITK_ERROR
        << "Found unsupported DICOM scalar pixel type: (enum value) "
        << io->GetComponentType();
    }
  }
  else if (io->GetPixelType() == itk::IOPixelEnum::RGB)
  {
    switch (io->GetComponentType())
    {
      switchTypeCase( TDim, itk::IOComponentEnum::UCHAR, itk::RGBPixel<unsigned char>)
      switchTypeCase( TDim, 
       itk::IOComponentEnum::CHAR, itk::RGBPixel<char>) switchTypeCase( TDim, itk::IOComponentEnum::USHORT,
          itk::RGBPixel<unsigned short>)
        switchTypeCase( TDim, itk::IOComponentEnum::SHORT, itk::RGBPixel<short>) switchTypeCase( TDim, 
          itk::IOComponentEnum::UINT, itk::RGBPixel<unsigned int>) switchTypeCase( TDim, itk::IOComponentEnum::INT, itk::RGBPixel<int>)
        switchTypeCase( TDim, itk::IOComponentEnum::ULONG, itk::RGBPixel<long unsigned int>)
        switchTypeCase( TDim, itk::IOComponentEnum::LONG, itk::RGBPixel<long int>) switchTypeCase( TDim, 
          itk::IOComponentEnum::FLOAT, itk::RGBPixel<float>) switchTypeCase( TDim, itk::IOComponentEnum::DOUBLE,
            itk::RGBPixel<double>) default
    : MITK_ERROR
        << "Found unsupported DICOM scalar pixel type: (enum value) "
        << io->GetComponentType();
    }
  }
  MITK_ERROR << "Unsupported DICOM pixel type";
  return nullptr;
}

mitk::Image::Pointer mitk::ITKDICOMSeriesReaderHelper::Load( const StringContainer& filenames,
                                                             bool correctTilt,
                                                             const GantryTiltInformation& tiltInfo,
                                                             const DICOMFrameLayout& layout )
{
  if ( filenames.empty() )
  {
    MITK_DEBUG
      << "Calling LoadDicomSeries with empty filename string container. Probably invalid application logic.";
    return nullptr; // this is not actually an error but the result is very simple
  }

  typedef itk::GDCMImageIO DcmIoType;
  DcmIoType::Pointer io = DcmIoType::New();

  try
  {
    if ( io->CanReadFile( filenames.front().c_str() ) )
    {
      io->SetFileName( filenames.front().c_str() );
      io->ReadImageInformation();

      if(io->GetNumberOfDimensions()==2 || io->GetSpacing(2)==0.)
      {
        if (filenames.size() > 1)
        {
          MITK_ERROR << "Invalid application logic was called to load multiple DICOM files into one image volume, but at least one DICOM file indicated that it is 2D.";
          return nullptr;
        }

        return LoadByTypeDispatch<2>(filenames, correctTilt, tiltInfo, io);
      }
      else
      {
        mitk::Image::Pointer loaded = LoadByTypeDispatch<3>(filenames, correctTilt, tiltInfo, io);

        // Only a single file can carry a frame layout, and io now reports what
        // GDCM actually applied to the buffer it just read.
        return 1 == filenames.size() ? ApplyPerFrameRescale(loaded, *io, layout) : loaded;
      }
    }
  }
  catch ( const itk::MemoryAllocationError& e )
  {
    MITK_ERROR << "Out of memory. Cannot load DICOM series: " << e.what();
  }
  catch ( const std::exception& e )
  {
    MITK_ERROR << "Error encountered when loading DICOM series:" << e.what();
  }
  catch ( ... )
  {
    MITK_ERROR << "Unspecified error encountered when loading DICOM series.";
  }

  return nullptr;
}

#define switch3DnTCase( IOType, T ) \
  case IOType:                      \
    return LoadDICOMByITK3DnT<T>( filenamesLists, correctTilt, tiltInfo, io );

mitk::Image::Pointer mitk::ITKDICOMSeriesReaderHelper::Load3DnT( const StringContainerList& filenamesLists,
                                                                 bool correctTilt,
                                                                 const GantryTiltInformation& tiltInfo )
{
  if ( filenamesLists.empty() || filenamesLists.front().empty() )
  {
    MITK_DEBUG
      << "Calling LoadDicomSeries with empty filename string container. Probably invalid application logic.";
    return nullptr; // this is not actually an error but the result is very simple
  }

  typedef itk::GDCMImageIO DcmIoType;
  DcmIoType::Pointer io = DcmIoType::New();

  try
  {
    if ( io->CanReadFile( filenamesLists.front().front().c_str() ) )
    {
      io->SetFileName( filenamesLists.front().front().c_str() );
      io->ReadImageInformation();

      if ( io->GetPixelType() == itk::IOPixelEnum::SCALAR )
      {
        switch ( io->GetComponentType() )
        {
          switch3DnTCase(itk::IOComponentEnum::UCHAR, unsigned char) switch3DnTCase(itk::IOComponentEnum::CHAR, char)
              switch3DnTCase(itk::IOComponentEnum::USHORT, unsigned short) switch3DnTCase(
                itk::IOComponentEnum::SHORT, short) switch3DnTCase(itk::IOComponentEnum::UINT,
                                                        unsigned int) switch3DnTCase(itk::IOComponentEnum::INT, int)
                switch3DnTCase(itk::IOComponentEnum::ULONG, long unsigned int) switch3DnTCase(itk::IOComponentEnum::LONG, long int)
                  switch3DnTCase(itk::IOComponentEnum::FLOAT, float) switch3DnTCase(itk::IOComponentEnum::DOUBLE, double) default
            : MITK_ERROR
              << "Found unsupported DICOM scalar pixel type: (enum value) "
              << io->GetComponentType();
        }
      }
      else if ( io->GetPixelType() == itk::IOPixelEnum::RGB )
      {
        switch ( io->GetComponentType() )
        {
          switch3DnTCase(itk::IOComponentEnum::UCHAR, itk::RGBPixel<unsigned char>)
              switch3DnTCase(itk::IOComponentEnum::CHAR, itk::RGBPixel<char>) switch3DnTCase(
                itk::IOComponentEnum::USHORT, itk::RGBPixel<unsigned short>) switch3DnTCase(itk::IOComponentEnum::SHORT,
                                                                                 itk::RGBPixel<short>)
                switch3DnTCase(itk::IOComponentEnum::UINT, itk::RGBPixel<unsigned int>) switch3DnTCase(
                  itk::IOComponentEnum::INT, itk::RGBPixel<int>) switch3DnTCase(itk::IOComponentEnum::ULONG,
                                                                     itk::RGBPixel<long unsigned int>)
                  switch3DnTCase(itk::IOComponentEnum::LONG, itk::RGBPixel<long int>) switch3DnTCase(
                    itk::IOComponentEnum::FLOAT, itk::RGBPixel<float>) switch3DnTCase(itk::IOComponentEnum::DOUBLE,
                                                                           itk::RGBPixel<double>) default
            : MITK_ERROR
              << "Found unsupported DICOM scalar pixel type: (enum value) "
              << io->GetComponentType();
        }
      }

      MITK_ERROR << "Unsupported DICOM pixel type";
      return nullptr;
    }
  }
  catch ( const itk::MemoryAllocationError& e )
  {
    MITK_ERROR << "Out of memory. Cannot load DICOM series: " << e.what();
  }
  catch ( const std::exception& e )
  {
    MITK_ERROR << "Error encountered when loading DICOM series:" << e.what();
  }
  catch ( ... )
  {
    MITK_ERROR << "Unspecified error encountered when loading DICOM series.";
  }

  return nullptr;
}

bool ConvertDICOMDateTimeString( const std::string& dateString,
                                 const std::string& timeString,
                                 OFDateTime& time )
{
  OFString content( timeString.c_str() );

  if ( !dateString.empty() )
  {
    content = OFString( dateString.c_str() ).append( content );
  }
  else
  {
    // This is a workaround for DICOM data that has an AquisitionTime but no AquisitionDate.
    // In this case, we use the current date. That's not really nice, but is absolutely OK
    // as we're only interested in the time anyways...
    OFString currentDate;
    DcmDate::getCurrentDate( currentDate );
    content = currentDate.append( content );
  }

  const OFCondition result = DcmDateTime::getOFDateTimeFromString( content, time );

  return result.good();
}

OFDateTime GetLowerDateTime( const OFDateTime& time1, const OFDateTime& time2 )
{
  OFDateTime result = time1;

  if ( ( time2.getDate() < time1.getDate() )
       || ( ( time2.getDate() == time1.getDate() ) && ( time2.getTime() < time1.getTime() ) ) )
  {
    result = time2;
  }

  return result;
}

OFDateTime GetUpperDateTime( const OFDateTime& time1, const OFDateTime& time2 )
{
  OFDateTime result = time1;

  if ( ( time2.getDate() > time1.getDate() )
       || ( ( time2.getDate() == time1.getDate() ) && ( time2.getTime() > time1.getTime() ) ) )
  {
    result = time2;
  }

  return result;
}

bool mitk::ITKDICOMSeriesReaderHelper::ExtractDateTimeBoundsAndTriggerOfTimeStep(
  const StringContainer& filenamesOfTimeStep, DateTimeBounds& bounds, TimeBounds& triggerBounds)
{
  DICOMGDCMTagScanner::Pointer filescanner = DICOMGDCMTagScanner::New();
  filescanner->SetInputFiles(filenamesOfTimeStep);
  filescanner->AddTag(AcquisitionDateTag);
  filescanner->AddTag(AcquisitionTimeTag);
  filescanner->AddTag(TriggerTimeTag);
  filescanner->Scan();

  const DICOMDatasetAccessingImageFrameList frameList = filescanner->GetFrameInfoList();

  bool result = false;
  bool firstAq = true;
  bool firstTr = true;

  triggerBounds = TimeBounds(0.0);

  for (auto pos = frameList.cbegin(); pos != frameList.cend(); ++pos)
  {
    const std::string aqDateStr = (*pos)->GetTagValueAsString(AcquisitionDateTag).value;
    const std::string aqTimeStr = (*pos)->GetTagValueAsString(AcquisitionTimeTag).value;
    const std::string triggerTimeStr = (*pos)->GetTagValueAsString(TriggerTimeTag).value;

    OFDateTime aqDateTime;
    const bool convertAqResult = ConvertDICOMDateTimeString(aqDateStr, aqTimeStr, aqDateTime);

    OFBool convertTriggerResult;
    mitk::ScalarType triggerTime = OFStandard::atof(triggerTimeStr.c_str(), &convertTriggerResult);

    if (convertAqResult)
    {
      if (firstAq)
      {
        bounds[0] = aqDateTime;
        bounds[1] = aqDateTime;
        firstAq = false;
      }
      else
      {
        bounds[0] = GetLowerDateTime(bounds[0], aqDateTime);
        bounds[1] = GetUpperDateTime(bounds[1], aqDateTime);
      }
      result = true;
    }

    if (convertTriggerResult)
    {
      if (firstTr)
      {
        triggerBounds[0] = triggerTime;
        triggerBounds[1] = triggerTime;
        firstTr = false;
      }
      else
      {
        triggerBounds[0] = std::min(triggerBounds[0], triggerTime);
        triggerBounds[1] = std::max(triggerBounds[1], triggerTime);
      }
      result = true;
    }
  }

  return result;
};

bool mitk::ITKDICOMSeriesReaderHelper::ExtractTimeBoundsOfTimeStep(
  const StringContainer& filenamesOfTimeStep, TimeBounds& bounds, const OFDateTime& baselineDateTime, bool& usedTriggerBounds )
{
  DateTimeBounds aqDTBounds;
  TimeBounds triggerBounds;

  bool result = ExtractDateTimeBoundsAndTriggerOfTimeStep(filenamesOfTimeStep, aqDTBounds, triggerBounds);

  mitk::ScalarType lowerBound = mitk::ComputeMiliSecDuration( baselineDateTime, aqDTBounds[0] );
  mitk::ScalarType upperBound = mitk::ComputeMiliSecDuration( baselineDateTime, aqDTBounds[1] );
  if ( lowerBound < mitk::eps || upperBound < mitk::eps || usedTriggerBounds)
  {
    lowerBound = triggerBounds[0];
    upperBound = triggerBounds[1];
    usedTriggerBounds = true;
  }
  else
  {
    usedTriggerBounds = false;
  }

  bounds[0] = lowerBound;
  bounds[1] = upperBound;

  return result;
};

mitk::ITKDICOMSeriesReaderHelper::TimeBoundsList
  mitk::ITKDICOMSeriesReaderHelper::ExtractTimeBoundsOfTimeSteps(
    const StringContainerList& filenamesOfTimeSteps )
{
  TimeBoundsList result;

  OFDateTime baseLine;

  // extract the timebounds
  DateTimeBounds baselineDateTimeBounds;
  TimeBounds triggerBounds;
  auto pos = filenamesOfTimeSteps.cbegin();
  ExtractDateTimeBoundsAndTriggerOfTimeStep(*pos, baselineDateTimeBounds, triggerBounds);
  baseLine = baselineDateTimeBounds[0];

  // timebounds for baseline is 0
  TimeBounds bounds( 0.0 );
  result.push_back( bounds );

  //start with not using trigger time. Will be changed by ExtractTimeBoundsOfTimeStep if needed.
  bool usedTriggerTime = false;

  // iterate over the remaining timesteps
  for ( ++pos;
        pos != filenamesOfTimeSteps.cend();
        ++pos )
  {
    TimeBounds bounds( 0.0 );
    TimeBounds dateTimeBounds;

    // extract the timebounds relative to the baseline
    if ( ExtractTimeBoundsOfTimeStep( *pos, dateTimeBounds, baseLine, usedTriggerTime) )
    {
      bounds[0] = dateTimeBounds[0];
      bounds[1] = dateTimeBounds[1];
    }

    result.push_back( bounds );
  }

  if (usedTriggerTime)
    MITK_DEBUG << "Used trigger time to extract time bounds of passed files";

  return result;
};

mitk::TimeGeometry::Pointer
  mitk::ITKDICOMSeriesReaderHelper::GenerateTimeGeometry( const BaseGeometry* templateGeometry,
                                                          const TimeBoundsList& boundsList )
{
  TimeGeometry::Pointer timeGeometry;

  double check = 0.0;
  const auto boundListSize = boundsList.size();
  for ( std::size_t pos = 0; pos < boundListSize; ++pos )
  {
    check += boundsList[pos][0];
    check += boundsList[pos][1];
  }

  if ( check < mitk::eps )
  { // if all bounds are zero we assume that the bounds could not be correctly determined
    // and as a fallback generate a time geometry in the old mitk style
    ProportionalTimeGeometry::Pointer newTimeGeometry = ProportionalTimeGeometry::New();
    newTimeGeometry->Initialize( templateGeometry, boundListSize );
    timeGeometry = newTimeGeometry.GetPointer();
  }
  else
  {
    ArbitraryTimeGeometry::Pointer newTimeGeometry = ArbitraryTimeGeometry::New();
    newTimeGeometry->ClearAllGeometries();
    newTimeGeometry->ReserveSpaceForGeometries( boundListSize );

    for ( std::size_t pos = 0; pos < boundListSize; ++pos )
    {
      TimeBounds bounds = boundsList[pos];
      if ( pos + 1 < boundListSize )
      { //Currently we do not explicitly support "gaps" in the time coverage
        //thus we set the max time bound of a time step to the min time bound
        //of its successor.
        bounds[1] = boundsList[pos + 1][0];
      }

      newTimeGeometry->AppendNewTimeStepClone(templateGeometry, bounds[0], bounds[1]);
    }
    timeGeometry = newTimeGeometry.GetPointer();
  }

  return timeGeometry;
};
