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

#include "mitkDICOMImageBlockDescriptor.h"
#include "mitkStringProperty.h"
#include "mitkLevelWindowProperty.h"
#include <gdcmUIDs.h>
#include <vector>

mitk::DICOMImageBlockDescriptor::DICOMImageBlockDescriptor()
: m_ReaderImplementationLevel( SOPClassUnknown )
, m_PropertyList( PropertyList::New() )
, m_TagCache( nullptr )
, m_PropertiesOutOfDate( true )
{
  m_PropertyFunctor = &mitk::DICOMImageBlockDescriptor::GetPropertyForDICOMValues;
}

mitk::DICOMImageBlockDescriptor::~DICOMImageBlockDescriptor()
{
}

mitk::DICOMImageBlockDescriptor::DICOMImageBlockDescriptor( const DICOMImageBlockDescriptor& other )
: m_ImageFrameList( other.m_ImageFrameList )
, m_MitkImage( other.m_MitkImage )
, m_SliceIsLoaded( other.m_SliceIsLoaded )
, m_ReaderImplementationLevel( other.m_ReaderImplementationLevel )
, m_TiltInformation( other.m_TiltInformation )
, m_PropertyList( other.m_PropertyList->Clone() )
, m_TagCache( other.m_TagCache )
, m_PropertiesOutOfDate( other.m_PropertiesOutOfDate )
, m_AdditionalTagMap(other.m_AdditionalTagMap)
, m_FoundAdditionalTags(other.m_FoundAdditionalTags)
, m_PropertyFunctor(other.m_PropertyFunctor)
{
  if ( m_MitkImage )
  {
    m_MitkImage = m_MitkImage->Clone();
  }

  m_PropertyFunctor = &mitk::DICOMImageBlockDescriptor::GetPropertyForDICOMValues;
}

mitk::DICOMImageBlockDescriptor& mitk::DICOMImageBlockDescriptor::
  operator=( const DICOMImageBlockDescriptor& other )
{
  if ( this != &other )
  {
    m_ImageFrameList            = other.m_ImageFrameList;
    m_MitkImage                 = other.m_MitkImage;
    m_SliceIsLoaded             = other.m_SliceIsLoaded;
    m_ReaderImplementationLevel = other.m_ReaderImplementationLevel;
    m_TiltInformation           = other.m_TiltInformation;

    m_AdditionalTagMap = other.m_AdditionalTagMap;
    m_FoundAdditionalTags = other.m_FoundAdditionalTags;
    m_PropertyFunctor = other.m_PropertyFunctor;

    if ( other.m_PropertyList )
    {
      m_PropertyList = other.m_PropertyList->Clone();
    }

    if ( other.m_MitkImage )
    {
      m_MitkImage = other.m_MitkImage->Clone();
    }

    m_TagCache            = other.m_TagCache;
    m_PropertiesOutOfDate = other.m_PropertiesOutOfDate;
  }
  return *this;
}

mitk::DICOMTagList mitk::DICOMImageBlockDescriptor::GetTagsOfInterest()
{
  DICOMTagList completeList;

  completeList.push_back( DICOMTag( 0x0018, 0x1164 ) ); // pixel spacing
  completeList.push_back( DICOMTag( 0x0028, 0x0030 ) ); // imager pixel spacing

  completeList.push_back( DICOMTag( 0x0008, 0x0018 ) ); // sop instance UID
  completeList.push_back( DICOMTag( 0x0008, 0x0016 ) ); // sop class UID

  completeList.push_back( DICOMTag( 0x0020, 0x0011 ) ); // series number
  completeList.push_back( DICOMTag( 0x0008, 0x1030 ) ); // study description
  completeList.push_back( DICOMTag( 0x0008, 0x103e ) ); // series description
  completeList.push_back( DICOMTag( 0x0008, 0x0060 ) ); // modality
  completeList.push_back( DICOMTag( 0x0018, 0x0024 ) ); // sequence name
  completeList.push_back( DICOMTag( 0x0020, 0x0037 ) ); // image orientation

  completeList.push_back( DICOMTag( 0x0020, 0x1041 ) ); // slice location
  completeList.push_back( DICOMTag( 0x0020, 0x0012 ) ); // acquisition number
  completeList.push_back( DICOMTag( 0x0020, 0x0013 ) ); // instance number
  completeList.push_back( DICOMTag( 0x0020, 0x0032 ) ); // image position patient

  completeList.push_back( DICOMTag( 0x0028, 0x1050 ) ); // window center
  completeList.push_back( DICOMTag( 0x0028, 0x1051 ) ); // window width
  completeList.push_back( DICOMTag( 0x0008, 0x0008 ) ); // image type
  completeList.push_back( DICOMTag( 0x0028, 0x0004 ) ); // photometric interpretation

  return completeList;
}


