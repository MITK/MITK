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

#ifndef IMAGETOITK_TXX_INCLUDED_C1C2FCD2
#define IMAGETOITK_TXX_INCLUDED_C1C2FCD2

#include "mitkImageToItk.h"
#include "mitkBaseProcess.h"
#include "itkImportMitkImageContainer.h"
#include "mitkImageReadAccessor.h"
#include "mitkImageWriteAccessor.h"
#include "mitkException.h"

template<typename TImageType>
struct SetLengthHelper
{
  SetLengthHelper(TImageType* in)
  { m_Image = in;}

private:
  TImageType* m_Image;
};

template< typename T, unsigned int VDimension>
struct SetLengthHelper< itk::Image< T, VDimension > >
{
  typedef itk::Image< T, VDimension > TImageType;

  SetLengthHelper(TImageType* in)
  { m_Image = in;}

  void SetVectorLength( size_t ) {}

private:
  TImageType* m_Image;
};

template< typename T, unsigned int VDimension>
struct SetLengthHelper< itk::VectorImage< T, VDimension > >
{
  typedef itk::VectorImage< T, VDimension > TImageType;

  SetLengthHelper(TImageType* in)
  { m_Image = in;}

  void SetVectorLength( size_t len) { m_Image->SetVectorLength( len ); }

private:
  TImageType* m_Image;
};

template <class TOutputImage>
void mitk::ImageToItk<TOutputImage>::SetInput(mitk::Image *input)
{
  this->SetInput(static_cast<const Image*>(input));
  m_ConstInput = false;
}

template <class TOutputImage>
void mitk::ImageToItk<TOutputImage>::SetInput(const mitk::Image* input)
{
  this->CheckInput(input);
  // Process object is not const-correct so the const_cast is required here
  itk::ProcessObject::PushFrontInput(input);
  m_ConstInput = true;
}

template<class TOutputImage>
mitk::Image *mitk::ImageToItk<TOutputImage>::GetInput(void)
{
  if (this->GetNumberOfInputs() < 1)
  {
    return 0;
  }

  return static_cast<mitk::Image*>(itk::ProcessObject::GetInput(0));
}

template<class TOutputImage>
const mitk::Image *mitk::ImageToItk<TOutputImage>::GetInput() const
{
  if (this->GetNumberOfInputs() < 1)
  {
    return 0;
  }

  return static_cast<const mitk::Image*>(itk::ProcessObject::GetInput(0));
}

template<class TOutputImage>
  void mitk::ImageToItk<TOutputImage>
  ::GenerateData()
{
  // Allocate output
  mitk::Image::Pointer input = this->GetInput();
  typename Superclass::OutputImageType::Pointer output = this->GetOutput();


  unsigned long noBytes = input->GetDimension(0);
  for (unsigned int i=1; i<TOutputImage::GetImageDimension(); ++i)
  {
    noBytes = noBytes * input->GetDimension(i);
  }
  const mitk::PixelType pixelType = input->GetPixelType();
  if (pixelType.GetPixelType() == itk::ImageIOBase::VECTOR)
  {
    noBytes *= pixelType.GetNumberOfComponents();
    SetLengthHelper<typename Superclass::OutputImageType> helper( output.GetPointer() );
    helper.SetVectorLength( pixelType.GetNumberOfComponents() );
  }

  mitk::ImageAccessorBase* imageAccess;
  if (m_ConstInput)
  {
    imageAccess = new mitk::ImageReadAccessor(input, static_cast<const ImageDataItem*>(NULL), m_Options);
  }
  else
  {
    imageAccess = new mitk::ImageWriteAccessor(input, static_cast<const ImageDataItem*>(NULL), m_Options);
  }

  // hier wird momentan wohl nur der erste Channel verwendet??!!
  if(imageAccess->GetData() == NULL)
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

    memcpy(output->GetBufferPointer(), imageAccess->GetData(), sizeof(InternalPixelType)*noBytes);

    delete imageAccess;
  }
  else
  {
    itkDebugMacro("do not copyMem ...");
    typedef itk::ImportMitkImageContainer< unsigned long, InternalPixelType >   ImportContainerType;
    typename ImportContainerType::Pointer import;

    import = ImportContainerType::New();
    import->Initialize();

    itkDebugMacro( << "size of container = " << import->Size() );
    //import->SetImageDataItem(m_ImageDataItem);
    import->SetImageAccessor(imageAccess,sizeof(InternalPixelType)*noBytes);

     output->SetPixelContainer(import);
    itkDebugMacro( << "size of container = " << import->Size() );
  }
}

template<class TOutputImage>
  void mitk::ImageToItk<TOutputImage>
  ::UpdateOutputInformation()
{
  mitk::Image::Pointer input = this->GetInput();
  if(input.IsNotNull() && (input->GetSource().IsNotNull()) && input->GetSource()->Updating())
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

  /// \warning 2D MITK images could have a 3D rotation, since they have a 3x3 geometry matrix.
  /// If it is only a rotation around the axial plane normal, it can be express with a 2x2 matrix.
  /// In this case, the ITK image conservs this information and is identical to the MITK image!
  /// If the MITK image contains any other rotation, the ITK image will have no rotation at all.
  /// Spacing is of course conserved in both cases.


  // the following loop devides by spacing now to normalize columns.
  // counterpart of InitializeByItk in mitkImage.h line 372 of revision 15092.

  // Check if information is lost
  if (  TOutputImage::ImageDimension <= 2)
  {
     if (( TOutputImage::ImageDimension == 2) && (
           ( matrix[0][2] != 0) ||
           ( matrix[1][2] != 0) ||
           ( matrix[2][0] != 0) ||
           ( matrix[2][1] != 0) ||
           (( matrix[2][2] != 1) &&  ( matrix[2][2] != -1) )))
     {
        // The 2D MITK image contains 3D rotation information.
        // This cannot be expressed in a 2D ITK image, so the ITK image will have no rotation

     }
     else
     {
        // The 2D MITK image can be converted to an 2D ITK image without information loss!
        for ( i=0; i < itkDimMax3; ++i)
           for( j=0; j < itkDimMax3; ++j )
              direction[i][j] = matrix[i][j]/spacing[j];
     }
  }
  else
  {
     // Normal 3D image. Conversion possible without problem!
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
void mitk::ImageToItk<TOutputImage>::CheckInput(const mitk::Image* input) const
{
  if(input == NULL)
  {
    itkExceptionMacro( << "image is null" );
  }
  if(input->GetDimension()!=TOutputImage::GetImageDimension())
  {
    itkExceptionMacro( << "image has dimension " << input->GetDimension() << " instead of " << TOutputImage::GetImageDimension() );
  }

  if(!(input->GetPixelType() == mitk::MakePixelType<TOutputImage>(input->GetPixelType().GetNumberOfComponents())))
  {
    itkExceptionMacro( << "image has wrong pixel type " );
  }
}

template<class TOutputImage>
  void
  mitk::ImageToItk<TOutputImage>
  ::PrintSelf(std::ostream& os, itk::Indent indent) const
{
  Superclass::PrintSelf(os,indent);
}

#endif //IMAGETOITK_TXX_INCLUDED_C1C2FCD2
