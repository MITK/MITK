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
#ifndef __itkFieldmapGeneratorFilter_h__
#define __itkFieldmapGeneratorFilter_h__

#include <itkImageSource.h>
#include <itkImage.h>
#include <itkImageRegion.h>
#include <itkVector.h>
#include <mitkNumericTypes.h>
#include <itkMatrix.h>
#include <vector>

namespace itk{

/**
* \brief Generate float image with artificial frequency maps used by Fiberfox. Simulates additional frequencies at (possibly multiple) positions based on 3D gaussians with the specified variance and amplitude and/or as a linear gradient in the image.
* See "Fiberfox: Facilitating the creation of realistic white matter software phantoms" (DOI: 10.1002/mrm.25045) for details.
*/

template< class OutputImageType >
class FieldmapGeneratorFilter : public ImageSource< OutputImageType >
{

public:
    typedef FieldmapGeneratorFilter Self;
    typedef ProcessObject Superclass;
    typedef SmartPointer< Self > Pointer;
    typedef SmartPointer< const Self > ConstPointer;

    typedef typename OutputImageType::PixelType     PixelType;
    typedef typename OutputImageType::IndexType     IndexType;
    typedef itk::ImageRegion<3>                     OutputImageRegionType;
    typedef itk::Matrix<double, 3, 3>               MatrixType;

    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)
    itkTypeMacro( FieldmapGeneratorFilter, ImageSource )

    /** Output image parameters. */
    itkSetMacro( Spacing, itk::Vector<double> )
    itkSetMacro( Origin, mitk::Point3D )
    itkSetMacro( DirectionMatrix, MatrixType )
    itkSetMacro( ImageRegion, OutputImageRegionType )

    /** Gradient direction and offset. */
    void SetGradient( vnl_vector_fixed< double, 3 > gradient ) { m_Gradient=gradient; }
    void SetOffset( vnl_vector_fixed< double, 3 > offset ) { m_Offset=offset; }

    /** Parameters of gaussian frequency sources. */
    void SetVariances( std::vector< double > variances ) { m_Variances=variances; }
    void SetHeights( std::vector< double > heights ) { m_Heights=heights; }
    void SetWorldPositions( std::vector< mitk::Point3D > worldPositions ) { m_WorldPositions=worldPositions; }

protected:

    void BeforeThreadedGenerateData();
    void ThreadedGenerateData( const OutputImageRegionType &outputRegionForThread, ThreadIdType threadId);

    FieldmapGeneratorFilter();
    virtual ~FieldmapGeneratorFilter();

    itk::Vector<double>                 m_Spacing;              ///< output image spacing
    mitk::Point3D                       m_Origin;               ///< output image origin
    MatrixType                          m_DirectionMatrix;      ///< output image rotation
    OutputImageRegionType               m_ImageRegion;          ///< output image size
    std::vector< double >               m_Variances;
    std::vector< double >               m_Heights;
    std::vector< mitk::Point3D >        m_WorldPositions;
    vnl_vector_fixed< double, 3 >       m_Gradient;
    vnl_vector_fixed< double, 3 >       m_Offset;
};

}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkFieldmapGeneratorFilter.cpp"
#endif

#endif // __itkFieldmapGeneratorFilter_h__