void mitk::DICOMImageBlockDescriptor::SetAdditionalTagsOfInterest(
  const AdditionalTagsMapType& tagMap)
{
  m_AdditionalTagMap = tagMap;
}


void mitk::DICOMImageBlockDescriptor::SetTiltInformation( const GantryTiltInformation& info )
{
  m_TiltInformation = info;
}

const mitk::GantryTiltInformation mitk::DICOMImageBlockDescriptor::GetTiltInformation() const
{
  return m_TiltInformation;
}

void mitk::DICOMImageBlockDescriptor::SetImageFrameList( const DICOMImageFrameList& framelist )
{
  m_ImageFrameList = framelist;
  m_SliceIsLoaded.resize( framelist.size() );
  m_SliceIsLoaded.assign( framelist.size(), false );

  m_PropertiesOutOfDate = true;
}

const mitk::DICOMImageFrameList& mitk::DICOMImageBlockDescriptor::GetImageFrameList() const
{
  return m_ImageFrameList;
}

void mitk::DICOMImageBlockDescriptor::SetMitkImage( Image::Pointer image )
{
  if ( m_MitkImage != image )
  {
    if ( m_TagCache.IsExpired() )
    {
      MITK_ERROR << "Unable to describe MITK image with properties without a tag-cache object!";
      m_MitkImage = nullptr;
      return;
    }

    if ( m_ImageFrameList.empty() )
    {
      MITK_ERROR << "Unable to describe MITK image with properties without a frame list!";
      m_MitkImage = nullptr;
      return;
    }

    // Should verify that the image matches m_ImageFrameList and m_TagCache
    // however, this is hard to do without re-analyzing all
    // TODO we should at least make sure that the number of frames is identical (plus rows/columns,
    // orientation)
    //      without gantry tilt correction, we can also check image origin

    m_MitkImage = this->DescribeImageWithProperties( this->FixupSpacing( image ) );
  }
}

mitk::Image::Pointer mitk::DICOMImageBlockDescriptor::GetMitkImage() const
{
  return m_MitkImage;
}

mitk::Image::Pointer mitk::DICOMImageBlockDescriptor::FixupSpacing( Image* mitkImage )
{
  if ( mitkImage )
  {
    Vector3D imageSpacing = mitkImage->GetGeometry()->GetSpacing();

    ScalarType desiredSpacingX = imageSpacing[0];
    ScalarType desiredSpacingY = imageSpacing[1];
    this->GetDesiredMITKImagePixelSpacing(
      desiredSpacingX, desiredSpacingY ); // prefer pixel spacing over imager pixel spacing

    if ( desiredSpacingX <= 0 || desiredSpacingY <= 0 )
    {
      return mitkImage;
    }

    MITK_DEBUG << "Loaded image with spacing " << imageSpacing[0] << ", " << imageSpacing[1];
    MITK_DEBUG << "Found correct spacing info " << desiredSpacingX << ", " << desiredSpacingY;

    imageSpacing[0] = desiredSpacingX;
    imageSpacing[1] = desiredSpacingY;
    mitkImage->GetGeometry()->SetSpacing( imageSpacing );
  }

  return mitkImage;
}
void mitk::DICOMImageBlockDescriptor::SetSliceIsLoaded( unsigned int index, bool isLoaded )
{
  if ( index < m_SliceIsLoaded.size() )
  {
    m_SliceIsLoaded[index] = isLoaded;
  }
  else
  {
    std::stringstream ss;
    ss << "Index " << index << " out of range (" << m_SliceIsLoaded.size() << " indices reserved)";
    throw std::invalid_argument( ss.str() );
  }
}

bool mitk::DICOMImageBlockDescriptor::IsSliceLoaded( unsigned int index ) const
{
  if ( index < m_SliceIsLoaded.size() )
  {
    return m_SliceIsLoaded[index];
  }
  else
  {
    std::stringstream ss;
    ss << "Index " << index << " out of range (" << m_SliceIsLoaded.size() << " indices reserved)";
    throw std::invalid_argument( ss.str() );
  }
}

