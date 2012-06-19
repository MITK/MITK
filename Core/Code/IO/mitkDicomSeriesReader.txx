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

#include <mitkDicomSeriesReader.h>

#include <itkImageSeriesReader.h>
#include <mitkProperties.h>

#include <itkResampleImageFilter.h>
#include <itkAffineTransform.h>
#include <itkLinearInterpolateImageFunction.h>

#include <limits>

namespace mitk
{

template <typename PixelType>
void DicomSeriesReader::LoadDicom(const StringContainer &filenames, DataNode &node, bool sort, bool load4D, bool correctTilt, UpdateCallBackMethod callback)
{
  const char* previousCLocale = setlocale(LC_NUMERIC, NULL);
  setlocale(LC_NUMERIC, "C");
  std::locale previousCppLocale( std::cin.getloc() );
  std::locale l( "C" );
  std::cin.imbue(l);
  
  const gdcm::Tag tagImagePositionPatient(0x0020,0x0032); // Image Position (Patient)
  const gdcm::Tag    tagImageOrientation(0x0020, 0x0037); // Image Orientation

  try
  {
    mitk::Image::Pointer image = mitk::Image::New();
    CallbackCommand *command = callback ? new CallbackCommand(callback) : 0;
    bool initialize_node = false;

    /* special case for Philips 3D+t ultrasound images */ 
    if ( DicomSeriesReader::IsPhilips3DDicom(filenames.front().c_str())  )
    {
      ReadPhilips3DDicom(filenames.front().c_str(), image);
      initialize_node = true;
    }
    else
    {
      /* default case: assume "normal" image blocks, possibly 3D+t */
      bool canLoadAs4D(true);
      gdcm::Scanner scanner;
      ScanForSliceInformation(filenames, scanner);
   
      // need non-const access for map
      gdcm::Scanner::MappingType& tagValueMappings = const_cast<gdcm::Scanner::MappingType&>(scanner.GetMappings());
      
      std::list<StringContainer> imageBlocks = SortIntoBlocksFor3DplusT( filenames, tagValueMappings, sort, canLoadAs4D );
      unsigned int volume_count = imageBlocks.size();

      GantryTiltInformation tiltInfo;
 
      // check possibility of a single slice with many timesteps. In this case, don't check for tilt, no second slice possible
      if ( imageBlocks.front().size() > 1 && correctTilt)
      {
        // check tiltedness here, potentially fixup ITK's loading result by shifting slice contents
        // check first and last position slice from tags, make some calculations to detect tilt

        std::string firstFilename(imageBlocks.front().front()); 
        // calculate from first and last slice to minimize rounding errors
        std::string secondFilename(imageBlocks.front().back());

        std::string imagePosition1(    ConstCharStarToString( tagValueMappings[ firstFilename.c_str() ][ tagImagePositionPatient ] ) );
        std::string imageOrientation( ConstCharStarToString( tagValueMappings[ firstFilename.c_str() ][ tagImageOrientation ] ) );
        std::string imagePosition2(    ConstCharStarToString( tagValueMappings[secondFilename.c_str() ][ tagImagePositionPatient ] ) );
      
        bool ignoredConversionError(-42); // hard to get here, no graceful way to react
        Point3D origin1( DICOMStringToPoint3D( imagePosition1, ignoredConversionError ) );
        Point3D origin2( DICOMStringToPoint3D( imagePosition2, ignoredConversionError ) );
      
        Vector3D right; right.Fill(0.0);
        Vector3D up; right.Fill(0.0); // might be down as well, but it is just a name at this point
        DICOMStringToOrientationVectors( imageOrientation, right, up, ignoredConversionError );

        tiltInfo = GantryTiltInformation ( origin1, origin2, right, up, filenames.size()-1 );
        correctTilt = tiltInfo.IsSheared() && tiltInfo.IsRegularGantryTilt();

        MITK_DEBUG << "** Loading now: shear? " << tiltInfo.IsSheared();
        MITK_DEBUG << "** Loading now: normal tilt? " << tiltInfo.IsRegularGantryTilt();
        MITK_DEBUG << "** Loading now: perform tilt correction? " << correctTilt;
      }
      else
      {
        correctTilt = false; // we CANNOT do that
      }

      if (volume_count == 1 || !canLoadAs4D || !load4D)
      {
        image = LoadDICOMByITK<PixelType>( imageBlocks.front(), correctTilt, tiltInfo, command ); // load first 3D block
        initialize_node = true;
      }
      else if (volume_count > 1)
      {
        // It is 3D+t! Read it and store into mitk image
        typedef itk::Image<PixelType, 4> ImageType;
        typedef itk::ImageSeriesReader<ImageType> ReaderType;

        DcmIoType::Pointer io = DcmIoType::New();
        typename ReaderType::Pointer reader = ReaderType::New();

        reader->SetImageIO(io);
        reader->ReverseOrderOff();

        if (command)
        {
          reader->AddObserver(itk::ProgressEvent(), command);
        }

        unsigned int act_volume = 1u;

        reader->SetFileNames(imageBlocks.front());
        reader->Update();

        typename ImageType::Pointer readVolume = reader->GetOutput();
        // if we detected that the images are from a tilted gantry acquisition, we need to push some pixels into the right position
        if (correctTilt)
        {
          readVolume = InPlaceFixUpTiltedGeometry( reader->GetOutput(), tiltInfo );
        }

        image->InitializeByItk( readVolume.GetPointer(), 1, volume_count);
        image->SetImportVolume( readVolume->GetBufferPointer(), 0u);

        gdcm::Scanner scanner;
        ScanForSliceInformation(filenames, scanner);
        
        DicomSeriesReader::CopyMetaDataToImageProperties( imageBlocks, scanner.GetMappings(), io, image);

        MITK_DEBUG << "Volume dimension: [" << image->GetDimension(0) << ", " 
                                            << image->GetDimension(1) << ", " 
                                            << image->GetDimension(2) << ", " 
                                            << image->GetDimension(3) << "]";

#if (GDCM_MAJOR_VERSION == 2) && (GDCM_MINOR_VERSION < 1) && (GDCM_BUILD_VERSION < 15)
        // workaround for a GDCM 2 bug until version 2.0.15:
        // GDCM read spacing vector wrongly. Instead of "row spacing, column spacing", it misinterprets the DICOM tag as "column spacing, row spacing".
        // this is undone here, until we use a GDCM that has this issue fixed.
        // From the commit comments, GDCM 2.0.15 fixed the spacing interpretation with bug 2901181
        // http://sourceforge.net/tracker/index.php?func=detail&aid=2901181&group_id=137895&atid=739587


        Vector3D correctedImageSpacing = image->GetGeometry()->GetSpacing();
        std::swap( correctedImageSpacing[0], correctedImageSpacing[1] );
        image->GetGeometry()->SetSpacing( correctedImageSpacing );
#endif

        MITK_DEBUG << "Volume spacing: [" << image->GetGeometry()->GetSpacing()[0] << ", " 
                                          << image->GetGeometry()->GetSpacing()[1] << ", " 
                                          << image->GetGeometry()->GetSpacing()[2] << "]";

        for (std::list<StringContainer>::iterator df_it = ++imageBlocks.begin(); df_it != imageBlocks.end(); ++df_it)
        {
          reader->SetFileNames(*df_it);
          reader->Update();
          readVolume = reader->GetOutput();
          
          if (correctTilt)
          {
            readVolume = InPlaceFixUpTiltedGeometry( reader->GetOutput(), tiltInfo );
          }

          image->SetImportVolume(readVolume->GetBufferPointer(), act_volume++);
        }
        
        initialize_node = true;
      }
   
    }

    if (initialize_node)
    {
      // forward some image properties to node
      node.GetPropertyList()->ConcatenatePropertyList( image->GetPropertyList(), true );

      node.SetData( image );
      setlocale(LC_NUMERIC, previousCLocale);
      std::cin.imbue(previousCppLocale);
    }
  }
  catch (std::exception& e)
  {
    // reset locale then throw up
    setlocale(LC_NUMERIC, previousCLocale);
    std::cin.imbue(previousCppLocale);

    throw e;
  }
}

template <typename PixelType>
Image::Pointer DicomSeriesReader::LoadDICOMByITK( const StringContainer& filenames, bool correctTilt, const GantryTiltInformation& tiltInfo, CallbackCommand* command )
{
  /******** Normal Case, 3D (also for GDCM < 2 usable) ***************/
  mitk::Image::Pointer image = mitk::Image::New();

  typedef itk::Image<PixelType, 3> ImageType;
  typedef itk::ImageSeriesReader<ImageType> ReaderType;

  DcmIoType::Pointer io = DcmIoType::New();
  typename ReaderType::Pointer reader = ReaderType::New();

  reader->SetImageIO(io);
  reader->ReverseOrderOff();

  if (command)
  {
    reader->AddObserver(itk::ProgressEvent(), command);
  }

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

  gdcm::Scanner scanner;
  ScanForSliceInformation(filenames, scanner);
  
  DicomSeriesReader::CopyMetaDataToImageProperties( filenames, scanner.GetMappings(), io, image);

  MITK_DEBUG << "Volume dimension: [" << image->GetDimension(0) << ", " 
                                      << image->GetDimension(1) << ", " 
                                      << image->GetDimension(2) << "]";

#if (GDCM_MAJOR_VERSION == 2) && (GDCM_MINOR_VERSION < 1) && (GDCM_BUILD_VERSION < 15)
    // workaround for a GDCM 2 bug until version 2.0.15:
    // GDCM read spacing vector wrongly. Instead of "row spacing, column spacing", it misinterprets the DICOM tag as "column spacing, row spacing".
    // this is undone here, until we use a GDCM that has this issue fixed.
    // From the commit comments, GDCM 2.0.15 fixed the spacing interpretation with bug 2901181
    // http://sourceforge.net/tracker/index.php?func=detail&aid=2901181&group_id=137895&atid=739587


    Vector3D correctedImageSpacing = image->GetGeometry()->GetSpacing();
    std::swap( correctedImageSpacing[0], correctedImageSpacing[1] );
    image->GetGeometry()->SetSpacing( correctedImageSpacing );
#endif

  MITK_DEBUG << "Volume spacing: [" << image->GetGeometry()->GetSpacing()[0] << ", " 
                                    << image->GetGeometry()->GetSpacing()[1] << ", " 
                                    << image->GetGeometry()->GetSpacing()[2] << "]";

  return image;
}

void 
DicomSeriesReader::ScanForSliceInformation(const StringContainer &filenames, gdcm::Scanner& scanner)
{
  const gdcm::Tag ippTag(0x0020,0x0032); //Image position (Patient)
  scanner.AddTag(ippTag);
  
  const gdcm::Tag tagImageOrientation(0x0020,0x0037); //Image orientation
  scanner.AddTag(tagImageOrientation);

  // TODO what if tags don't exist?
  const gdcm::Tag tagSliceLocation(0x0020, 0x1041); // slice location
  scanner.AddTag( tagSliceLocation );

  const gdcm::Tag tagInstanceNumber(0x0020, 0x0013); // (image) instance number
  scanner.AddTag( tagInstanceNumber );

  const gdcm::Tag tagSOPInstanceNumber(0x0008, 0x0018); // SOP instance number
  scanner.AddTag( tagSOPInstanceNumber );

  scanner.Scan(filenames); // make available image position for each file
}

std::list<DicomSeriesReader::StringContainer> 
DicomSeriesReader::SortIntoBlocksFor3DplusT( 
    const StringContainer& presortedFilenames, 
    const gdcm::Scanner::MappingType& tagValueMappings,
    bool /*sort*/, 
    bool& canLoadAs4D )
{
  std::list<StringContainer> imageBlocks;

  // ignore sort request, because most likely re-sorting is now needed due to changes in GetSeries(bug #8022)
  StringContainer sorted_filenames = DicomSeriesReader::SortSeriesSlices(presortedFilenames);
 
  std::string firstPosition;
  unsigned int numberOfBlocks(0); // number of 3D image blocks

  const gdcm::Tag ippTag(0x0020,0x0032); //Image position (Patient)

  // loop files to determine number of image blocks
  for (StringContainer::const_iterator fileIter = sorted_filenames.begin();
       fileIter != sorted_filenames.end();
       ++fileIter)
  {
    gdcm::Scanner::TagToValue tagToValueMap = tagValueMappings.find( fileIter->c_str() )->second;
    
    if(tagToValueMap.find(ippTag) == tagToValueMap.end())
    {
      continue;
    }
    
    std::string position = tagToValueMap.find(ippTag)->second;
    MITK_DEBUG << "  " << *fileIter << " at " << position;
    if (firstPosition.empty())
    {
      firstPosition = position;
    }

    if ( position == firstPosition )
    {
      ++numberOfBlocks;
    }
    else
    {
      break; // enough information to know the number of image blocks
    }
  }

  MITK_DEBUG << "  ==> Assuming " << numberOfBlocks << " time steps";

  if (numberOfBlocks == 0) return imageBlocks; // only possible if called with no files


  // loop files to sort them into image blocks
  unsigned int numberOfExpectedSlices(0);
  for (unsigned int block = 0; block < numberOfBlocks; ++block)
  {
    StringContainer filesOfCurrentBlock;

    for ( StringContainer::const_iterator fileIter = sorted_filenames.begin() + block;
         fileIter != sorted_filenames.end();
         //fileIter += numberOfBlocks) // TODO shouldn't this work? give invalid iterators on first attempts
         )
    {
      filesOfCurrentBlock.push_back( *fileIter );
      for (unsigned int b = 0; b < numberOfBlocks; ++b)
      {
        if (fileIter != sorted_filenames.end())
          ++fileIter;
      }
    }

    imageBlocks.push_back(filesOfCurrentBlock);
    
    if (block == 0) 
    {
      numberOfExpectedSlices = filesOfCurrentBlock.size();
    }
    else
    {
      if (filesOfCurrentBlock.size() != numberOfExpectedSlices)
      {
        MITK_WARN << "DicomSeriesReader expected " << numberOfBlocks
                  << " image blocks of "
                  << numberOfExpectedSlices
                  << " images each. Block "
                  << block
                  << " got "
                  << filesOfCurrentBlock.size()
                  << " instead. Cannot load this as 3D+t"; // TODO implement recovery (load as many slices 3D+t as much as possible)
        canLoadAs4D = false;
      }
    }
  }

  return imageBlocks;
}

template <typename ImageType>
typename ImageType::Pointer
DicomSeriesReader::InPlaceFixUpTiltedGeometry( ImageType* input, const GantryTiltInformation& tiltInfo )
{
  typedef itk::ResampleImageFilter<ImageType,ImageType> ResampleFilterType;
  typename ResampleFilterType::Pointer resampler = ResampleFilterType::New();
  resampler->SetInput( input );

  /*
     transform for a point is
      - transform from actual position to index coordinates
      - apply a shear that undoes the gantry tilt
      - transform back into world coordinates

     anybody who does this in a simpler way: don't forget to write up how and why your solution works
  */
  typedef itk::AffineTransform< double, ImageType::ImageDimension > TransformType;
  typename TransformType::Pointer transformShear = TransformType::New();

  // row 1, column 2 corrects shear in parallel to Y axis, proportional to distance in Z direction
  transformShear->Shear( 1, 2, tiltInfo.GetMatrixCoefficientForCorrection() ); 
 
  typename TransformType::Pointer imageIndexToWorld = TransformType::New();
  imageIndexToWorld->SetOffset( input->GetOrigin().GetVectorFromOrigin() );
  imageIndexToWorld->SetMatrix( input->GetDirection() );
  
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
  resampler->SetDefaultPixelValue( std::numeric_limits<typename ImageType::PixelType>::min() ); // TODO minimum meaningful modality value?
  
  // adjust size in Y direction! (maybe just transform the outer last pixel to see how much space we would need

  resampler->SetOutputParametersFromImage( input ); // we basically need the same image again, just sheared

  typename ImageType::SizeType largerSize = resampler->GetSize(); // now the resampler already holds the input image's size.
  largerSize[1] += tiltInfo.GetTiltCorrectedAdditionalSize();
  resampler->SetSize( largerSize );

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
