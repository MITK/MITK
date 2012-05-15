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

/*===================================================================

This file is based heavily on a corresponding ITK filter.

===================================================================*/
#ifndef __itkDiffusionTensorPrincipleDirectionImageFilter_h_
#define __itkDiffusionTensorPrincipleDirectionImageFilter_h_

#include "MitkDiffusionImagingExports.h"
#include "itkImageToImageFilter.h"
//#include "vnl/vnl_matrix.h"
#include "vnl/vnl_vector_fixed.h"
#include "vnl/vnl_matrix.h"
#include "vnl/algo/vnl_svd.h"
#include "itkVectorContainer.h"
#include "itkVectorImage.h"
#include "itkDiffusionTensor3D.h"

namespace itk{
  /** \class DiffusionTensorPrincipleDirectionImageFilter
 */

  template< class TTensorPixelType, class TPDPixelType=double>
  class DiffusionTensorPrincipleDirectionImageFilter :
      public ImageToImageFilter< Image< DiffusionTensor3D<TTensorPixelType>, 3 >,
      Image< Vector< TPDPixelType, 3 >, 3 > >
  {

  public:

    typedef DiffusionTensorPrincipleDirectionImageFilter Self;
    typedef SmartPointer<Self>                      Pointer;
    typedef SmartPointer<const Self>                ConstPointer;
    typedef ImageToImageFilter< Image< DiffusionTensor3D<TTensorPixelType>, 3 >,
    Image< Vector< TPDPixelType, 3 >, 3 > >
    Superclass;

    /** Method for creation through the object factory. */
    itkNewMacro(Self);

    /** Runtime information support. */
    itkTypeMacro(DiffusionTensorPrincipleDirectionImageFilter,
                 ImageToImageFilter);

    typedef TTensorPixelType                 TensorComponentType;

    typedef TPDPixelType                  DirectionPixelType;

    typedef typename Superclass::InputImageType      InputImageType;

    typedef typename Superclass::OutputImageType      OutputImageType;

    typedef typename Superclass::OutputImageRegionType
        OutputImageRegionType;

    void SetImage( const InputImageType *image );

  protected:
    DiffusionTensorPrincipleDirectionImageFilter();
    ~DiffusionTensorPrincipleDirectionImageFilter() {};
    void PrintSelf(std::ostream& os, Indent indent) const;

    void BeforeThreadedGenerateData();
    void ThreadedGenerateData( const
                               OutputImageRegionType &outputRegionForThread, int);

  private:

  };

}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkDiffusionTensorPrincipleDirectionImageFilter.txx"
#endif

#endif //__itkDiffusionTensorPrincipleDirectionImageFilter_h_

