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

//#define MBILOG_ENABLE_DEBUG

#include <dcvrdt.h>

#define BOOST_DATE_TIME_NO_LIB
//Prevent unnecessary/unwanted auto link in this compilation when activating boost libraries in the MITK superbuild
//It is necessary because BOOST_ALL_DYN_LINK overwrites BOOST_DATE_TIME_NO_LIB
#if defined(BOOST_ALL_DYN_LINK)
  #undef BOOST_ALL_DYN_LINK
#endif

#include <boost/date_time/posix_time/posix_time_types.hpp>

#include "mitkITKDICOMSeriesReaderHelper.h"
#include "mitkITKDICOMSeriesReaderHelper.txx"

#include "mitkDICOMGDCMTagScanner.h"
#include "mitkArbitraryTimeGeometry.h"

#define switch3DCase( IOType, T ) \
  case IOType:                    \
    return LoadDICOMByITK<T>( filenames, correctTilt, tiltInfo, io );

bool mitk::ITKDICOMSeriesReaderHelper::CanHandleFile( const std::string& filename )
{
  MITK_DEBUG << "ITKDICOMSeriesReaderHelper::CanHandleFile " << filename;
  itk::GDCMImageIO::Pointer tester = itk::GDCMImageIO::New();
  return tester->CanReadFile( filename.c_str() );
}

