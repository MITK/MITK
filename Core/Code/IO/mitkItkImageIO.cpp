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
#include <mitkImageReadAccessor.h>
#include <mitkCustomMimeType.h>
#include <mitkIOMimeTypes.h>

#include <itkImage.h>
#include <itkImageIOFactory.h>
#include <itkImageFileReader.h>
#include <itkImageIORegion.h>
#include <itkMetaDataObject.h>

#include <clocale>

namespace mitk {

ItkImageIO::ItkImageIO(const ItkImageIO& other)
  : AbstractFileIO(other)
  , m_ImageIO(dynamic_cast<itk::ImageIOBase*>(other.m_ImageIO->Clone().GetPointer()))
{
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

  if (rank)
  {
    this->AbstractFileReader::SetRanking(rank);
    this->AbstractFileWriter::SetRanking(rank);
  }

  this->RegisterService();
}

std::vector<BaseData::Pointer> ItkImageIO::Read()
{
  std::vector<BaseData::Pointer> result;

  const std::string& locale = "C";
  const std::string& currLocale = setlocale( LC_ALL, NULL );

  if ( locale.compare(currLocale)!=0 )
  {
    try
    {
      setlocale(LC_ALL, locale.c_str());
    }
    catch(...)
    {
      MITK_INFO << "Could not set locale " << locale;
    }
  }

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
  void* buffer = new unsigned char[m_ImageIO->GetImageSizeInBytes()];
  m_ImageIO->Read( buffer );

  image->Initialize( MakePixelType(m_ImageIO), ndim, dimensions );
  image->SetImportChannel( buffer, 0, Image::ManageMemory );

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
  ProportionalTimeGeometry::Pointer timeGeometry = ProportionalTimeGeometry::New();
  timeGeometry->Initialize(slicedGeometry, image->GetDimension(3));
  image->SetTimeGeometry(timeGeometry);

  buffer = NULL;
  MITK_INFO << "number of image components: "<< image->GetPixelType().GetNumberOfComponents() << std::endl;

  const itk::MetaDataDictionary& dictionary = m_ImageIO->GetMetaDataDictionary();
  for (itk::MetaDataDictionary::ConstIterator iter = dictionary.Begin(), iterEnd = dictionary.End();
       iter != iterEnd; ++iter)
  {
    std::string key = std::string("meta.") + iter->first;
    if (iter->second->GetMetaDataObjectTypeInfo() == typeid(std::string))
    {
      std::string value = dynamic_cast<itk::MetaDataObject<std::string>*>(iter->second.GetPointer())->GetMetaDataObjectValue();
      image->SetProperty(key.c_str(), mitk::StringProperty::New(value));
    }
  }

  MITK_INFO << "...finished!" << std::endl;

  try
  {
    setlocale(LC_ALL, currLocale.c_str());
  }
  catch(...)
  {
    MITK_INFO << "Could not reset locale " << currLocale;
  }

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

  struct LocaleSwitch
  {
    LocaleSwitch(const std::string& newLocale)
      : m_OldLocale(std::setlocale(LC_ALL, NULL))
      , m_NewLocale(newLocale)
    {
      if (m_OldLocale == NULL)
      {
        m_OldLocale = "";
      }
      else if (m_NewLocale != m_OldLocale)
      {
        // set the locale
        if (std::setlocale(LC_ALL, m_NewLocale.c_str()) == NULL)
        {
          MITK_INFO << "Could not set locale " << m_NewLocale;
          m_OldLocale = NULL;
        }
      }
    }

    ~LocaleSwitch()
    {
      if (m_OldLocale != NULL && std::setlocale(LC_ALL, m_OldLocale) == NULL)
      {
        MITK_INFO << "Could not reset locale " << m_OldLocale;
      }
    }

  private:
    const char* m_OldLocale;
    const std::string m_NewLocale;
  };

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

    // ***** Remove const_cast after bug 17952 is fixed ****
    ImageReadAccessor imageAccess(const_cast<mitk::Image*>(image));
    m_ImageIO->Write(imageAccess.GetData());
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
  if (image == NULL || !m_ImageIO->SupportsDimension(image->GetDimension()))
  {
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

}
