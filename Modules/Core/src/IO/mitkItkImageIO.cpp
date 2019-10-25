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


#include "mitkItkImageIO.h"

#include <mitkImage.h>
#include <mitkImageAccessLock.h>
#include <mitkCustomMimeType.h>
#include <mitkIOMimeTypes.h>
#include <mitkLocaleSwitch.h>
#include <mitkCoreServices.h>
#include <mitkIPropertyPersistence.h>
#include <mitkArbitraryTimeGeometry.h>

#include <itkImage.h>
#include <itkImageIOFactory.h>
#include <itkImageFileReader.h>
#include <itkImageIORegion.h>
#include <itkMetaDataObject.h>

#include <algorithm>

namespace mitk {

const char * const PROPERTY_NAME_TIMEGEOMETRY_TYPE = "org.mitk.timegeometry.type";
const char * const PROPERTY_NAME_TIMEGEOMETRY_TIMEPOINTS = "org.mitk.timegeometry.timepoints";
const char * const PROPERTY_KEY_TIMEGEOMETRY_TYPE = "org_mitk_timegeometry_type";
const char * const PROPERTY_KEY_TIMEGEOMETRY_TIMEPOINTS = "org_mitk_timegeometry_timepoints";


ItkImageIO::ItkImageIO(const ItkImageIO& other)
  : AbstractFileIO(other)
  , m_ImageIO(dynamic_cast<itk::ImageIOBase*>(other.m_ImageIO->Clone().GetPointer()))
{
  this->InitializeDefaultMetaDataKeys();
}

std::vector<std::string> ItkImageIO::FixUpImageIOExtensions(const std::string& imageIOName)
{
  std::vector<std::string> extensions;
  // Try to fix-up some known ITK image IO classes
  if (imageIOName == "GiplImageIO")
  {
    extensions.push_back("gipl");
    extensions.push_back("gipl.gz");
  }
  else if (imageIOName == "GDCMImageIO")
  {
    extensions.push_back("gdcm");
    extensions.push_back("dcm");
    extensions.push_back("DCM");
    extensions.push_back("dc3");
    extensions.push_back("DC3");
    extensions.push_back("ima");
    extensions.push_back("img");
  }
  else if (imageIOName == "PNGImageIO")
  {
    extensions.push_back("png");
    extensions.push_back("PNG");
  }
  else if (imageIOName == "StimulateImageIO")
  {
    extensions.push_back("spr");
  }
  else if (imageIOName == "HDF5ImageIO")
  {
    extensions.push_back("hdf");
    extensions.push_back("h4");
    extensions.push_back("hdf4");
    extensions.push_back("h5");
    extensions.push_back("hdf5");
    extensions.push_back("he4");
    extensions.push_back("he5");
    extensions.push_back("hd5");
  }
  else if (imageIOName == "GE4ImageIO" || imageIOName == "GE5ImageIO")
  {
    extensions.push_back("");
  }

  if (!extensions.empty())
  {
    MITK_DEBUG << "Fixing up known extensions for " << imageIOName;
  }

  return extensions;
}

ItkImageIO::ItkImageIO(itk::ImageIOBase::Pointer imageIO)
  : AbstractFileIO(Image::GetStaticNameOfClass())
  , m_ImageIO(imageIO)
{
  if (m_ImageIO.IsNull() )
  {
    mitkThrow() << "ITK ImageIOBase argument must not be NULL";
  }

  this->AbstractFileReader::SetMimeTypePrefix(IOMimeTypes::DEFAULT_BASE_NAME() + ".image.");
  this->InitializeDefaultMetaDataKeys();

  std::vector<std::string> readExtensions = m_ImageIO->GetSupportedReadExtensions();

  if (readExtensions.empty())
  {
    std::string imageIOName = m_ImageIO->GetNameOfClass();
    MITK_DEBUG << "ITK ImageIOBase " << imageIOName << " does not provide read extensions";
    readExtensions = FixUpImageIOExtensions(imageIOName);
  }

  CustomMimeType customReaderMimeType;
  customReaderMimeType.SetCategory("Images");
  for(std::vector<std::string>::const_iterator iter = readExtensions.begin(),
      endIter = readExtensions.end(); iter != endIter; ++iter)
  {
    std::string extension = *iter;
    if (!extension.empty() && extension[0] == '.')
    {
      extension.assign(iter->begin()+1, iter->end());
    }
    customReaderMimeType.AddExtension(extension);
  }
  this->AbstractFileReader::SetMimeType(customReaderMimeType);

  std::vector<std::string> writeExtensions = imageIO->GetSupportedWriteExtensions();
  if (writeExtensions.empty())
  {
    std::string imageIOName = imageIO->GetNameOfClass();
    MITK_DEBUG << "ITK ImageIOBase " << imageIOName << " does not provide write extensions";
    writeExtensions = FixUpImageIOExtensions(imageIOName);
  }

  if (writeExtensions != readExtensions)
  {
    CustomMimeType customWriterMimeType;
    customWriterMimeType.SetCategory("Images");
    for(std::vector<std::string>::const_iterator iter = writeExtensions.begin(),
        endIter = writeExtensions.end(); iter != endIter; ++iter)
    {
      std::string extension = *iter;
      if (!extension.empty() && extension[0] == '.')
      {
        extension.assign(iter->begin()+1, iter->end());
      }
      customWriterMimeType.AddExtension(extension);
    }
    this->AbstractFileWriter::SetMimeType(customWriterMimeType);
  }

  std::string description = std::string("ITK ") + imageIO->GetNameOfClass();
  this->SetReaderDescription(description);
  this->SetWriterDescription(description);

  this->RegisterService();
}

ItkImageIO::ItkImageIO(const CustomMimeType& mimeType, itk::ImageIOBase::Pointer imageIO, int rank)
  : AbstractFileIO(Image::GetStaticNameOfClass(), mimeType, std::string("ITK ") + imageIO->GetNameOfClass())
  , m_ImageIO(imageIO)
{
  if (m_ImageIO.IsNull() )
  {
    mitkThrow() << "ITK ImageIOBase argument must not be NULL";
  }

  this->AbstractFileReader::SetMimeTypePrefix(IOMimeTypes::DEFAULT_BASE_NAME() + ".image.");
  this->InitializeDefaultMetaDataKeys();

  if (rank)
  {
    this->AbstractFileReader::SetRanking(rank);
    this->AbstractFileWriter::SetRanking(rank);
  }

  this->RegisterService();
}

/**Helper function that converts the content of a meta data into a time point vector.
 * If MetaData is not valid or cannot be converted an empty vector is returned.*/
std::vector<TimePointType> ConvertMetaDataObjectToTimePointList(const itk::MetaDataObjectBase* data)
{
  const itk::MetaDataObject<std::string>* timeGeometryTimeData = dynamic_cast<const itk::MetaDataObject<std::string>*>(data);
  std::vector<TimePointType> result;

  if (timeGeometryTimeData)
  {
    std::string dataStr = timeGeometryTimeData->GetMetaDataObjectValue();
    std::stringstream stream(dataStr);
    TimePointType tp;
    while (stream >> tp)
    {
      result.push_back(tp);
    }
  }

  return result;
};

std::vector<BaseData::Pointer> ItkImageIO::Read()
{
  std::vector<BaseData::Pointer> result;
  mitk::LocaleSwitch localeSwitch("C");

  Image::Pointer image = Image::New();

  const unsigned int MINDIM = 2;
  const unsigned int MAXDIM = 4;

  const std::string path = this->GetLocalFileName();

  MITK_INFO << "loading " << path << " via itk::ImageIOFactory... " << std::endl;

  // Check to see if we can read the file given the name or prefix
  if (path.empty())
  {
    mitkThrow() << "Empty filename in mitk::ItkImageIO ";
  }

  // Got to allocate space for the image. Determine the characteristics of
  // the image.
  m_ImageIO->SetFileName( path );
  m_ImageIO->ReadImageInformation();

  unsigned int ndim = m_ImageIO->GetNumberOfDimensions();
  if ( ndim < MINDIM || ndim > MAXDIM )
  {
    MITK_WARN << "Sorry, only dimensions 2, 3 and 4 are supported. The given file has " << ndim << " dimensions! Reading as 4D.";
    ndim = MAXDIM;
  }

  itk::ImageIORegion ioRegion( ndim );
  itk::ImageIORegion::SizeType ioSize = ioRegion.GetSize();
  itk::ImageIORegion::IndexType ioStart = ioRegion.GetIndex();

  unsigned int dimensions[ MAXDIM ];
  dimensions[ 0 ] = 0;
  dimensions[ 1 ] = 0;
  dimensions[ 2 ] = 0;
  dimensions[ 3 ] = 0;

  ScalarType spacing[ MAXDIM ];
  spacing[ 0 ] = 1.0f;
  spacing[ 1 ] = 1.0f;
  spacing[ 2 ] = 1.0f;
  spacing[ 3 ] = 1.0f;

  Point3D origin;
  origin.Fill(0);

  unsigned int i;
  for ( i = 0; i < ndim ; ++i )
  {
    ioStart[ i ] = 0;
    ioSize[ i ] = m_ImageIO->GetDimensions( i );
    if(i<MAXDIM)
    {
      dimensions[ i ] = m_ImageIO->GetDimensions( i );
      spacing[ i ] = m_ImageIO->GetSpacing( i );
      if(spacing[ i ] <= 0)
        spacing[ i ] = 1.0f;
    }
    if(i<3)
    {
      origin[ i ] = m_ImageIO->GetOrigin( i );
    }
  }

  ioRegion.SetSize( ioSize );
  ioRegion.SetIndex( ioStart );

  MITK_INFO << "ioRegion: " << ioRegion << std::endl;
  m_ImageIO->SetIORegion( ioRegion );
  image->Initialize(MakePixelType(m_ImageIO), ndim, dimensions);
  m_ImageIO->Read(image->GetVolumeData()->GetData());
  
  const itk::MetaDataDictionary& dictionary = m_ImageIO->GetMetaDataDictionary();

  // access direction of itk::Image and include spacing
  mitk::Matrix3D matrix;
  matrix.SetIdentity();
  unsigned int j, itkDimMax3 = (ndim >= 3? 3 : ndim);
  for ( i=0; i < itkDimMax3; ++i)
    for( j=0; j < itkDimMax3; ++j )
      matrix[i][j] = m_ImageIO->GetDirection(j)[i];

  // re-initialize PlaneGeometry with origin and direction
  PlaneGeometry* planeGeometry = image->GetSlicedGeometry(0)->GetPlaneGeometry(0);
  planeGeometry->SetOrigin(origin);
  planeGeometry->GetIndexToWorldTransform()->SetMatrix(matrix);

  // re-initialize SlicedGeometry3D
  SlicedGeometry3D* slicedGeometry = image->GetSlicedGeometry(0);
  slicedGeometry->InitializeEvenlySpaced(planeGeometry, image->GetDimension(2));
  slicedGeometry->SetSpacing(spacing);

  MITK_INFO << slicedGeometry->GetCornerPoint(false,false,false);
  MITK_INFO << slicedGeometry->GetCornerPoint(true,true,true);

  // re-initialize TimeGeometry
  TimeGeometry::Pointer timeGeometry;

  if (dictionary.HasKey(PROPERTY_NAME_TIMEGEOMETRY_TYPE) || dictionary.HasKey(PROPERTY_KEY_TIMEGEOMETRY_TYPE))
  {  //also check for the name because of backwards compatibility. Past code version stored with the name and not with the key
    itk::MetaDataObject<std::string>::ConstPointer timeGeometryTypeData = nullptr;
    if (dictionary.HasKey(PROPERTY_NAME_TIMEGEOMETRY_TYPE))
    {
      timeGeometryTypeData = dynamic_cast<const itk::MetaDataObject<std::string>*>(dictionary.Get(
        PROPERTY_NAME_TIMEGEOMETRY_TYPE));
    }
    else
    {
      timeGeometryTypeData = dynamic_cast<const itk::MetaDataObject<std::string>*>(dictionary.Get(
        PROPERTY_KEY_TIMEGEOMETRY_TYPE));
    }

      if (timeGeometryTypeData->GetMetaDataObjectValue() == ArbitraryTimeGeometry::GetStaticNameOfClass())
      {
          MITK_INFO << "used time geometry: " << ArbitraryTimeGeometry::GetStaticNameOfClass() << std::endl;
          typedef std::vector<TimePointType> TimePointVector;
          TimePointVector timePoints;

          if (dictionary.HasKey(PROPERTY_NAME_TIMEGEOMETRY_TIMEPOINTS))
          {
              timePoints = ConvertMetaDataObjectToTimePointList(dictionary.Get(
                  PROPERTY_NAME_TIMEGEOMETRY_TIMEPOINTS));
          }
          else if(dictionary.HasKey(PROPERTY_KEY_TIMEGEOMETRY_TIMEPOINTS))
          {
            timePoints = ConvertMetaDataObjectToTimePointList(dictionary.Get(
              PROPERTY_KEY_TIMEGEOMETRY_TIMEPOINTS));
          }


          if (timePoints.size() - 1 != image->GetDimension(3))
          {
              MITK_ERROR << "Stored timepoints (" << timePoints.size() - 1 <<
                  ") and size of image time dimension (" << image->GetDimension(3) <<
                  ") do not match. Switch to ProportionalTimeGeometry fallback" << std::endl;
          }
          else
          {
              ArbitraryTimeGeometry::Pointer arbitraryTimeGeometry = ArbitraryTimeGeometry::New();
              TimePointVector::const_iterator pos = timePoints.begin();
              TimePointVector::const_iterator prePos = pos++;

              for (; pos != timePoints.end(); ++prePos, ++pos)
              {
                  arbitraryTimeGeometry->AppendTimeStepClone(slicedGeometry, *pos, *prePos);
              }

              timeGeometry = arbitraryTimeGeometry;
          }
      }
  }

  if (timeGeometry.IsNull())
  { //Fallback. If no other valid time geometry has been created, create a ProportionalTimeGeometry
    MITK_INFO << "used time geometry: " << ProportionalTimeGeometry::GetStaticNameOfClass() << std::endl;
    ProportionalTimeGeometry::Pointer propTimeGeometry = ProportionalTimeGeometry::New();
    propTimeGeometry->Initialize(slicedGeometry, image->GetDimension(3));
    timeGeometry = propTimeGeometry;
  }

  image->SetTimeGeometry(timeGeometry);

  MITK_INFO << "number of image components: "<< image->GetPixelType().GetNumberOfComponents() << std::endl;

  for (itk::MetaDataDictionary::ConstIterator iter = dictionary.Begin(), iterEnd = dictionary.End();
       iter != iterEnd; ++iter)
  {
    if (iter->second->GetMetaDataObjectTypeInfo() == typeid(std::string))
    {
      const std::string& key = iter->first;
      std::string assumedPropertyName = key;
      std::replace(assumedPropertyName.begin(), assumedPropertyName.end(), '_', '.');

      std::string mimeTypeName = GetMimeType()->GetName();

      //Check of there is already a info for the key and our mime type.
      IPropertyPersistence::InfoMapType infos = mitk::CoreServices::GetPropertyPersistence()->GetInfosByKey(key);

      auto predicate = [mimeTypeName](const std::pair<const std::string, PropertyPersistenceInfo::Pointer>& x){return x.second.IsNotNull() && x.second->GetMimeTypeName() == mimeTypeName; };
      auto finding = std::find_if(infos.begin(), infos.end(), predicate);

      if (finding == infos.end())
      {
        auto predicateWild = [](const std::pair<const std::string, PropertyPersistenceInfo::Pointer>& x){return x.second.IsNotNull() && x.second->GetMimeTypeName() == PropertyPersistenceInfo::ANY_MIMETYPE_NAME(); };
        finding = std::find_if(infos.begin(), infos.end(), predicateWild);
      }

      PropertyPersistenceInfo::Pointer info;

      if (finding != infos.end())
      {
        assumedPropertyName = finding->first;
        info = finding->second;
      }
      else
      { //we have not found anything suitable so we generate our own info
        info = PropertyPersistenceInfo::New(key);
        info->SetMimeTypeName(PropertyPersistenceInfo::ANY_MIMETYPE_NAME());
      }

      std::string value = dynamic_cast<itk::MetaDataObject<std::string>*>(iter->second.GetPointer())->GetMetaDataObjectValue();

      mitk::BaseProperty::Pointer loadedProp = info->GetDeserializationFunction()(value);

      image->SetProperty(assumedPropertyName.c_str(), loadedProp);

      // Read properties should be persisted unless they are default properties
      // which are written anyway
      bool isDefaultKey( false );

      for( const auto &defaultKey : m_DefaultMetaDataKeys )
      {
        if (defaultKey.length() <= assumedPropertyName.length())
        {
          // does the start match the default key
          if (assumedPropertyName.substr(0, defaultKey.length()).find(defaultKey) != std::string::npos)
          {
            isDefaultKey = true;
            break;
          }
        }
      }

      if( !isDefaultKey )
      {
        mitk::CoreServices::GetPropertyPersistence()->AddInfo(assumedPropertyName, info);
      }
    }
  }

  MITK_INFO << "...finished!" << std::endl;

  result.push_back(image.GetPointer());
  return result;
}

AbstractFileIO::ConfidenceLevel ItkImageIO::GetReaderConfidenceLevel() const
{
  return m_ImageIO->CanReadFile(GetLocalFileName().c_str()) ? IFileReader::Supported : IFileReader::Unsupported;
}

void ItkImageIO::Write()
{
  const mitk::Image* image = dynamic_cast<const mitk::Image*>(this->GetInput());

  if (image == NULL)
  {
    mitkThrow() << "Cannot write non-image data";
  }


  // Switch the current locale to "C"
  LocaleSwitch localeSwitch("C");

  // Clone the image geometry, because we might have to change it
  // for writing purposes
  BaseGeometry::Pointer geometry = image->GetGeometry()->Clone();

  // Check if geometry information will be lost
  if (image->GetDimension() == 2 &&
      !geometry->Is2DConvertable())
  {
    MITK_WARN << "Saving a 2D image with 3D geometry information. Geometry information will be lost! You might consider using Convert2Dto3DImageFilter before saving.";

    // set matrix to identity
    mitk::AffineTransform3D::Pointer affTrans = mitk::AffineTransform3D::New();
    affTrans->SetIdentity();
    mitk::Vector3D spacing = geometry->GetSpacing();
    mitk::Point3D origin = geometry->GetOrigin();
    geometry->SetIndexToWorldTransform(affTrans);
    geometry->SetSpacing(spacing);
    geometry->SetOrigin(origin);
  }

  LocalFile localFile(this);
  const std::string path = localFile.GetFileName();

  MITK_INFO << "Writing image: " << path << std::endl;

  try
  {
    // Implementation of writer using itkImageIO directly. This skips the use
    // of templated itkImageFileWriter, which saves the multiplexing on MITK side.

    const unsigned int dimension = image->GetDimension();
    const unsigned int* const dimensions = image->GetDimensions();
    const mitk::PixelType pixelType = image->GetPixelType();
    const mitk::Vector3D mitkSpacing = geometry->GetSpacing();
    const mitk::Point3D mitkOrigin = geometry->GetOrigin();

    // Due to templating in itk, we are forced to save a 4D spacing and 4D Origin,
    // though they are not supported in MITK
    itk::Vector<double, 4u> spacing4D;
    spacing4D[0] = mitkSpacing[0];
    spacing4D[1] = mitkSpacing[1];
    spacing4D[2] = mitkSpacing[2];
    spacing4D[3] = 1; // There is no support for a 4D spacing. However, we should have a valid value here

    itk::Vector<double, 4u> origin4D;
    origin4D[0] = mitkOrigin[0];
    origin4D[1] = mitkOrigin[1];
    origin4D[2] = mitkOrigin[2];
    origin4D[3] = 0; // There is no support for a 4D origin. However, we should have a valid value here

    // Set the necessary information for imageIO
    m_ImageIO->SetNumberOfDimensions(dimension);
    m_ImageIO->SetPixelType(pixelType.GetPixelType());
    m_ImageIO->SetComponentType(pixelType.GetComponentType() < PixelComponentUserType ?
                                  static_cast<itk::ImageIOBase::IOComponentType>(pixelType.GetComponentType()) :
                                  itk::ImageIOBase::UNKNOWNCOMPONENTTYPE);
    m_ImageIO->SetNumberOfComponents( pixelType.GetNumberOfComponents() );

    itk::ImageIORegion ioRegion( dimension );

    for(unsigned int i = 0; i < dimension; i++)
    {
      m_ImageIO->SetDimensions(i, dimensions[i]);
      m_ImageIO->SetSpacing(i, spacing4D[i]);
      m_ImageIO->SetOrigin(i, origin4D[i]);

      mitk::Vector3D mitkDirection;
      mitkDirection.SetVnlVector(geometry->GetIndexToWorldTransform()->GetMatrix().GetVnlMatrix().get_column(i));
      itk::Vector<double, 4u> direction4D;
      direction4D[0] = mitkDirection[0];
      direction4D[1] = mitkDirection[1];
      direction4D[2] = mitkDirection[2];

      // MITK only supports a 3x3 direction matrix. Due to templating in itk, however, we must
      // save a 4x4 matrix for 4D images. in this case, add an homogneous component to the matrix.
      if (i == 3)
      {
        direction4D[3] = 1; // homogenous component
      }
      else
      {
        direction4D[3] = 0;
      }
      vnl_vector<double> axisDirection(dimension);
      for(unsigned int j = 0; j < dimension; j++)
      {
        axisDirection[j] = direction4D[j] / spacing4D[i];
      }
      m_ImageIO->SetDirection(i, axisDirection);

      ioRegion.SetSize(i, image->GetLargestPossibleRegion().GetSize(i));
      ioRegion.SetIndex(i, image->GetLargestPossibleRegion().GetIndex(i));
    }

    //use compression if available
    m_ImageIO->UseCompressionOn();

    m_ImageIO->SetIORegion(ioRegion);
    m_ImageIO->SetFileName(path);

    // Handle time geometry
    const ArbitraryTimeGeometry* arbitraryTG = dynamic_cast<const ArbitraryTimeGeometry*>(image->GetTimeGeometry());
    if (arbitraryTG)
    {
      itk::EncapsulateMetaData<std::string>(m_ImageIO->GetMetaDataDictionary(), PROPERTY_KEY_TIMEGEOMETRY_TYPE, ArbitraryTimeGeometry::GetStaticNameOfClass());

      std::stringstream stream;
      stream << arbitraryTG->GetTimeBounds(0)[0];
      for (TimeStepType pos = 0; pos<arbitraryTG->CountTimeSteps(); ++pos)
      {
        stream << " " << arbitraryTG->GetTimeBounds(pos)[1];
      }
      std::string data = stream.str();

      itk::EncapsulateMetaData<std::string>(m_ImageIO->GetMetaDataDictionary(), PROPERTY_KEY_TIMEGEOMETRY_TIMEPOINTS, data);
    }

    // Handle properties
    mitk::PropertyList::Pointer imagePropertyList = image->GetPropertyList();

    for(const auto &property : *imagePropertyList->GetMap())
    {
      PropertyPersistenceInfo::Pointer info = mitk::CoreServices::GetPropertyPersistence()->GetInfo(property.first, GetMimeType()->GetName(), true);

      if( info.IsNull())
      {
        continue;
      }

      std::string value = info->GetSerializationFunction()(property.second);

      if( value == mitk::BaseProperty::VALUE_CANNOT_BE_CONVERTED_TO_STRING )
      {
        continue;
      }

      std::string key = info->GetKey();

      itk::EncapsulateMetaData<std::string>(m_ImageIO->GetMetaDataDictionary(), key, value);
    }
    Image::Pointer imagePointer = const_cast<Image*>(image);
    ImageRegionAccessor accessor(imagePointer);
    ImageAccessLock lock(&accessor);

    m_ImageIO->Write(accessor.getData());
  }
  catch (const std::exception& e)
  {
    mitkThrow() << e.what();
  }
}

AbstractFileIO::ConfidenceLevel ItkImageIO::GetWriterConfidenceLevel() const
{
  // Check if the image dimension is supported
  const Image* image = dynamic_cast<const Image*>(this->GetInput());
  if (image == NULL)
  {
    // We cannot write a null object, DUH!
    return IFileWriter::Unsupported;
  }

  if ( ! m_ImageIO->SupportsDimension(image->GetDimension()))
  {
    // okay, dimension is not supported. We have to look at a special case:
    // 3D-Image with one slice. We can treat that as a 2D image.
    if ((image->GetDimension() == 3) && (image->GetSlicedGeometry()->GetSlices() == 1))
      return IFileWriter::Supported;
    else
      return IFileWriter::Unsupported;
  }

  // Check if geometry information will be lost
  if (image->GetDimension() == 2 &&
      !image->GetGeometry()->Is2DConvertable())
  {
    return IFileWriter::PartiallySupported;
  }
  return IFileWriter::Supported;
}

ItkImageIO* ItkImageIO::IOClone() const
{
  return new ItkImageIO(*this);
}

void ItkImageIO::InitializeDefaultMetaDataKeys()
{
  this->m_DefaultMetaDataKeys.push_back("NRRD.space");
  this->m_DefaultMetaDataKeys.push_back("NRRD.kinds");
  this->m_DefaultMetaDataKeys.push_back(PROPERTY_NAME_TIMEGEOMETRY_TYPE);
  this->m_DefaultMetaDataKeys.push_back(PROPERTY_NAME_TIMEGEOMETRY_TIMEPOINTS);
  this->m_DefaultMetaDataKeys.push_back("ITK.InputFilterName");
}

}
