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
#include <mitkProportionalTimeGeometry.h>

#include <itkImage.h>
#include <itkImageIOFactory.h>
#include <itkImageFileReader.h>
#include <itkImageIORegion.h>
#include <itkMetaDataObject.h>

namespace mitk {

ItkImageIO::ItkImageIO(const ItkImageIO& other)
  : AbstractFileIO(other)
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
    MITK_WARN << "Fixing up known extensions for " << imageIOName;
  }

  return extensions;
}

ItkImageIO::ItkImageIO(itk::ImageIOBase::Pointer imageIO)
  : AbstractFileIO()
  , m_ImageIO(imageIO)
{
  std::vector<std::string> extensions = imageIO->GetSupportedReadExtensions();
  if (extensions.empty())
  {
    std::string imageIOName = imageIO->GetNameOfClass();
    MITK_WARN << "ITK ImageIOBase " << imageIOName << " does not provide read extensions";
    extensions = FixUpImageIOExtensions(imageIOName);
  }
  for(std::vector<std::string>::const_iterator iter = extensions.begin(),
      endIter = extensions.end(); iter != endIter; ++iter)
  {
    std::string extension = *iter;
    if (!extension.empty() && extension[0] == '.')
    {
      extension.assign(extension.begin()+1, extension.end());
    }
    this->AbstractFileReader::AddExtension(extension);
  }

  extensions = imageIO->GetSupportedWriteExtensions();
  if (extensions.empty())
  {
    std::string imageIOName = imageIO->GetNameOfClass();
    MITK_WARN << "ITK ImageIOBase " << imageIOName << " does not provide write extensions";
    extensions = FixUpImageIOExtensions(imageIOName);
  }
  for(std::vector<std::string>::const_iterator iter = extensions.begin(),
      endIter = extensions.end(); iter != endIter; ++iter)
  {
    std::string extension = *iter;
    if (!extension.empty() && extension[0] == '.')
    {
      extension.assign(extension.begin()+1, extension.end());
    }
    this->AbstractFileWriter::AddExtension(extension);
  }

  this->SetCategory("Images");

  std::string description = std::string("ITK ") + imageIO->GetNameOfClass();
  this->SetReaderDescription(description);
  this->SetWriterDescription(description);

  this->RegisterService();
}

std::vector<BaseData::Pointer> ItkImageIO::Read(std::istream& stream)
{
  return mitk::AbstractFileReader::Read(stream);
}

std::vector<BaseData::Pointer> ItkImageIO::Read(const std::string& path)
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

  MITK_INFO << "loading " << path << " via itk::ImageIOFactory... " << std::endl;

  // Check to see if we can read the file given the name or prefix
  if (path.empty())
  {
    mitkThrow() << "Empty filename in mitk::ItkImageIO ";
  }

  itk::ImageIOBase::Pointer imageIO = itk::ImageIOFactory::CreateImageIO( path.c_str(), itk::ImageIOFactory::ReadMode );
  if ( imageIO.IsNull() )
  {
    mitkThrow() << "Could not create itk::ImageIOBase object for filename " << path;
  }

  // Got to allocate space for the image. Determine the characteristics of
  // the image.
  imageIO->SetFileName( path.c_str() );
  imageIO->ReadImageInformation();

  unsigned int ndim = imageIO->GetNumberOfDimensions();
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
    ioSize[ i ] = imageIO->GetDimensions( i );
    if(i<MAXDIM)
    {
      dimensions[ i ] = imageIO->GetDimensions( i );
      spacing[ i ] = imageIO->GetSpacing( i );
      if(spacing[ i ] <= 0)
        spacing[ i ] = 1.0f;
    }
    if(i<3)
    {
      origin[ i ] = imageIO->GetOrigin( i );
    }
  }

  ioRegion.SetSize( ioSize );
  ioRegion.SetIndex( ioStart );

  MITK_INFO << "ioRegion: " << ioRegion << std::endl;
  imageIO->SetIORegion( ioRegion );
  void* buffer = new unsigned char[imageIO->GetImageSizeInBytes()];
  imageIO->Read( buffer );

  image->Initialize( MakePixelType(imageIO), ndim, dimensions );
  image->SetImportChannel( buffer, 0, Image::ManageMemory );

  // access direction of itk::Image and include spacing
  mitk::Matrix3D matrix;
  matrix.SetIdentity();
  unsigned int j, itkDimMax3 = (ndim >= 3? 3 : ndim);
  for ( i=0; i < itkDimMax3; ++i)
    for( j=0; j < itkDimMax3; ++j )
      matrix[i][j] = imageIO->GetDirection(j)[i];

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

  const itk::MetaDataDictionary& dictionary = imageIO->GetMetaDataDictionary();
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


bool mitk::ItkImageIO::CanRead(const std::string& path) const
{
  if (AbstractFileReader::CanRead(path) == false) return false;

  itk::ImageIOBase::Pointer imageIO = itk::ImageIOFactory::CreateImageIO(path.c_str(), itk::ImageIOFactory::ReadMode);
  if (imageIO.IsNull())
    return false;

  return imageIO->CanReadFile(path.c_str());
}

void ItkImageIO::Write(const BaseData* data, const std::string& path)
{

}

void ItkImageIO::Write(const BaseData* data, std::ostream& stream)
{
  AbstractFileWriter::Write(data, stream);
}

ItkImageIO* ItkImageIO::Clone() const
{
  return new ItkImageIO(*this);
}

}
