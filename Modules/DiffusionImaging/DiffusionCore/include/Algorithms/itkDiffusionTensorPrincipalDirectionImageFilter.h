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
#ifndef __itkDiffusionTensorPrincipalDirectionImageFilter_h_
#define __itkDiffusionTensorPrincipalDirectionImageFilter_h_

#include <MitkDiffusionCoreExports.h>
#include <itkImageToImageFilter.h>
#include <vnl/vnl_vector_fixed.h>
#include <vnl/vnl_matrix.h>
#include <vnl/algo/vnl_svd.h>
#include <itkVectorContainer.h>
#include <itkVectorImage.h>
#include <itkDiffusionTensor3D.h>
#include <mitkFiberBundle.h>

namespace itk{
/** \brief Extracts principal eigenvectors of the input tensors
 */

template< class TTensorPixelType>
class DiffusionTensorPrincipalDirectionImageFilter : public ImageToImageFilter< Image< DiffusionTensor3D<TTensorPixelType>, 3 >, Image< unsigned char, 3 > >
{

public:

    typedef DiffusionTensorPrincipalDirectionImageFilter Self;
    typedef SmartPointer<Self>                      Pointer;
    typedef SmartPointer<const Self>                ConstPointer;
    typedef ImageToImageFilter< Image< DiffusionTensor3D<TTensorPixelType>, 3 >, Image< unsigned char, 3 > >
    Superclass;

    /** Method for creation through the object factory. */
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    /** Runtime information support. */
    itkTypeMacro(DiffusionTensorPrincipalDirectionImageFilter, ImageToImageFilter)

    typedef TTensorPixelType                            TensorComponentType;
    typedef typename Superclass::InputImageType         InputImageType;
    typedef typename Superclass::OutputImageType        OutputImageType;
    typedef typename Superclass::OutputImageRegionType  OutputImageRegionType;
    typedef itk::Image<unsigned char, 3>                ItkUcharImgType;
    typedef vnl_vector_fixed< double, 3 >               DirectionType;
    typedef Image< float, 4 >                           PeakImageType;

    void SetImage( const InputImageType *image );

    // input
    itkSetMacro( MaskImage, ItkUcharImgType::Pointer)
    itkSetMacro( NormalizeVectors, bool)
    itkSetMacro( UsePolarCoordinates, bool)
    itkSetMacro( FaThreshold, float)

    // output
    itkGetMacro( PeakImage, PeakImageType::Pointer)

    protected:
        DiffusionTensorPrincipalDirectionImageFilter();
    ~DiffusionTensorPrincipalDirectionImageFilter() {}
    void PrintSelf(std::ostream& os, Indent indent) const;

    void BeforeThreadedGenerateData();
    void ThreadedGenerateData( const OutputImageRegionType &outputRegionForThread, ThreadIdType );
    void AfterThreadedGenerateData();

private:

    bool                                m_NormalizeVectors;     ///< Normalizes the output vector to length 1
    ItkUcharImgType::Pointer            m_MaskImage;            ///< Extraction is only performed inside of the binary mask
    PeakImageType::Pointer              m_PeakImage;
    bool                                m_UsePolarCoordinates;
    float                               m_FaThreshold;
};

}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkDiffusionTensorPrincipalDirectionImageFilter.txx"
#endif

#endif //__itkDiffusionTensorPrincipalDirectionImageFilter_h_