bool mitk::DICOMImageBlockDescriptor::AllSlicesAreLoaded() const
{
  bool allLoaded = true;
  for ( auto iter = m_SliceIsLoaded.cbegin(); iter != m_SliceIsLoaded.cend(); ++iter )
  {
    allLoaded &= *iter;
  }

  return allLoaded;
}

/*
   PS defined      IPS defined     PS==IPS
        0               0                     --> UNKNOWN spacing, loader will invent
        0               1                     --> spacing as at detector surface
        1               0                     --> spacing as in patient
        1               1             0       --> detector surface spacing CORRECTED for geometrical
   magnifications: spacing as in patient
        1               1             1       --> detector surface spacing NOT corrected for geometrical
   magnifications: spacing as at detector
*/
mitk::PixelSpacingInterpretation mitk::DICOMImageBlockDescriptor::GetPixelSpacingInterpretation() const
{
  if ( m_ImageFrameList.empty() || m_TagCache.IsExpired() )
  {
    MITK_ERROR << "Invalid call to GetPixelSpacingInterpretation. Need to have initialized tag-cache!";
    return SpacingUnknown;
  }

  const std::string pixelSpacing       = this->GetPixelSpacing();
  const std::string imagerPixelSpacing = this->GetImagerPixelSpacing();

  if ( pixelSpacing.empty() )
  {
    if ( imagerPixelSpacing.empty() )
    {
      return SpacingUnknown;
    }
    else
    {
      return SpacingAtDetector;
    }
  }
  else // Pixel Spacing defined
  {
    if ( imagerPixelSpacing.empty() )
    {
      return SpacingInPatient;
    }
    else if ( pixelSpacing != imagerPixelSpacing )
    {
      return SpacingInPatient;
    }
    else
    {
      return SpacingAtDetector;
    }
  }
}

std::string mitk::DICOMImageBlockDescriptor::GetPixelSpacing() const
{
  if ( m_ImageFrameList.empty() || m_TagCache.IsExpired() )
  {
    MITK_ERROR << "Invalid call to GetPixelSpacing. Need to have initialized tag-cache!";
    return std::string( "" );
  }

  static const DICOMTag tagPixelSpacing( 0x0028, 0x0030 );
  return m_TagCache.Lock()->GetTagValue( m_ImageFrameList.front(), tagPixelSpacing ).value;
}

std::string mitk::DICOMImageBlockDescriptor::GetImagerPixelSpacing() const
{
  if ( m_ImageFrameList.empty() || m_TagCache.IsExpired() )
  {
    MITK_ERROR << "Invalid call to GetImagerPixelSpacing. Need to have initialized tag-cache!";
    return std::string( "" );
  }

  static const DICOMTag tagImagerPixelSpacing( 0x0018, 0x1164 );
  return m_TagCache.Lock()->GetTagValue( m_ImageFrameList.front(), tagImagerPixelSpacing ).value;
}

void mitk::DICOMImageBlockDescriptor::GetDesiredMITKImagePixelSpacing( ScalarType& spacingX,
                                                                       ScalarType& spacingY ) const
{
  const std::string pixelSpacing = this->GetPixelSpacing();
  // preference for "in patient" pixel spacing
  if ( !DICOMStringToSpacing( pixelSpacing, spacingX, spacingY ) )
  {
    const std::string imagerPixelSpacing = this->GetImagerPixelSpacing();
    // fallback to "on detector" spacing
    if ( !DICOMStringToSpacing( imagerPixelSpacing, spacingX, spacingY ) )
    {
      // at this point we have no hints whether the spacing is correct
      // do a quick sanity check and either trust in the input or set both to 1
      // We assume neither spacing to be negative, zero or unexpectedly large for
      // medical images
      if (spacingX < mitk::eps || spacingX > 1000 || spacingY < mitk::eps || spacingY > 1000)
      {
        spacingX = spacingY = 1.0;
      }
    }
  }
}

void mitk::DICOMImageBlockDescriptor::SetProperty( const std::string& key, BaseProperty* value )
{
  m_PropertyList->SetProperty( key, value );
}