mitk::Image::Pointer mitk::ITKDICOMSeriesReaderHelper::Load( const StringContainer& filenames,
                                                             bool correctTilt,
                                                             const GantryTiltInformation& tiltInfo )
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

      if ( io->GetPixelType() == itk::ImageIOBase::SCALAR )
      {
        switch ( io->GetComponentType() )
        {
          switch3DCase(DcmIoType::UCHAR, unsigned char) switch3DCase(DcmIoType::CHAR, char) switch3DCase(
            DcmIoType::USHORT, unsigned short) switch3DCase(DcmIoType::SHORT, short)
              switch3DCase(DcmIoType::UINT, unsigned int) switch3DCase(DcmIoType::INT, int) switch3DCase(
                DcmIoType::ULONG, long unsigned int) switch3DCase(DcmIoType::LONG, long int)
                switch3DCase(DcmIoType::FLOAT, float) switch3DCase(DcmIoType::DOUBLE, double) default
            : MITK_ERROR
              << "Found unsupported DICOM scalar pixel type: (enum value) "
              << io->GetComponentType();
        }
      }
      else if ( io->GetPixelType() == itk::ImageIOBase::RGB )
      {
        switch ( io->GetComponentType() )
        {
          switch3DCase(DcmIoType::UCHAR, itk::RGBPixel<unsigned char>) switch3DCase(
            DcmIoType::CHAR, itk::RGBPixel<char>) switch3DCase(DcmIoType::USHORT,
                                                               itk::RGBPixel<unsigned short>)
              switch3DCase(DcmIoType::SHORT, itk::RGBPixel<short>) switch3DCase(
                DcmIoType::UINT, itk::RGBPixel<unsigned int>) switch3DCase(DcmIoType::INT, itk::RGBPixel<int>)
                switch3DCase(DcmIoType::ULONG, itk::RGBPixel<long unsigned int>)
                  switch3DCase(DcmIoType::LONG, itk::RGBPixel<long int>) switch3DCase(
                    DcmIoType::FLOAT, itk::RGBPixel<float>) switch3DCase(DcmIoType::DOUBLE,
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

      if ( io->GetPixelType() == itk::ImageIOBase::SCALAR )
      {
        switch ( io->GetComponentType() )
        {
          switch3DnTCase(DcmIoType::UCHAR, unsigned char) switch3DnTCase(DcmIoType::CHAR, char)
              switch3DnTCase(DcmIoType::USHORT, unsigned short) switch3DnTCase(
                DcmIoType::SHORT, short) switch3DnTCase(DcmIoType::UINT,
                                                        unsigned int) switch3DnTCase(DcmIoType::INT, int)
                switch3DnTCase(DcmIoType::ULONG, long unsigned int) switch3DnTCase(DcmIoType::LONG, long int)
                  switch3DnTCase(DcmIoType::FLOAT, float) switch3DnTCase(DcmIoType::DOUBLE, double) default
            : MITK_ERROR
              << "Found unsupported DICOM scalar pixel type: (enum value) "
              << io->GetComponentType();
        }
      }
      else if ( io->GetPixelType() == itk::ImageIOBase::RGB )
      {
        switch ( io->GetComponentType() )
        {
          switch3DnTCase(DcmIoType::UCHAR, itk::RGBPixel<unsigned char>)
              switch3DnTCase(DcmIoType::CHAR, itk::RGBPixel<char>) switch3DnTCase(
                DcmIoType::USHORT, itk::RGBPixel<unsigned short>) switch3DnTCase(DcmIoType::SHORT,
                                                                                 itk::RGBPixel<short>)
                switch3DnTCase(DcmIoType::UINT, itk::RGBPixel<unsigned int>) switch3DnTCase(
                  DcmIoType::INT, itk::RGBPixel<int>) switch3DnTCase(DcmIoType::ULONG,
                                                                     itk::RGBPixel<long unsigned int>)
                  switch3DnTCase(DcmIoType::LONG, itk::RGBPixel<long int>) switch3DnTCase(
                    DcmIoType::FLOAT, itk::RGBPixel<float>) switch3DnTCase(DcmIoType::DOUBLE,
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

  const OFCondition result = DcmDateTime::getOFDateTimeFromString( content, time );

  return result.good();
}

boost::posix_time::ptime ConvertOFDateTimeToPTime( const OFDateTime& time )
{
  const boost::gregorian::date boostDate(
    time.getDate().getYear(), time.getDate().getMonth(), time.getDate().getDay() );

  const boost::posix_time::time_duration boostTime =
    boost::posix_time::hours( time.getTime().getHour() )
    + boost::posix_time::minutes( time.getTime().getMinute() )
    + boost::posix_time::seconds( time.getTime().getIntSecond() )
    + boost::posix_time::milliseconds( time.getTime().getMilliSecond() );

  boost::posix_time::ptime result( boostDate, boostTime );

  return result;
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

double ComputeMiliSecDuration( const OFDateTime& start, const OFDateTime& stop )
{
  const boost::posix_time::ptime startTime = ConvertOFDateTimeToPTime( start );
  const boost::posix_time::ptime stopTime  = ConvertOFDateTimeToPTime( stop );

  ::boost::posix_time::time_duration duration = stopTime - startTime;

  double result = duration.total_milliseconds();

  return result;
}

bool mitk::ITKDICOMSeriesReaderHelper::ExtractTimeBoundsOfTimeStep(
  const StringContainer& filenamesOfTimeStep, DateTimeBounds& bounds )
{
  const DICOMTag acquisitionDateTag( 0x0008, 0x0022 );
  const DICOMTag acquisitionTimeTag( 0x0008, 0x0032 );

  DICOMGDCMTagScanner::Pointer filescanner = DICOMGDCMTagScanner::New();
  filescanner->SetInputFiles( filenamesOfTimeStep );
  filescanner->AddTag( acquisitionDateTag );
  filescanner->AddTag( acquisitionTimeTag );
  filescanner->Scan();

  const DICOMGDCMImageFrameList frameList = filescanner->GetFrameInfoList();

  bool result = false;
  bool first  = true;

  for ( DICOMGDCMImageFrameList::const_iterator pos = frameList.cbegin(); pos != frameList.cend(); ++pos )
  {
    const std::string dateStr = ( *pos )->GetTagValueAsString( acquisitionDateTag ).value;
    const std::string timeStr = ( *pos )->GetTagValueAsString( acquisitionTimeTag ).value;

    OFDateTime time;
    const bool convertResult = ConvertDICOMDateTimeString( dateStr, timeStr, time );

    if ( convertResult )
    {
      if ( first )
      {
        bounds[0] = time;
        bounds[1] = time;
        first     = false;
      }
      else
      {
        bounds[0] = GetLowerDateTime( bounds[0], time );
        bounds[1] = GetUpperDateTime( bounds[1], time );
      }
      result = true;
    }
  }

  return result;
};

mitk::ITKDICOMSeriesReaderHelper::TimeBoundsList
  mitk::ITKDICOMSeriesReaderHelper::ExtractTimeBoundsOfTimeSteps(
    const StringContainerList& filenamesOfTimeSteps )
{
  TimeBoundsList result;

  OFDateTime baseLine;
  bool baseLineSet = false;

  for ( StringContainerList::const_iterator pos = filenamesOfTimeSteps.cbegin();
        pos != filenamesOfTimeSteps.cend();
        ++pos )
  {
    TimeBounds bounds( 0.0 );
    DateTimeBounds dateTimeBounds;

    if ( ExtractTimeBoundsOfTimeStep( *pos, dateTimeBounds ) )
    {
      if ( !baseLineSet )
      {
        baseLineSet = true;
        baseLine    = dateTimeBounds[0];
      }

      bounds[0] = ComputeMiliSecDuration( baseLine, dateTimeBounds[0] );
      bounds[1] = ComputeMiliSecDuration( baseLine, dateTimeBounds[1] );
    }

    result.push_back( bounds );
  }

  return result;
};

mitk::TimeGeometry::Pointer
  mitk::ITKDICOMSeriesReaderHelper::GenerateTimeGeometry( const BaseGeometry* templateGeometry,
                                                          const TimeBoundsList& boundsList )
{
  TimeGeometry::Pointer timeGeometry;

  double check = 0.0;
  const auto boundListSize = boundsList.size();
  for ( auto pos = 0; pos < boundListSize; ++pos )
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

    for ( auto pos = 0; pos < boundListSize; ++pos )
    {
      TimeBounds bounds = boundsList[pos];
      if ( pos + 1 < boundListSize )
      {
        bounds[1] = boundsList[pos + 1][0];
      }

      newTimeGeometry->AppendTimeStepClone( templateGeometry, bounds[1], bounds[0] );
    }
    timeGeometry = newTimeGeometry.GetPointer();
  }

  return timeGeometry;
};
