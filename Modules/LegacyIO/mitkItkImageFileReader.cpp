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


#include "mitkItkImageFileReader.h"
#include "mitkConfig.h"
#include "mitkException.h"
#include <mitkProportionalTimeGeometry.h>
#include <mitkLocaleSwitch.h>

#include <itkImageFileReader.h>
#include <itksys/SystemTools.hxx>
#include <itksys/Directory.hxx>
#include <itkImage.h>
//#include <itkImageSeriesReader.h>
#include <itkImageFileReader.h>
#include <itkImageIOFactory.h>
#include <itkImageIORegion.h>
#include <itkMetaDataObject.h>
//#include <itkImageSeriesReader.h>
//#include <itkDICOMImageIO2.h>
//#include <itkDICOMSeriesFileNames.h>
//#include <itkGDCMImageIO.h>
//#include <itkGDCMSeriesFileNames.h>
//#include <itkNumericSeriesFileNames.h>


void mitk::ItkImageFileReader::GenerateData()
{
  mitk::LocaleSwitch localeSwitch("C");
  mitk::Image::Pointer image = this->GetOutput();

  const unsigned int MINDIM = 2;
  const unsigned int MAXDIM = 4;

  MITK_INFO("mitkItkImageFileReader") << "loading " << m_FileName << " via itk::ImageIOFactory... " << std::endl;

  // Check to see if we can read the file given the name or prefix
  if ( m_FileName == "" )
  {
    mitkThrow() << "Empty filename in mitk::ItkImageFileReader ";
    return ;
  }

  itk::ImageIOBase::Pointer imageIO = itk::ImageIOFactory::CreateImageIO( m_FileName.c_str(), itk::ImageIOFactory::ReadMode );
  if ( imageIO.IsNull() )
  {
    //itkWarningMacro( << "File Type not supported!" );
    mitkThrow() << "Could not create itk::ImageIOBase object for filename " << m_FileName;
    return ;
  }

  // Got to allocate space for the image. Determine the characteristics of
  // the image.
  imageIO->SetFileName( m_FileName.c_str() );
  imageIO->ReadImageInformation();

  unsigned int ndim = imageIO->GetNumberOfDimensions();
  if ( ndim < MINDIM || ndim > MAXDIM )
  {
    itkWarningMacro( << "Sorry, only dimensions 2, 3 and 4 are supported. The given file has " << ndim << " dimensions! Reading as 4D." );
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

  MITK_INFO("mitkItkImageFileReader") << "ioRegion: " << ioRegion << std::endl;
  imageIO->SetIORegion( ioRegion );
  void* buffer = new unsigned char[imageIO->GetImageSizeInBytes()];
  imageIO->Read( buffer );

  image->Initialize( MakePixelType(imageIO), ndim, dimensions );
  image->SetImportVolume( buffer, 0, Image::ManageMemory );

  // access direction of itk::Image and include spacing
  mitk::Matrix3D matrix;
  matrix.SetIdentity();
  unsigned int j, itkDimMax3 = (ndim >= 3? 3 : ndim);
  for ( i=0; i < itkDimMax3; ++i)
    for( j=0; j < itkDimMax3; ++j )
      matrix[i][j] = imageIO->GetDirection(j)[i];

  // re-initialize PlaneGeometry with origin and direction
  PlaneGeometry* planeGeometry = static_cast<PlaneGeometry*>(image->GetSlicedGeometry(0)->GetPlaneGeometry(0));
  planeGeometry->SetOrigin(origin);
  planeGeometry->GetIndexToWorldTransform()->SetMatrix(matrix);

  // re-initialize SlicedGeometry3D
  SlicedGeometry3D* slicedGeometry = image->GetSlicedGeometry(0);
  slicedGeometry->InitializeEvenlySpaced(planeGeometry, image->GetDimension(2));
  slicedGeometry->SetSpacing(spacing);

  MITK_INFO("mitkItkImageFileReader") << slicedGeometry->GetCornerPoint(false,false,false);
  MITK_INFO("mitkItkImageFileReader") << slicedGeometry->GetCornerPoint(true,true,true);

  // re-initialize TimeGeometry
  ProportionalTimeGeometry::Pointer timeGeometry = ProportionalTimeGeometry::New();
  timeGeometry->Initialize(slicedGeometry, image->GetDimension(3));
  image->SetTimeGeometry(timeGeometry);

  buffer = nullptr;
  MITK_INFO("mitkItkImageFileReader") << "number of image components: "<< image->GetPixelType().GetNumberOfComponents() << std::endl;
//  mitk::DataNode::Pointer node = this->GetOutput();
//  node->SetData( image );

  // add level-window property
  //if ( image->GetPixelType().GetNumberOfComponents() == 1 )
  //{
  //  SetDefaultImageProperties( node );
  //}
  MITK_INFO("mitkItkImageFileReader") << "...finished!" << std::endl;
}


bool mitk::ItkImageFileReader::CanReadFile(const std::string filename, const std::string filePrefix, const std::string filePattern)
{
  // First check the extension
  if(  filename == "" )
    return false;

  // check if image is serie
  if( filePattern != "" && filePrefix != "" )
    return false;

  itk::ImageIOBase::Pointer imageIO = itk::ImageIOFactory::CreateImageIO( filename.c_str(), itk::ImageIOFactory::ReadMode );
  if ( imageIO.IsNull() )
    return false;

  try
  {
    imageIO->SetFileName( filename.c_str() );
    imageIO->ReadImageInformation();
    itk::MetaDataDictionary imgMetaDictionary = imageIO->GetMetaDataDictionary();
    std::vector<std::string> imgMetaKeys = imgMetaDictionary.GetKeys();
    std::vector<std::string>::const_iterator itKey = imgMetaKeys.begin();
    std::string metaString;

    for (; itKey != imgMetaKeys.end(); itKey ++)
    {
      itk::ExposeMetaData<std::string> (imgMetaDictionary, *itKey, metaString);
      if (itKey->find("modality") != std::string::npos)
      {
        if (metaString.find("DWMRI") != std::string::npos)
        {
          return false; // DiffusionImageReader should handle this
        }
      }
    }
  }catch(...)
  {
    MITK_INFO("mitkItkImageFileReader") << "Could not read ImageInformation ";
  }

  return true;
}

mitk::ItkImageFileReader::ItkImageFileReader()
    : m_FileName(""), m_FilePrefix(""), m_FilePattern("")
{
}

mitk::ItkImageFileReader::~ItkImageFileReader()
{
}
