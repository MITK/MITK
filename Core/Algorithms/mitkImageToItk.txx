/*=========================================================================

  Program:   Medical Imaging & Interaction Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/ for details.

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

template<class TPixel, unsigned int VImageDimension>
  void ImageToItk<TPixel, VImageDimension>
  ::GenerateData()
{
  // Allocate output
  mitk::Image::ConstPointer input = this->GetInput();
  typename OutputImageType::Pointer output = this->GetOutput();
  
  
  unsigned long noBytes = input->GetDimension(0);
  for (unsigned int i=1; i<VImageDimension; ++i)
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
    typename ImportContainerType::Pointer import;
    
    import = ImportContainerType::New();
    import->Initialize();
    
    itkDebugMacro( << "size of container = " << import->Size() );
    import->SetImportPointer( (TPixel*) m_ImageDataItem->GetData(),	noBytes, false);
    
    output->SetPixelContainer(import);
    itkDebugMacro( << "size of container = " << import->Size() );
  }
}

template<class TPixel, unsigned int VImageDimension>
  void ImageToItk<TPixel, VImageDimension>
  ::UpdateOutputInformation()
{
  mitk::Image::ConstPointer input = this->GetInput();
  if(input.IsNotNull() && (input->GetSource()!=NULL) && input->GetSource()->Updating())
  {
    typename OutputImageType::Pointer output = this->GetOutput();
    unsigned long t1 = input->GetUpdateMTime()+1;
    if (t1 > m_OutputInformationMTime.GetMTime())
    {
      output->SetPipelineMTime(t1);

      this->GenerateOutputInformation();

      m_OutputInformationMTime.Modified();
    }
    return;
  }
  Superclass::UpdateOutputInformation();
}

template<class TPixel, unsigned int VImageDimension>
  void ImageToItk<TPixel, VImageDimension>
  ::GenerateOutputInformation()
{
  mitk::Image::ConstPointer input = this->GetInput();
  typename OutputImageType::Pointer output = this->GetOutput();
  
  SizeType  size;
  double origin[ VImageDimension ];
  double spacing[ VImageDimension ];
  
  for (unsigned int i=0; i < VImageDimension; ++i)
  {
    size[i]    = input->GetDimension(i);
    spacing[i] = input->GetSlicedGeometry()->GetSpacing()[i];
  }
  
  mitk::Point3D mitkorigin;
  mitkorigin.Fill(0.0);
  input->GetGeometry()->UnitsToMM(mitkorigin, mitkorigin);
  itk2vtk(mitkorigin, origin);
  
  IndexType start;
  start.Fill( 0 );
  
  RegionType region;
  region.SetIndex( start );
  region.SetSize(  size  );
  
  output->SetRegions( region );
  output->SetOrigin( origin );
  output->SetSpacing( spacing );
}

template<class TPixel, unsigned int VImageDimension>
  void
  ImageToItk<TPixel, VImageDimension>
  ::PrintSelf(std::ostream& os, itk::Indent indent) const
{
  Superclass::PrintSelf(os,indent);
}

}
#endif //IMAGETOITK_TXX_INCLUDED_C1C2FCD2
