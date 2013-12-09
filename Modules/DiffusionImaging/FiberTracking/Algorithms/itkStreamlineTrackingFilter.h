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
#ifndef __itkStreamlineTrackingFilter_h_
#define __itkStreamlineTrackingFilter_h_

#include "FiberTrackingExports.h"
#include <itkImageToImageFilter.h>
#include <itkVectorContainer.h>
#include <itkVectorImage.h>
#include <itkDiffusionTensor3D.h>
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkCellArray.h>
#include <vtkPoints.h>
#include <vtkPolyLine.h>

namespace itk{

/**
* \brief Performes deterministic streamline tracking on the input tensor image.   */

  template< class TTensorPixelType, class TPDPixelType=double>
  class StreamlineTrackingFilter :
      public ImageToImageFilter< Image< DiffusionTensor3D<TTensorPixelType>, 3 >,
      Image< Vector< TPDPixelType, 3 >, 3 > >
  {

  public:

    typedef StreamlineTrackingFilter Self;
    typedef SmartPointer<Self>                      Pointer;
    typedef SmartPointer<const Self>                ConstPointer;
    typedef ImageToImageFilter< Image< DiffusionTensor3D<TTensorPixelType>, 3 >, Image< Vector< TPDPixelType, 3 >, 3 > > Superclass;

    /** Method for creation through the object factory. */
    itkNewMacro(Self)

    /** Runtime information support. */
    itkTypeMacro(StreamlineTrackingFilter, ImageToImageFilter)

    typedef TTensorPixelType                            TensorComponentType;
    typedef TPDPixelType                                DirectionPixelType;
    typedef typename Superclass::InputImageType         InputImageType;
    typedef typename Superclass::OutputImageType        OutputImageType;
    typedef typename Superclass::OutputImageRegionType  OutputImageRegionType;
    typedef itk::Image<unsigned char, 3>                ItkUcharImgType;
    typedef itk::Image<float, 3>                        ItkFloatImgType;
    typedef itk::Image< vnl_vector_fixed<double,3>, 3>  ItkPDImgType;
    typedef vtkSmartPointer< vtkPolyData >              FiberPolyDataType;

    itkGetMacro( FiberPolyData, FiberPolyDataType )
    itkSetMacro( SeedImage, ItkUcharImgType::Pointer)
    itkSetMacro( MaskImage, ItkUcharImgType::Pointer)
    itkSetMacro( FaImage, ItkFloatImgType::Pointer)
    itkSetMacro( SeedsPerVoxel, int)
    itkSetMacro( FaThreshold, float)
    itkSetMacro( StepSize, float)
    itkSetMacro( F, float )
    itkSetMacro( G, float )
    itkSetMacro( Interpolate, bool )
    itkSetMacro( MinTractLength, float )
    itkGetMacro( MinTractLength, float )
    itkSetMacro( MinCurvatureRadius, float )
    itkGetMacro( MinCurvatureRadius, float )
    itkSetMacro( ResampleFibers, bool )

  protected:
    StreamlineTrackingFilter();
    ~StreamlineTrackingFilter() {}
    void PrintSelf(std::ostream& os, Indent indent) const;

    void CalculateNewPosition(itk::ContinuousIndex<double, 3>& pos, vnl_vector_fixed<double,3>& dir, typename InputImageType::IndexType& index);
    float FollowStreamline(itk::ContinuousIndex<double, 3> pos, int dirSign, vtkPoints* points, std::vector< vtkIdType >& ids, int imageIdx);
    bool IsValidPosition(itk::ContinuousIndex<double, 3>& pos, typename InputImageType::IndexType& index, vnl_vector_fixed< float, 8 >& interpWeights, int imageIdx);

    double RoundToNearest(double num);
    void BeforeThreadedGenerateData();
    void ThreadedGenerateData( const OutputImageRegionType &outputRegionForThread, ThreadIdType threadId);
    void AfterThreadedGenerateData();

    FiberPolyDataType AddPolyData(FiberPolyDataType poly1, FiberPolyDataType poly2);

    FiberPolyDataType m_FiberPolyData;
    vtkSmartPointer<vtkPoints> m_Points;
    vtkSmartPointer<vtkCellArray> m_Cells;

    std::vector< ItkFloatImgType::Pointer >    m_EmaxImage;
    ItkFloatImgType::Pointer    m_FaImage;
    std::vector< ItkPDImgType::Pointer >       m_PdImage;
    std::vector< typename InputImageType::Pointer > m_InputImage;

    int m_NumberOfInputs;
    float m_FaThreshold;
    float m_MinCurvatureRadius;
    float m_StepSize;
    int m_MaxLength;
    float m_MinTractLength;
    int m_SeedsPerVoxel;
    float m_F;
    float m_G;
    std::vector< int > m_ImageSize;
    std::vector< float > m_ImageSpacing;
    ItkUcharImgType::Pointer m_SeedImage;
    ItkUcharImgType::Pointer m_MaskImage;
    bool m_Interpolate;
    float m_PointPistance;
    bool m_ResampleFibers;

    itk::VectorContainer< int, FiberPolyDataType >::Pointer m_PolyDataContainer;

  private:

  };

}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkStreamlineTrackingFilter.cpp"
#endif

#endif //__itkStreamlineTrackingFilter_h_

