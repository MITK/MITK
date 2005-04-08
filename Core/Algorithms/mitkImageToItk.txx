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


template <class TOutputImage>
void mitk::ImageToItk<TOutputImage>::SetInput(const mitk::Image *input)
{
  if(input == NULL)
    itkExceptionMacro( << "image is null" );
  if(input->GetDimension()!=TOutputImage::GetImageDimension())
    itkExceptionMacro( << "image has dimension " << input->GetDimension() << " instead of " << TOutputImage::GetImageDimension() );
  
  
  if(typeid(PixelType) != *input->GetPixelType().GetTypeId())
    itkExceptionMacro( << "image has wrong pixel type " );
  
  // Process object is not const-correct so the const_cast is required here
  this->ProcessObject::SetNthInput(0, 
    const_cast< mitk::Image * >( input ) );
}

template<class TOutputImage>
void mitk::ImageToItk<TOutputImage>::SetInput( unsigned int index, const mitk::Image * input ) 
{
  if( index+1 > this->GetNumberOfInputs() )
  {
    this->SetNumberOfRequiredInputs( index + 1 );
  }

  if(input == NULL)
    itkExceptionMacro( << "image is null" );
  if(input->GetDimension()!=TOutputImage::GetImageDimension())
    itkExceptionMacro( << "image has dimension " << input->GetDimension() << " instead of " << TOutputImage::GetImageDimension() );
  
  
  if(typeid(PixelType) != *input->GetPixelType().GetTypeId())
    itkExceptionMacro( << "image has wrong pixel type " );

  // Process object is not const-correct so the const_cast is required here
  this->ProcessObject::SetNthInput(index, 
    const_cast< mitk::Image *>( input ) );
}

template<class TOutputImage>
const mitk::Image *mitk::ImageToItk<TOutputImage>::GetInput(void) 
{
	if (this->GetNumberOfInputs() < 1)
	{
		return 0;
	}

	return static_cast< const mitk::Image * >
		(this->ProcessObject::GetInput(0) );
}

template<class TOutputImage>
const mitk::Image *mitk::ImageToItk<TOutputImage>::GetInput(unsigned int idx)
{
	return static_cast< mitk::Image * >
		(this->ProcessObject::GetInput(idx));
}

template<class TOutputImage>
  void mitk::ImageToItk<TOutputImage>
  ::GenerateData()
{
  // Allocate output
  mitk::Image::ConstPointer input = this->GetInput();
  typename OutputImageType::Pointer output = this->GetOutput();
  
  
  unsigned long noBytes = input->GetDimension(0);
  for (unsigned int i=1; i<TOutputImage::GetImageDimension(); ++i)
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
    typedef itk::ImportImageContainer< unsigned long, PixelType >   ImportContainerType;
    typename ImportContainerType::Pointer import;
    
    import = ImportContainerType::New();
    import->Initialize();
    
    itkDebugMacro( << "size of container = " << import->Size() );
    import->SetImportPointer( (PixelType*) m_ImageDataItem->GetData(),	noBytes, false);
    
    output->SetPixelContainer(import);
    itkDebugMacro( << "size of container = " << import->Size() );
  }
}

template<class TOutputImage>
  void mitk::ImageToItk<TOutputImage>
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

template<class TOutputImage>
  void mitk::ImageToItk<TOutputImage>
  ::GenerateOutputInformation()
{
  mitk::Image::ConstPointer input = this->GetInput();
  typename OutputImageType::Pointer output = this->GetOutput();
  
  SizeType  size;
  double origin[ 4 ];   // itk2vtk() expects 3 dimensions, so we can't use VImageDimension if image is 2d!
  double *spacing = new double[ TOutputImage::GetImageDimension() ];

  for (unsigned int i=0; i < TOutputImage::GetImageDimension(); ++i)
  {
    size[i]    = input->GetDimension(i);
    spacing[i] = input->GetSlicedGeometry()->GetSpacing()[i];
  }
  
  mitk::Point3D mitkorigin;
  mitkorigin.Fill(0.0);
  input->GetGeometry()->IndexToWorld(mitkorigin, mitkorigin);
  itk2vtk(mitkorigin, origin);
  
  IndexType start;
  start.Fill( 0 );
  
  RegionType region;
  region.SetIndex( start );
  region.SetSize(  size  );
  
  output->SetRegions( region );
  output->SetOrigin( origin );
  output->SetSpacing( spacing );
  delete [] spacing;
}

template<class TOutputImage>
  void
  mitk::ImageToItk<TOutputImage>
  ::PrintSelf(std::ostream& os, itk::Indent indent) const
{
  Superclass::PrintSelf(os,indent);
}

#endif //IMAGETOITK_TXX_INCLUDED_C1C2FCD2