mitk::BaseProperty* mitk::DICOMImageBlockDescriptor::GetProperty( const std::string& key ) const
{
  this->UpdateImageDescribingProperties();
  return m_PropertyList->GetProperty( key );
}

std::string mitk::DICOMImageBlockDescriptor::GetPropertyAsString( const std::string& key ) const
{
  this->UpdateImageDescribingProperties();
  const mitk::BaseProperty::Pointer property = m_PropertyList->GetProperty( key );
  if ( property.IsNotNull() )
  {
    return property->GetValueAsString();
  }
  else
  {
    return std::string( "" );
  }
}

void mitk::DICOMImageBlockDescriptor::SetFlag( const std::string& key, bool value )
{
  m_PropertyList->ReplaceProperty( key, BoolProperty::New( value ) );
}

bool mitk::DICOMImageBlockDescriptor::GetFlag( const std::string& key, bool defaultValue ) const
{
  this->UpdateImageDescribingProperties();
  BoolProperty::ConstPointer boolProp = dynamic_cast<BoolProperty*>( this->GetProperty( key ) );
  if ( boolProp.IsNotNull() )
  {
    return boolProp->GetValue();
  }
  else
  {
    return defaultValue;
  }
}

void mitk::DICOMImageBlockDescriptor::SetIntProperty( const std::string& key, int value )
{
  m_PropertyList->ReplaceProperty( key, IntProperty::New( value ) );
}

int mitk::DICOMImageBlockDescriptor::GetIntProperty( const std::string& key, int defaultValue ) const
{
  this->UpdateImageDescribingProperties();
  IntProperty::ConstPointer intProp = dynamic_cast<IntProperty*>( this->GetProperty( key ) );
  if ( intProp.IsNotNull() )
  {
    return intProp->GetValue();
  }
  else
  {
    return defaultValue;
  }
}

double mitk::DICOMImageBlockDescriptor::stringtodouble( const std::string& str ) const
{
  double d;
  std::string trimmedstring( str );
  try
  {
    trimmedstring = trimmedstring.erase( trimmedstring.find_last_not_of( " \n\r\t" ) + 1 );
  }
  catch ( ... )
  {
    // no last not of
  }

  std::string firstcomponent( trimmedstring );
  try
  {
    firstcomponent = trimmedstring.erase( trimmedstring.find_first_of( "\\" ) );
  }
  catch ( ... )
  {
    // no last not of
  }

  std::istringstream converter( firstcomponent );
  if ( !firstcomponent.empty() && ( converter >> d ) && converter.eof() )
  {
    return d;
  }
  else
  {
    throw std::invalid_argument( "Argument is not a convertable number" );
  }
}

