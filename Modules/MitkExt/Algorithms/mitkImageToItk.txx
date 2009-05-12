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
#include "mitkBaseProcess.h"
#include "itkImportMitkImageContainer.h"
#include <itkSmartPointerForwardReference.txx>


template <class TOutputImage>
void mitk::ImageToItk<TOutputImage>::SetInput(const mitk::Image *input)
{
  if(input == NULL)
    itkExceptionMacro( << "image is null" );
  if(input->GetDimension()!=TOutputImage::GetImageDimension())
    itkExceptionMacro( << "image has dimension " << input->GetDimension() << " instead of " << TOutputImage::GetImageDimension() );
  
  
  if(!(input->GetPixelType() == typeid(PixelType)))
    itkExceptionMacro( << "image has wrong pixel type " );
  
  // Process object is not const-correct so the const_cast is required here
  itk::ProcessObject::SetNthInput(0, 
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
  
  
  if(!(input->GetPixelType() == typeid(PixelType)))
    itkExceptionMacro( << "image has wrong pixel type " );

  // Process object is not const-correct so the const_cast is required here
  itk::ProcessObject::SetNthInput(index, 
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
    (itk::ProcessObject::GetInput(0) );
}

template<class TOutputImage>
const mitk::Image *mitk::ImageToItk<TOutputImage>::GetInput(unsigned int idx)
{
  return static_cast< mitk::Image * >
    (itk::ProcessObject::GetInput(idx));
}

template<class TOutputImage>
  void mitk::ImageToItk<TOutputImage>
  ::GenerateData()
{
  // Allocate output
  mitk::Image::ConstPointer input = this->GetInput();
  typename Superclass::OutputImageType::Pointer output = this->GetOutput();
  
  
  unsigned long noBytes = input->GetDimension(0);
  for (unsigned int i=1; i<TOutputImage::GetImageDimension(); ++i)
  {
    noBytes = noBytes * input->GetDimension(i);
  }
  
  // hier wird momentan wohl nur der erste Channel verwendet??!!
  m_ImageDataItem = const_cast<mitk::Image*>(input.GetPointer())->GetChannelData( m_Channel );
  if(m_ImageDataItem.GetPointer() == NULL)
  {
    itkWarningMacro(<< "no image data to import in ITK image");

    RegionType bufferedRegion;
    output->SetBufferedRegion(bufferedRegion);
    return;
  }
  
  if (m_CopyMemFlag)
  {
    itkDebugMacro("copyMem ...");

    output->Allocate();
    
    memcpy( (PixelType *) output->GetBufferPointer(), m_ImageDataItem->GetData(), sizeof(PixelType)*noBytes);
    
  }
  else
  {
    itkDebugMacro("do not copyMem ...");
    typedef itk::ImportMitkImageContainer< unsigned long, PixelType >   ImportContainerType;
    typename ImportContainerType::Pointer import;

    import = ImportContainerType::New();
    import->Initialize();

    itkDebugMacro( << "size of container = " << import->Size() );
    import->SetImageDataItem(m_ImageDataItem);

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
    typename Superclass::OutputImageType::Pointer output = this->GetOutput();
    unsigned long t1 = input->GetUpdateMTime()+1;
    if (t1 > this->m_OutputInformationMTime.GetMTime())
    {
      output->SetPipelineMTime(t1);

      this->GenerateOutputInformation();

      this->m_OutputInformationMTime.Modified();
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
  typename Superclass::OutputImageType::Pointer output = this->GetOutput();

  // allocate size, origin, spacing, direction in types of output image
  SizeType  size;
  const unsigned int itkDimMin3 = (TOutputImage::ImageDimension > 3 ? TOutputImage::ImageDimension : 3);
  const unsigned int itkDimMax3 = (TOutputImage::ImageDimension < 3 ? TOutputImage::ImageDimension : 3);
  typename Superclass::OutputImageType::PointType::ValueType origin[ itkDimMin3 ];   
  typename Superclass::OutputImageType::SpacingType::ComponentType spacing[ itkDimMin3 ];
  typename Superclass::OutputImageType::DirectionType direction;

  // copy as much information as possible into size and spacing
  unsigned int i;
  for ( i=0; i < itkDimMax3; ++i)
  {
    size[i]    = input->GetDimension(i);
    spacing[i] = input->GetGeometry()->GetSpacing()[i];
  }
  for ( ; i < TOutputImage::ImageDimension; ++i)
  {
    origin[i]  = 0.0;
    size[i]    = input->GetDimension(i);
    spacing[i] = 1.0;
  }

  // build region from size  
  IndexType start;
  start.Fill( 0 );
  RegionType region;
  region.SetIndex( start );
  region.SetSize(  size  );
  
  // copy as much information as possible into origin
  const mitk::Point3D& mitkorigin = input->GetGeometry()->GetOrigin();
  itk2vtk(mitkorigin, origin);
  
  // copy as much information as possible into direction
  direction.SetIdentity();
  unsigned int j;
  const AffineTransform3D::MatrixType& matrix = input->GetGeometry()->GetIndexToWorldTransform()->GetMatrix();
  
  /// \warning 2D MITK images will get a 2D identity matrix in ITK
  /// \todo Get clear about how to handle directed ITK 2D images in ITK
  
  // the following loop devides by spacing now to normalize columns.
  // counterpart of InitializeByItk in mitkImage.h line 372 of revision 15092. 
  if ( itkDimMax3 >= 3)
  {
    for ( i=0; i < itkDimMax3; ++i)
      for( j=0; j < itkDimMax3; ++j )
        direction[i][j] = matrix[i][j]/spacing[j];
  }

  // set information into output image
  output->SetRegions( region );
  output->SetOrigin( origin );
  output->SetSpacing( spacing );
  output->SetDirection( direction );
}

template<class TOutputImage>
  void
  mitk::ImageToItk<TOutputImage>
  ::PrintSelf(std::ostream& os, itk::Indent indent) const
{
  Superclass::PrintSelf(os,indent);
}

#endif //IMAGETOITK_TXX_INCLUDED_C1C2FCD2
