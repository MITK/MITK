/*=========================================================================

Program:   Insight Segmentation & Registration Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) 2002 Insight Consortium. All rights reserved.
See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

Portions of this code are covered under the VTK copyright.
See VTKCopyright.txt or http://www.kitware.com/VTKCopyright.htm for details.

This software is distributed WITHOUT ANY WARRANTY; without even 
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
PURPOSE.  See the above copyright notices for more information.
      
=========================================================================*/
#ifndef IMAGETOITK_TXX_INCLUDED_C1C2FCD2
#define IMAGETOITK_TXX_INCLUDED_C1C2FCD2

#include "mitkImageToItk.h"

namespace mitk
{

template<class TPixel, unsigned int VImageDimension>
  ImageToItk<TPixel, VImageDimension>
  ::ImageToItk() 
    : itk::ImageSource< itk::Image<TPixel, VImageDimension> >(), 
        m_CopyMemFlag(false), m_Channel(0)
{
  
}


template<class TPixel, unsigned int VImageDimension>
  ImageToItk<TPixel, VImageDimension>
  ::~ImageToItk()
{
  
}

template<class TPixel, unsigned int VImageDimension>
void ImageToItk<TPixel, VImageDimension>::SetInput(const mitk::Image *input)
{
  if(input == NULL)
    itkExceptionMacro( << "image is null" );
  if(input->GetDimension()!=VImageDimension)
    itkExceptionMacro( << "image has dimension " << input->GetDimension() << " instead of " << VImageDimension );
  
  
  if(typeid(TPixel) != *input->GetPixelType().GetTypeId())
    itkExceptionMacro( << "image has wrong pixel type " );
  
  // Process object is not const-correct so the const_cast is required here
  this->ProcessObject::SetNthInput(0, 
    const_cast< mitk::Image * >( input ) );
}

template<class TPixel, unsigned int VImageDimension>
void ImageToItk<TPixel, VImageDimension>::SetInput( unsigned int index, const mitk::Image * input ) 
{
  if( index+1 > this->GetNumberOfInputs() )
  {
    this->SetNumberOfRequiredInputs( index + 1 );
  }

  if(input == NULL)
    itkExceptionMacro( << "image is null" );
  if(input->GetDimension()!=VImageDimension)
    itkExceptionMacro( << "image has dimension " << input->GetDimension() << " instead of " << VImageDimension );
  
  
  if(typeid(TPixel) != *input->GetPixelType().GetTypeId())
    itkExceptionMacro( << "image has wrong pixel type " );

  // Process object is not const-correct so the const_cast is required here
  this->ProcessObject::SetNthInput(index, 
    const_cast< mitk::Image *>( input ) );
}

template<class TPixel, unsigned int VImageDimension>
const mitk::Image *ImageToItk<TPixel, VImageDimension>::GetInput(void) 
{
	if (this->GetNumberOfInputs() < 1)
	{
		return 0;
	}

	return static_cast< const mitk::Image * >
		(this->ProcessObject::GetInput(0) );
}

template<class TPixel, unsigned int VImageDimension>
const mitk::Image *ImageToItk<TPixel, VImageDimension>::GetInput(unsigned int idx)
{
	return static_cast< mitk::Image * >
		(this->ProcessObject::GetInput(idx));
}


/** set the @a mitk::Image that shall be accessed from itk via this itk-wrapper-class.
* @param n channel number to be accessed
*
template<class TPixel, unsigned int VImageDimension>
  void ImageToItk<TPixel, VImageDimension>
  ::SetMitkImage(mitk::Image::Pointer image, int n)
{
  
  std::cout << "ImageToItk::SetMitkImage() ... " << std::endl;
  
  if(image == NULL)
    itkExceptionMacro( << "image is null" );
  if(image->GetDimension()!=VImageDimension)
    itkExceptionMacro( << "image has dimension " << image->GetDimension() << " instead of " << VImageDimension );
  
  
  if(typeid(TPixel) != *image->GetPixelType().GetTypeId())
    itkExceptionMacro( << "image has wrong pixel type " );
  
  itkDebugMacro("ImageToItk::SetImageToItk ...");
  
  std::cout << "Hallo..  " << std::endl;    
  m_MitkImage = image;
  
  m_Channel = n;
  
  
  this->Modified();
  
  
  itkDebugMacro( << "ImageToItk::SetMitkImage OK!" );
  std::cout << "ImageToItk::SetMitkImage() OK! " << std::endl;
  
}*/

template<class TPixel, unsigned int VImageDimension>
  void ImageToItk<TPixel, VImageDimension>
  ::SetCopyMemFlagOn()
{
  m_CopyMemFlag = true;
}

template<class TPixel, unsigned int VImageDimension>
  void ImageToItk<TPixel, VImageDimension>
  ::SetCopyMemFlagOff()
{
  m_CopyMemFlag = false;
}

template<class TPixel, unsigned int VImageDimension>
  void ImageToItk<TPixel, VImageDimension>
  ::SetCopyMemFlag(bool flag)
{
  m_CopyMemFlag = flag;
}


template<class TPixel, unsigned int VImageDimension>
  void ImageToItk<TPixel, VImageDimension>
  ::GenerateData()
{
  
  std::cout << "ImageToItk<TPixel, VImageDimension>::GenerateData() ... " << std::endl;
  
  
  // Allocate output
  mitk::Image::ConstPointer input = this->GetInput();
  typename OutputImageType::Pointer output = this->GetOutput();
  
  
  unsigned long noBytes = input->GetDimension(0);
  for (int i=1; i<VImageDimension; ++i)
  {
    noBytes = noBytes * input->GetDimension(i);
  }
  
  if (m_CopyMemFlag)
  {
    itkDebugMacro("copyMem ...");
    
    
    output->Allocate();
    
    // hier wird momentan wohl nur der erste Channel verwendet??!!
    m_ImageDataItem = const_cast<mitk::Image*>(input.GetPointer())->GetChannelData( m_Channel );
    memcpy( (PixelType *) output->GetBufferPointer(), m_ImageDataItem->GetData(), sizeof(PixelType)*noBytes);
    
  }
  else
  {
    m_ImageDataItem = const_cast<mitk::Image*>(input.GetPointer())->GetChannelData( m_Channel );
    
    itkDebugMacro("do not copyMem ...");
    typedef itk::ImportImageContainer< unsigned long, TPixel >   ImportContainerType;
    ImportContainerType::Pointer import;
    
    import = ImportContainerType::New();
    import->Initialize();
    
    itkDebugMacro( << "size of container = " << import->Size() );
    import->SetImportPointer( (TPixel*) m_ImageDataItem->GetData(),	noBytes, false);
    
    output->SetPixelContainer(import);
    //output->Update();
    itkDebugMacro( << "size of container = " << import->Size() );
  }
  
  std::cout << "ImageToItk<TPixel, VImageDimension>::GenerateData() OK! " << std::endl;
  
  
}

template<class TPixel, unsigned int VImageDimension>
  void ImageToItk<TPixel, VImageDimension>
  ::GenerateOutputInformation()
{
  std::cout << "ImageToItk<TPixel, VImageDimension>::GenerateOutputInformation() ... " << std::endl;
  
  mitk::Image::ConstPointer input = this->GetInput();
  typename OutputImageType::Pointer output = this->GetOutput();
  
  SizeType  size;
  double origin[ VImageDimension ];
  double spacing[ VImageDimension ];
  
  for (int i=0; i < VImageDimension; ++i)
  {
    size[i]    = input->GetDimension(i);
    spacing[i] = input->GetGeometry()->GetSpacing()[i];
  }
  
  mitk::Point3D mitkorigin(0,0,0);
  input->GetGeometry()->UnitsToMM(mitkorigin, mitkorigin);
  origin[0] = mitkorigin.x;    // X coordinate
  origin[1] = mitkorigin.y;    // Y coordinate
  origin[2] = mitkorigin.z;    // Z coordinate
  
  IndexType start;
  start.Fill( 0 );
  
  RegionType region;
  region.SetIndex( start );
  region.SetSize(  size  );
  
  output->SetRegions( region );
  output->SetOrigin( origin );
  output->SetSpacing( spacing );
  
  std::cout << "ImageToItk<TPixel, VImageDimension>::GenerateOutputInformation() OK! " << std::endl;
  
}

template<class TPixel, unsigned int VImageDimension>
  void
  ImageToItk<TPixel, VImageDimension>
  ::PrintSelf(std::ostream& os, itk::Indent indent) const
{
  Superclass::PrintSelf(os,indent);
  
  std::cout << "Mit2itkWrapper .. " << std::endl;
}

}
#endif //IMAGETOITK_TXX_INCLUDED_C1C2FCD2
