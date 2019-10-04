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

#ifndef MITKDICOMSERIESREADER_TXX_
#define MITKDICOMSERIESREADER_TXX_

#include <string>
#include <vector>
#include <iostream>
#include <limits>
#include <map>

#include <boost/filesystem.hpp>

#include <itkImageSeriesReader.h>

#include <itkResampleImageFilter.h>
#include <itkAffineTransform.h>
#include <itkLinearInterpolateImageFunction.h>
#include <itkTimeProbesCollectorBase.h>
#include <itkMetaDataDictionary.h>

#include <itkImage.h>
#include <itkMetaDataObject.h>
#include <itkImageSeriesWriter.h>
#include <itkNumericSeriesFileNames.h>

#include <mitkImage.h>
#include <mitkProperties.h>
#include <mitkDicomSeriesReader.h>
#include <mitkLookupTables.h>
#include <mitkLookupTableProperty.h>

namespace mitk
{
typedef std::map<std::string, StringLookupTable> DicomTagToValueList;

inline void AddMetaDataToDictionary(itk::MetaDataDictionary& fromDict, DicomTagToValueList& list)
{
  typedef itk::MetaDataDictionary DictionaryType;
  typedef itk::MetaDataObject<std::string> MetaDataStringType;

  DictionaryType::ConstIterator itr = fromDict.Begin();
  DictionaryType::ConstIterator end = fromDict.End();

  StringLookupTable valueList;
  while (itr != end)
  {
    itk::MetaDataObjectBase::Pointer entry = itr->second;

    MetaDataStringType::Pointer entryvalue = dynamic_cast<MetaDataStringType*>(entry.GetPointer());
    if (entryvalue)
    {
      std::string tagkey = itr->first;
      std::string tagvalue = entryvalue->GetMetaDataObjectValue();

      list[tagkey].SetTableValue(list[tagkey].GetLookupTable().size(), tagvalue);
    }

    ++itr;
  }
}

template <typename ReaderType>
void CopyDictionaryFromReader(ReaderType* reader, DicomTagToValueList& list)
{
  if (reader == nullptr)
  {
    return;
  }

  // Get MetaDataDictionary and copy him to MITK Image
  mitk::ReaderType::DictionaryArrayRawPointer inputDict = reader->GetMetaDataDictionaryArray();
  mitk::ReaderType::DictionaryArrayType::const_iterator iter = inputDict->begin();
  for (; iter != inputDict->end(); ++iter)
  {
    AddMetaDataToDictionary(*(*(iter)), list);
  }
}

template <typename PixelType>
Image::Pointer DicomSeriesReader::LoadDICOMByITK4D( std::list<StringContainer>& imageBlocks, ImageBlockDescriptor imageBlockDescriptor, bool correctTilt, const GantryTiltInformation& tiltInfo, DcmIoType::Pointer& io, CallbackCommand* command, Image::Pointer preLoadedImageBlock )
{
  mitk::Image::Pointer image = mitk::Image::New();

  // It is 3D+t! Read it and store into mitk image
  typedef itk::Image<PixelType, 4> ImageType;
  typedef itk::ImageSeriesReader<ImageType> ReaderType;

  typename ReaderType::Pointer reader = ReaderType::New();

  reader->SetImageIO(io);
  reader->ReverseOrderOff();

  if (command)
  {
    reader->AddObserver(itk::ProgressEvent(), command);
  }

  if (preLoadedImageBlock.IsNull())
  {
    reader->SetFileNames(imageBlocks.front());

    reader->Update();

    typename ImageType::Pointer readVolume = reader->GetOutput();
    // if we detected that the images are from a tilted gantry acquisition, we need to push some pixels into the right position
    if (correctTilt)
    {
      readVolume = InPlaceFixUpTiltedGeometry( reader->GetOutput(), tiltInfo );
    }

    unsigned int volume_count = imageBlocks.size();
    image->InitializeByItk( readVolume.GetPointer(), 1, volume_count);
    image->SetImportVolume( readVolume->GetBufferPointer(), 0u);

    FixSpacingInformation( image, imageBlockDescriptor );
  }
  else
  {
    StringContainer fakeList;
    fakeList.push_back( imageBlocks.front().front() );
    reader->SetFileNames(fakeList); // only ONE first filename to get MetaDataDictionary

    image = preLoadedImageBlock;
  }

  assert (!imageBlockDescriptor.GetSlicesInfo().empty());
  CopyMetaDataToImageProperties( imageBlocks, io, imageBlockDescriptor, image);

  MITK_DEBUG << "Volume dimension: [" << image->GetDimension(0) << ", "
    << image->GetDimension(1) << ", "
    << image->GetDimension(2) << ", "
    << image->GetDimension(3) << "]";

  MITK_DEBUG << "Volume spacing: [" << image->GetGeometry()->GetSpacing()[0] << ", "
    << image->GetGeometry()->GetSpacing()[1] << ", "
    << image->GetGeometry()->GetSpacing()[2] << "]";

  // Get MetaDataDictionary and copy him to MITK Image
  DicomTagToValueList list;
  CopyDictionaryFromReader<ReaderType>(reader, list);

  if ( preLoadedImageBlock.IsNull() )
  {
    unsigned int act_volume = 1u;
    for (auto df_it = ++imageBlocks.begin(); df_it != imageBlocks.end(); ++df_it)
    {
      reader->SetFileNames(*df_it);
      reader->Update();
      typename ImageType::Pointer readVolume = reader->GetOutput();

      if (correctTilt)
      {
        readVolume = InPlaceFixUpTiltedGeometry( reader->GetOutput(), tiltInfo );
      }

      image->SetImportVolume(readVolume->GetBufferPointer(), act_volume++);
      CopyDictionaryFromReader<ReaderType>(reader, list);
    }
  }

  image->SetMetaDataDictionary(list);

  return image;
}

// TODO This function is for debugging purposes. It allows you to save itkImage in the DICOM file set.
// dumpFilePath - The path to which the received DICOM files.
template <typename ImageType>
void DicomSeriesReader::dumpITKImageToDICOM(ImageType* itkImage, const std::vector<itk::MetaDataDictionary*>& dictionary,
  const std::string dumpFilePath)
{
  const unsigned int InputDimension = 3;
  const unsigned int OutputDimension = 2;

  typedef itk::Image<typename ImageType::PixelType, InputDimension> InputImageType;
  typedef itk::Image<typename ImageType::PixelType, OutputDimension> OutputImageType;
  typedef itk::NumericSeriesFileNames OutputNamesGeneratorType;

  typedef itk::ImageSeriesWriter<InputImageType, OutputImageType> SeriesWriterType;

  typename SeriesWriterType::Pointer seriesWriter = SeriesWriterType::New();

  seriesWriter->SetInput(itkImage);

  auto dcmIO = DcmIoType::New();
  seriesWriter->SetImageIO(dcmIO);

  boost::filesystem::path currentPath = dumpFilePath;
  boost::filesystem::create_directories(currentPath);

  OutputNamesGeneratorType::Pointer outputNames = OutputNamesGeneratorType::New();
  std::string seriesFormat(currentPath.string());
  seriesFormat = seriesFormat + "IM%06d.dcm";
  outputNames->SetSeriesFormat (seriesFormat.c_str());
  outputNames->SetStartIndex (1);
  const int array_size = dictionary.size();
  outputNames->SetEndIndex(array_size);
  seriesWriter->SetFileNames(outputNames->GetFileNames());

  seriesWriter->SetMetaDataDictionaryArray(&dictionary);

  try
  {
    seriesWriter->Update();
  }
  catch(itk::ExceptionObject & excp)
  {
    std::cerr << "Exception thrown while writing the series " << std::endl;
    std::cerr << excp << std::endl;
    return;
  }
}

template <typename PixelType>
Image::Pointer DicomSeriesReader::LoadDICOMByITK( const StringContainer& filenames, bool correctTilt, const GantryTiltInformation& tiltInfo, DcmIoType::Pointer& io, CallbackCommand* command, Image::Pointer preLoadedImageBlock )
{
  /******** Normal Case, 3D (also for GDCM < 2 usable) ***************/
  mitk::Image::Pointer image = mitk::Image::New();

  typedef itk::Image<PixelType, 3> ImageType;
  typedef itk::ImageSeriesReader<ImageType> ReaderType;

  typename ReaderType::Pointer reader = ReaderType::New();

  if (!io) {
  }
  io = itk::GDCMImageIO::New();
  reader->SetImageIO(io);
  reader->ReverseOrderOff();

  if (command)
  {
    reader->AddObserver(itk::ProgressEvent(), command);
  }

  if (preLoadedImageBlock.IsNull())
  {
    reader->SetFileNames(filenames);
    reader->Update();

    typename ImageType::Pointer readVolume = reader->GetOutput();

    // if we detected that the images are from a tilted gantry acquisition, we need to push some pixels into the right position
    if (correctTilt)
    {
      readVolume = InPlaceFixUpTiltedGeometry( reader->GetOutput(), tiltInfo );
    }

    image->InitializeByItk(readVolume.GetPointer());
    image->SetImportVolume(readVolume->GetBufferPointer());
  }
  else
  {
    image = preLoadedImageBlock;
    StringContainer fakeList;
    fakeList.push_back( filenames.front() );
    reader->SetFileNames( fakeList ); // we always need to load at least one file to get the MetaDataDictionary
    reader->Update();
  }

  MITK_DEBUG << "Volume dimension: [" << image->GetDimension(0) << ", "
                                      << image->GetDimension(1) << ", "
                                      << image->GetDimension(2) << "]";

  MITK_DEBUG << "Volume spacing: [" << image->GetGeometry()->GetSpacing()[0] << ", "
                                    << image->GetGeometry()->GetSpacing()[1] << ", "
                                    << image->GetGeometry()->GetSpacing()[2] << "]";

  // Get MetaDataDictionary and copy him to MITK Image
  DicomTagToValueList list;
  CopyDictionaryFromReader<ReaderType>(reader, list);

  image->SetMetaDataDictionary(list);

  return image;
}

template <typename ImageType>
typename ImageType::Pointer
DicomSeriesReader::InPlaceFixUpTiltedGeometry( ImageType* input, const GantryTiltInformation& tiltInfo )
{
  typedef itk::ResampleImageFilter<ImageType,ImageType> ResampleFilterType;
  typename ResampleFilterType::Pointer resampler = ResampleFilterType::New();
  resampler->SetInput( input );

  /*
     Transform for a point is
      - transform from actual position to index coordinates
      - apply a shear that undoes the gantry tilt
      - transform back into world coordinates

     Anybody who does this in a simpler way: don't forget to write up how and why your solution works
  */
  typedef itk::ScalableAffineTransform< double, ImageType::ImageDimension > TransformType;
  typename TransformType::Pointer transformShear = TransformType::New();

  /**
    - apply a shear and spacing correction to the image block that corrects the ITK reader's error
      - ITK ignores the shear and loads slices into an orthogonal volume
      - ITK calculates the spacing from the origin distance, which is more than the actual spacing with gantry tilt images
    - to undo the effect
      - we have calculated some information in tiltInfo:
        - the shift in Y direction that is added with each additional slice is the most important information
        - the Y-shift is calculated in mm world coordinates
      - we apply a shearing transformation to the ITK-read image volume
        - to do this locally,
          - we transform the image volume back to origin and "normal" orientation by applying the inverse of its transform
            (this brings us into the image's "index coordinate" system)
          - we apply a shear with the Y-shift factor put into a unit transform at row 1, col 2
          - we transform the image volume back to its actual position (from index to world coordinates)
      - we lastly apply modify the image spacing in z direction by replacing this number with the correctly calulcated inter-slice distance
  */

  ScalarType factor = tiltInfo.GetMatrixCoefficientForCorrectionInWorldCoordinates() / input->GetSpacing()[1];
  // row 1, column 2 corrects shear in parallel to Y axis, proportional to distance in Z direction
  transformShear->Shear( 1, 2, factor );

  typename TransformType::Pointer imageIndexToWorld = TransformType::New();
  imageIndexToWorld->SetOffset( input->GetOrigin().GetVectorFromOrigin() );

  typename TransformType::MatrixType indexToWorldMatrix;
  indexToWorldMatrix = input->GetDirection();

  typename ImageType::DirectionType scale;
  for ( unsigned int i = 0; i < ImageType::ImageDimension; i++ )
  {
    scale[i][i] = input->GetSpacing()[i];
  }
  indexToWorldMatrix *= scale;

  imageIndexToWorld->SetMatrix( indexToWorldMatrix );

  typename TransformType::Pointer imageWorldToIndex = TransformType::New();
  imageIndexToWorld->GetInverse( imageWorldToIndex );

  typename TransformType::Pointer gantryTiltCorrection = TransformType::New();
  gantryTiltCorrection->Compose( imageWorldToIndex );
  gantryTiltCorrection->Compose( transformShear );
  gantryTiltCorrection->Compose( imageIndexToWorld );

  resampler->SetTransform( gantryTiltCorrection );

  typedef itk::LinearInterpolateImageFunction< ImageType, double > InterpolatorType;
  typename InterpolatorType::Pointer interpolator = InterpolatorType::New();
  resampler->SetInterpolator( interpolator );
  /*
     This would be the right place to invent a meaningful value for positions outside of the image.
     For CT, HU -1000 might be meaningful, but a general solution seems not possible. Even for CT,
     -1000 would only look natural for many not all images.
  */

  // TODO use (0028,0120) Pixel Padding Value if present
  resampler->SetDefaultPixelValue( itk::NumericTraits< typename ImageType::PixelType >::min() );

  // adjust size in Y direction! (maybe just transform the outer last pixel to see how much space we would need

  resampler->SetOutputParametersFromImage( input ); // we basically need the same image again, just sheared


  // if tilt positive, then we need additional pixels BELOW origin, otherwise we need pixels behind the end of the block

  // in any case we need more size to accomodate shifted slices
  typename ImageType::SizeType largerSize = resampler->GetSize(); // now the resampler already holds the input image's size.
  largerSize[1] += static_cast<typename ImageType::SizeType::SizeValueType>(tiltInfo.GetTiltCorrectedAdditionalSize() / input->GetSpacing()[1]+ 2.0);
  resampler->SetSize( largerSize );

  // in SOME cases this additional size is below/behind origin
  if ( tiltInfo.GetMatrixCoefficientForCorrectionInWorldCoordinates() > 0.0 )
  {
    typename ImageType::DirectionType imageDirection = input->GetDirection();
    Vector3D yDirection;
    yDirection[0] = imageDirection[0][1];
    yDirection[1] = imageDirection[1][1];
    yDirection[2] = imageDirection[2][1];
    yDirection.Normalize();

    typename ImageType::PointType shiftedOrigin;
    shiftedOrigin = input->GetOrigin();

    // add some pixels to make everything fit
    shiftedOrigin[0] -= yDirection[0] * (tiltInfo.GetTiltCorrectedAdditionalSize() + 1.0 * input->GetSpacing()[1]);
    shiftedOrigin[1] -= yDirection[1] * (tiltInfo.GetTiltCorrectedAdditionalSize() + 1.0 * input->GetSpacing()[1]);
    shiftedOrigin[2] -= yDirection[2] * (tiltInfo.GetTiltCorrectedAdditionalSize() + 1.0 * input->GetSpacing()[1]);

    resampler->SetOutputOrigin( shiftedOrigin );
  }

  resampler->Update();
  typename ImageType::Pointer result = resampler->GetOutput();

  // ImageSeriesReader calculates z spacing as the distance between the first two origins.
  // This is not correct in case of gantry tilt, so we set our calculated spacing.
  typename ImageType::SpacingType correctedSpacing = result->GetSpacing();
  correctedSpacing[2] = tiltInfo.GetRealZSpacing();
  result->SetSpacing( correctedSpacing );

  return result;
}

}

#endif