mitk::Image::Pointer mitk::DICOMImageBlockDescriptor::DescribeImageWithProperties( Image* mitkImage )
{
  // TODO: this is a collection of properties that have been provided by the
  // legacy DicomSeriesReader.
  // We should at some point clean up this collection and name them in a more
  // consistent way!

  if ( !mitkImage )
    return mitkImage;

  // first part: add some tags that describe individual slices
  // these propeties are defined at analysis time (see UpdateImageDescribingProperties())

  const char* propertyKeySliceLocation  = "dicom.image.0020.1041";
  const char* propertyKeyInstanceNumber = "dicom.image.0020.0013";
  const char* propertyKeySOPInstanceUID = "dicom.image.0008.0018";

  mitkImage->SetProperty( propertyKeySliceLocation, this->GetProperty( "sliceLocationForSlices" ) );
  mitkImage->SetProperty( propertyKeyInstanceNumber, this->GetProperty( "instanceNumberForSlices" ) );
  mitkImage->SetProperty( propertyKeySOPInstanceUID, this->GetProperty( "SOPInstanceUIDForSlices" ) );
  mitkImage->SetProperty( "files", this->GetProperty( "filenamesForSlices" ) );

  // second part: add properties that describe the whole image block
  mitkImage->SetProperty( "dicomseriesreader.SOPClassUID", StringProperty::New( this->GetSOPClassUID() ) );

  mitkImage->SetProperty( "dicomseriesreader.SOPClass", StringProperty::New( this->GetSOPClassUIDAsName() ) );

  mitkImage->SetProperty(
    "dicomseriesreader.PixelSpacingInterpretationString",
    StringProperty::New( PixelSpacingInterpretationToString( this->GetPixelSpacingInterpretation() ) ) );
  mitkImage->SetProperty(
    "dicomseriesreader.PixelSpacingInterpretation",
    GenericProperty<PixelSpacingInterpretation>::New( this->GetPixelSpacingInterpretation() ) );

  mitkImage->SetProperty(
    "dicomseriesreader.ReaderImplementationLevelString",
    StringProperty::New( ReaderImplementationLevelToString( m_ReaderImplementationLevel ) ) );

  mitkImage->SetProperty( "dicomseriesreader.ReaderImplementationLevel",
                          GenericProperty<ReaderImplementationLevel>::New( m_ReaderImplementationLevel ) );

  mitkImage->SetProperty( "dicomseriesreader.GantyTiltCorrected",
                          BoolProperty::New( this->GetTiltInformation().IsRegularGantryTilt() ) );

  mitkImage->SetProperty( "dicomseriesreader.3D+t", BoolProperty::New( this->GetFlag( "3D+t", false ) ) );

  // level window
  const std::string windowCenter = this->GetPropertyAsString( "windowCenter" );
  const std::string windowWidth  = this->GetPropertyAsString( "windowWidth" );
  try
  {
    const double level  = stringtodouble( windowCenter );
    const double window = stringtodouble( windowWidth );
    mitkImage->SetProperty( "levelwindow", LevelWindowProperty::New( LevelWindow( level, window ) ) );
  }
  catch ( ... )
  {
    // nothing, no levelwindow to be predicted...
  }

  const std::string modality = this->GetPropertyAsString( "modality" );
  mitkImage->SetProperty( "modality", StringProperty::New( modality ) );

  mitkImage->SetProperty( "dicom.pixel.PhotometricInterpretation",
                          this->GetProperty( "photometricInterpretation" ) );
  mitkImage->SetProperty( "dicom.image.imagetype", this->GetProperty( "imagetype" ) );

  mitkImage->SetProperty( "dicom.study.StudyDescription", this->GetProperty( "studyDescription" ) );
  mitkImage->SetProperty( "dicom.series.SeriesDescription", this->GetProperty( "seriesDescription" ) );

  mitkImage->SetProperty( "dicom.pixel.Rows", this->GetProperty( "rows" ) );
  mitkImage->SetProperty( "dicom.pixel.Columns", this->GetProperty( "columns" ) );

  // third part: get all found additional tags of interest

  for (auto tag : m_FoundAdditionalTags)
  {
    BaseProperty* prop = this->GetProperty(tag);
    if (prop)
    {
      mitkImage->SetProperty(tag.c_str(), prop);
    }
  }

  // fourth part: get something from ImageIO. BUT this needs to be created elsewhere. or not at all!

  return mitkImage;
}

void mitk::DICOMImageBlockDescriptor::SetReaderImplementationLevel( const ReaderImplementationLevel& level )
{
  m_ReaderImplementationLevel = level;
}

mitk::ReaderImplementationLevel mitk::DICOMImageBlockDescriptor::GetReaderImplementationLevel() const
{
  return m_ReaderImplementationLevel;
}

std::string mitk::DICOMImageBlockDescriptor::GetSOPClassUID() const
{
  if ( !m_ImageFrameList.empty() && !m_TagCache.IsExpired() )
  {
    static const DICOMTag tagSOPClassUID( 0x0008, 0x0016 );
    return m_TagCache.Lock()->GetTagValue( m_ImageFrameList.front(), tagSOPClassUID ).value;
  }
  else
  {
    MITK_ERROR
      << "Invalid call to DICOMImageBlockDescriptor::GetSOPClassUID(). Need to have initialized tag-cache!";
    return std::string( "" );
  }
}

std::string mitk::DICOMImageBlockDescriptor::GetSOPClassUIDAsName() const
{
  if ( !m_ImageFrameList.empty() && !m_TagCache.IsExpired() )
  {
    gdcm::UIDs uidKnowledge;
    uidKnowledge.SetFromUID( this->GetSOPClassUID().c_str() );

    const char* name = uidKnowledge.GetName();
    if ( name )
    {
      return std::string( name );
    }
    else
    {
      return std::string( "" );
    }
  }
  else
  {
    MITK_ERROR << "Invalid call to DICOMImageBlockDescriptor::GetSOPClassUIDAsName(). Need to have "
                  "initialized tag-cache!";
    return std::string( "" );
  }
}

