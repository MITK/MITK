
#ifndef __itkDiffusionTensorPrincipleDirectionImageFilter_txx
#define __itkDiffusionTensorPrincipleDirectionImageFilter_txx

#include <time.h>
#include <stdio.h>
#include <stdlib.h>

//#include "itkDiffusionTensorPrincipleDirectionImageFilter.h"
#include "itkImageRegionConstIterator.h"
#include "itkImageRegionConstIteratorWithIndex.h"
#include "itkImageRegionIterator.h"
#include "itkArray.h"
#include "vnl/vnl_vector.h"

#define _USE_MATH_DEFINES
#include <math.h>

namespace itk {

  //#define QBALL_RECON_PI       M_PI

  template< class TTensorPixelType, class TPDPixelType>
  DiffusionTensorPrincipleDirectionImageFilter< TTensorPixelType, 
    TPDPixelType>
    ::DiffusionTensorPrincipleDirectionImageFilter()
  {
    // At least 1 inputs is necessary for a vector image.
    // For images added one at a time we need at least six
    this->SetNumberOfRequiredInputs( 1 );
  }

  template< class TTensorPixelType, 
  class TPDPixelType>
    void DiffusionTensorPrincipleDirectionImageFilter< TTensorPixelType, 
    TPDPixelType>
    ::BeforeThreadedGenerateData()
  {
  }

  template< class TTensorPixelType, 
  class TPDPixelType>
    void DiffusionTensorPrincipleDirectionImageFilter< TTensorPixelType, 
    TPDPixelType>
    ::ThreadedGenerateData(const OutputImageRegionType& outputRegionForThread,
    int ) 
  {

    typedef itk::DiffusionTensor3D<TTensorPixelType> TensorType;
    typedef ImageRegionConstIterator< InputImageType > InputIteratorType;
    typedef typename InputImageType::PixelType         InputTensorType;
    typename InputImageType::Pointer inputImagePointer = NULL;
    inputImagePointer = static_cast< InputImageType * >( 
      this->ProcessObject::GetInput(0) );

    typename OutputImageType::Pointer outputImage =
      static_cast< OutputImageType * >(this->ProcessObject::GetOutput(0));
    outputImage->SetSpacing( inputImagePointer->GetSpacing() );   // Set the image spacing
    outputImage->SetOrigin( inputImagePointer->GetOrigin() );     // Set the image origin
    outputImage->SetDirection( inputImagePointer->GetDirection() );  // Set the image direction
    outputImage->SetRegions( inputImagePointer->GetLargestPossibleRegion() );
    outputImage->Allocate();

    ImageRegionIterator< OutputImageType > oit(outputImage, outputRegionForThread);
    oit.GoToBegin();

    InputIteratorType git(inputImagePointer, outputRegionForThread );
    git.GoToBegin();
    while( !git.IsAtEnd() )
    {
      InputTensorType b = git.Get();
      TensorType tensor = b.GetDataPointer();

      typename OutputImageType::PixelType dir;
      typename TensorType::EigenValuesArrayType eigenvalues;
      typename TensorType::EigenVectorsMatrixType eigenvectors;
      if(tensor.GetTrace()!=0)
      {
        tensor.ComputeEigenAnalysis(eigenvalues, eigenvectors);

//        int index = 2;
//        if( (eigenvalues[0] >= eigenvalues[1])
//          && (eigenvalues[0] >= eigenvalues[2]) )
//          index = 0;
//        else if(eigenvalues[1] >= eigenvalues[2])
//          index = 1;

        vnl_vector_fixed<double,3> vec;
        vec[0] = eigenvectors(/*index*/2,0);
        vec[1] = eigenvectors(/*index*/2,1);
        vec[2] = eigenvectors(/*index*/2,2);

        dir[0] = (TPDPixelType)vec[0];
        dir[1] = (TPDPixelType)vec[1];
        dir[2] = (TPDPixelType)vec[2];
      }
      else
      {
        dir[0] = (TPDPixelType)0;
        dir[1] = (TPDPixelType)0;
        dir[2] = (TPDPixelType)0;
      }
      oit.Set( dir );

      ++oit;
      ++git; // Gradient  image iterator
    }

    std::cout << "One Thread finished extraction" << std::endl;
  }

  template< class TTensorPixelType, 
  class TPDPixelType>
    void DiffusionTensorPrincipleDirectionImageFilter< TTensorPixelType, 
    TPDPixelType>
    ::PrintSelf(std::ostream& os, Indent indent) const
  {
  }

}
#endif // __itkDiffusionQballPrincipleDirectionsImageFilter_txx