int mitk::DICOMImageBlockDescriptor::GetNumberOfTimeSteps() const
{
  int result = 1;
  this->m_PropertyList->GetIntProperty("timesteps", result);

  return result;
};

int mitk::DICOMImageBlockDescriptor::GetNumberOfFramesPerTimeStep() const
{
  const int numberOfTimesteps = this->GetNumberOfTimeSteps();
  int numberOfFramesPerTimestep = this->m_ImageFrameList.size() / numberOfTimesteps;
  assert(int(double((double)this->m_ImageFrameList.size() / (double)numberOfTimesteps))
    == numberOfFramesPerTimestep); // this should hold

  return numberOfFramesPerTimestep;
};


void mitk::DICOMImageBlockDescriptor::SetTagCache( DICOMTagCache* privateCache )
{
  // this must only be used during loading and never afterwards
  m_TagCache = privateCache;
}

#define printPropertyRange( label, property_name )                                 \
  \
{                                                                             \
    const std::string first = this->GetPropertyAsString( #property_name "First" ); \
    const std::string last  = this->GetPropertyAsString( #property_name "Last" );  \
    if ( !first.empty() || !last.empty() )                                         \
    {                                                                              \
      if ( first == last )                                                         \
      {                                                                            \
        os << "  " label ": '" << first << "'" << std::endl;                       \
      }                                                                            \
      else                                                                         \
      {                                                                            \
        os << "  " label ": '" << first << "' - '" << last << "'" << std::endl;    \
      }                                                                            \
    }                                                                              \
  \
}

#define printProperty( label, property_name )                              \
  \
{                                                                     \
    const std::string first = this->GetPropertyAsString( #property_name ); \
    if ( !first.empty() )                                                  \
    {                                                                      \
      os << "  " label ": '" << first << "'" << std::endl;                 \
    }                                                                      \
  \
}

#define printBool( label, commands )                                           \
  \
{                                                                         \
    os << "  " label ": '" << ( commands ? "yes" : "no" ) << "'" << std::endl; \
  \
}




void mitk::DICOMImageBlockDescriptor::Print(std::ostream& os, bool filenameDetails) const
{
  os << "  Number of Frames: '" << m_ImageFrameList.size() << "'" << std::endl;
  os << "  SOP class: '" << this->GetSOPClassUIDAsName() << "'" << std::endl;

  printProperty( "Series Number", seriesNumber );
  printProperty( "Study Description", studyDescription );
  printProperty( "Series Description", seriesDescription );
  printProperty( "Modality", modality );
  printProperty( "Sequence Name", sequenceName );

  printPropertyRange( "Slice Location", sliceLocation );
  printPropertyRange( "Acquisition Number", acquisitionNumber );
  printPropertyRange( "Instance Number", instanceNumber );
  printPropertyRange( "Image Position", imagePositionPatient );
  printProperty( "Image Orientation", orientation );

  os << "  Pixel spacing interpretation: '"
     << PixelSpacingInterpretationToString( this->GetPixelSpacingInterpretation() ) << "'" << std::endl;
  printBool( "Gantry Tilt", this->GetTiltInformation().IsRegularGantryTilt() )
    // printBool("3D+t", this->GetFlag("3D+t",false))

    // os << "  MITK image loaded: '" << (this->GetMitkImage().IsNotNull() ? "yes" : "no") << "'" <<
    // std::endl;
    if ( filenameDetails )
  {
    os << "  Files in this image block:" << std::endl;
    for ( auto frameIter = m_ImageFrameList.begin(); frameIter != m_ImageFrameList.end(); ++frameIter )
    {
      os << "    " << ( *frameIter )->Filename;
      if ( ( *frameIter )->FrameNo > 0 )
      {
        os << ", " << ( *frameIter )->FrameNo;
      }
      os << std::endl;
    }
  }
}

#define storeTagValueToProperty( tag_name, tag_g, tag_e )                  \
  \
{                                                                     \
    const DICOMTag t( tag_g, tag_e );                                      \
    const std::string tagValue = tagCache->GetTagValue( firstFrame, t ).value; \
    const_cast<DICOMImageBlockDescriptor*>( this )                         \
      ->SetProperty( #tag_name, StringProperty::New( tagValue ) );         \
  \
}

#define storeTagValueRangeToProperty( tag_name, tag_g, tag_e )                  \
  \
{                                                                          \
    const DICOMTag t( tag_g, tag_e );                                           \
    const std::string tagValueFirst = tagCache->GetTagValue( firstFrame, t ).value; \
    const std::string tagValueLast  = tagCache->GetTagValue( lastFrame, t ).value;  \
    const_cast<DICOMImageBlockDescriptor*>( this )                              \
      ->SetProperty( #tag_name "First", StringProperty::New( tagValueFirst ) ); \
    const_cast<DICOMImageBlockDescriptor*>( this )                              \
      ->SetProperty( #tag_name "Last", StringProperty::New( tagValueLast ) );   \
  \
}


void mitk::DICOMImageBlockDescriptor::UpdateImageDescribingProperties() const
{
  if ( !m_PropertiesOutOfDate )
    return;

  if ( !m_ImageFrameList.empty() )
  {
    if ( m_TagCache.IsExpired() )
    {
      MITK_ERROR << "Invalid call to DICOMImageBlockDescriptor::UpdateImageDescribingProperties(). Need to "
                    "have initialized tag-cache!";
      return;
    }

    auto tagCache = m_TagCache.Lock();

    const DICOMImageFrameInfo::Pointer firstFrame = m_ImageFrameList.front();
    const DICOMImageFrameInfo::Pointer lastFrame  = m_ImageFrameList.back();

    // see macros above
    storeTagValueToProperty( seriesNumber, 0x0020, 0x0011 );
    storeTagValueToProperty( studyDescription, 0x0008, 0x1030 );
    storeTagValueToProperty( seriesDescription, 0x0008, 0x103e );
    storeTagValueToProperty( modality, 0x0008, 0x0060 );
    storeTagValueToProperty( sequenceName, 0x0018, 0x0024 );
    storeTagValueToProperty( orientation, 0x0020, 0x0037 );
    storeTagValueToProperty( rows, 0x0028, 0x0010 );
    storeTagValueToProperty( columns, 0x0028, 0x0011 );

    storeTagValueRangeToProperty( sliceLocation, 0x0020, 0x1041 );
    storeTagValueRangeToProperty( acquisitionNumber, 0x0020, 0x0012 );
    storeTagValueRangeToProperty( instanceNumber, 0x0020, 0x0013 );
    storeTagValueRangeToProperty( imagePositionPatient, 0x0020, 0x0032 );

    storeTagValueToProperty( windowCenter, 0x0028, 0x1050 );
    storeTagValueToProperty( windowWidth, 0x0028, 0x1051 );
    storeTagValueToProperty( imageType, 0x0008, 0x0008 );
    storeTagValueToProperty( photometricInterpretation, 0x0028, 0x0004 );

    // some per-image attributes
    // frames are just numbered starting from 0. timestep 1 (the second time-step) has frames starting at
    // (number-of-frames-per-timestep)
    //     std::string propertyKeySliceLocation = "dicom.image.0020.1041";
    //     std::string propertyKeyInstanceNumber = "dicom.image.0020.0013";
    //     std::string propertyKeySOPInstanceNumber = "dicom.image.0008.0018";

    StringLookupTable sliceLocationForSlices;
    StringLookupTable instanceNumberForSlices;
    StringLookupTable SOPInstanceUIDForSlices;
    StringLookupTable filenamesForSlices;

    const DICOMTag tagSliceLocation( 0x0020, 0x1041 );
    const DICOMTag tagInstanceNumber( 0x0020, 0x0013 );
    const DICOMTag tagSOPInstanceNumber( 0x0008, 0x0018 );

    std::unordered_map<std::string, DICOMCachedValueLookupTable> additionalTagResultList;

    unsigned int slice(0);
    int timePoint(-1);
    const int framesPerTimeStep = this->GetNumberOfFramesPerTimeStep();
    for ( auto frameIter = m_ImageFrameList.begin(); frameIter != m_ImageFrameList.end();
          ++slice, ++frameIter )
    {
      unsigned int zSlice = slice%framesPerTimeStep;
      if ( zSlice == 0)
      {
        timePoint++;
      }

      const std::string sliceLocation = tagCache->GetTagValue( *frameIter, tagSliceLocation ).value;
      sliceLocationForSlices.SetTableValue( slice, sliceLocation );

      const std::string instanceNumber = tagCache->GetTagValue( *frameIter, tagInstanceNumber ).value;
      instanceNumberForSlices.SetTableValue( slice, instanceNumber );

      const std::string sopInstanceUID = tagCache->GetTagValue( *frameIter, tagSOPInstanceNumber ).value;
      SOPInstanceUIDForSlices.SetTableValue( slice, sopInstanceUID );

      const std::string filename = ( *frameIter )->Filename;
      filenamesForSlices.SetTableValue( slice, filename );

      MITK_DEBUG << "Tag info for slice " << slice << ": SL '" << sliceLocation << "' IN '" << instanceNumber
                 << "' SOP instance UID '" << sopInstanceUID << "'";

      for (const auto& tag : m_AdditionalTagMap)
      {
        const DICOMTagCache::FindingsListType findings = tagCache->GetTagValue( *frameIter, tag.first );
        for (const auto& finding : findings)
        {
          if (finding.isValid)
          {
            std::string propKey = (tag.second.empty()) ? DICOMTagPathToPropertyName(finding.path) : tag.second;
          DICOMCachedValueInfo info{ static_cast<unsigned int>(timePoint), zSlice, finding.value };
            additionalTagResultList[propKey].SetTableValue(slice, info);
          }
        }
      }
    }

    // add property or properties with proper names
    auto* thisInstance = const_cast<DICOMImageBlockDescriptor*>( this );
    thisInstance->SetProperty( "sliceLocationForSlices",
                               StringLookupTableProperty::New( sliceLocationForSlices ) );

    thisInstance->SetProperty( "instanceNumberForSlices",
                               StringLookupTableProperty::New( instanceNumberForSlices ) );

    thisInstance->SetProperty( "SOPInstanceUIDForSlices",
                               StringLookupTableProperty::New( SOPInstanceUIDForSlices ) );

    thisInstance->SetProperty( "filenamesForSlices", StringLookupTableProperty::New( filenamesForSlices ) );


    //add properties for additional tags of interest

    for ( auto iter = additionalTagResultList.cbegin(); iter != additionalTagResultList.cend(); ++iter )
    {
      thisInstance->SetProperty( iter->first, m_PropertyFunctor( iter->second ) );
      thisInstance->m_FoundAdditionalTags.insert(m_FoundAdditionalTags.cend(),iter->first);
    }

    m_PropertiesOutOfDate = false;
  }
}


mitk::BaseProperty::Pointer
mitk::DICOMImageBlockDescriptor::GetPropertyForDICOMValues(const DICOMCachedValueLookupTable& cacheLookupTable)
{
  const auto& lookupTable = cacheLookupTable.GetLookupTable();
  typedef std::pair<int, DICOMCachedValueInfo> PairType;
  if ( std::adjacent_find(
         lookupTable.cbegin(),
         lookupTable.cend(),
         []( const PairType& lhs, const PairType& rhs ) { return lhs.second.Value != rhs.second.Value; } )
       == lookupTable.cend() )
  {
    return static_cast<mitk::BaseProperty::Pointer>(
      mitk::StringProperty::New(cacheLookupTable.GetTableValue(0).Value).GetPointer());
  }

  StringLookupTable stringTable;
  for (auto element : lookupTable)
  {
    stringTable.SetTableValue(element.first, element.second.Value);
  }

  return static_cast<mitk::BaseProperty::Pointer>(
    mitk::StringLookupTableProperty::New(stringTable).GetPointer());
}


void mitk::DICOMImageBlockDescriptor::SetTagLookupTableToPropertyFunctor( TagLookupTableToPropertyFunctor functor )
{
  if ( functor != nullptr )
  {
    m_PropertyFunctor = functor;
  }
}

mitk::BaseProperty::ConstPointer mitk::DICOMImageBlockDescriptor::GetConstProperty(const std::string &propertyKey,
  const std::string &/*contextName*/,  bool /*fallBackOnDefaultContext*/) const
{
  this->UpdateImageDescribingProperties();
  return m_PropertyList->GetConstProperty(propertyKey);
};

std::vector<std::string> mitk::DICOMImageBlockDescriptor::GetPropertyKeys(const std::string &/*contextName*/,  bool /*includeDefaultContext*/) const
{
  this->UpdateImageDescribingProperties();
  return m_PropertyList->GetPropertyKeys();
};

std::vector<std::string> mitk::DICOMImageBlockDescriptor::GetPropertyContextNames() const
{
  return std::vector<std::string>();
};
