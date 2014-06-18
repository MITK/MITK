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

#include <MitkFiberTrackingExports.h>
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
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    /** Runtime information support. */
    itkTypeMacro(StreamlineTrackingFilter, ImageToImageFilter)

    typedef TTensorPixelType                            TensorComponentType;
    typedef TPDPixelType                                DirectionPixelType;
    typedef typename Superclass::InputImageType         InputImageType;
    typedef typename Superclass::OutputImageType        OutputImageType;
    typedef typename Superclass::OutputImageRegionType  OutputImageRegionType;
    typedef itk::Image<unsigned char, 3>                ItkUcharImgType;
    typedef itk::Image<double, 3>                       ItkDoubleImgType;
    typedef itk::Image<float, 3>                        ItkFloatImgType;
    typedef itk::Image< vnl_vector_fixed<double,3>, 3>  ItkPDImgType;
    typedef vtkSmartPointer< vtkPolyData >              FiberPolyDataType;

    itkGetMacro( FiberPolyData, FiberPolyDataType )     ///< Output fibers
    itkSetMacro( SeedImage, ItkUcharImgType::Pointer)   ///< Seeds are only placed inside of this mask.
    itkSetMacro( MaskImage, ItkUcharImgType::Pointer)   ///< Tracking is only performed inside of this mask image.
    itkSetMacro( FaImage, ItkFloatImgType::Pointer)     ///< Use this FA image instead of the automatically calculated one. Necessary for multi tensor tracking.
    itkSetMacro( SeedsPerVoxel, int)                    ///< One seed placed in the center of each voxel or multiple seeds randomly placed inside each voxel.
    itkSetMacro( FaThreshold, double)                    ///< FA termination criterion.
    itkSetMacro( StepSize, double)                       ///< Integration step size in mm
    itkSetMacro( F, double )                             ///< Tensor deflection parameter f
    itkSetMacro( G, double )                             ///< Tensor deflection parameter g
    itkSetMacro( Interpolate, bool )                    ///< Toggle between nearest neighbour (false) and trilinear interpolation (true)
    itkSetMacro( MinTractLength, double )                ///< Shorter tracts are discarded.
    itkGetMacro( MinTractLength, double )
    itkSetMacro( MinCurvatureRadius, double )            ///< Tracking is stopped if curvature radius (in mm) is too small.
    itkGetMacro( MinCurvatureRadius, double )
    itkSetMacro( ResampleFibers, bool )                 ///< If enabled, the resulting fibers are resampled to feature point distances of 0.5*MinSpacing. This is recommendable for very short integration steps and many seeds. If disabled, the resulting fiber bundle might become very large.

  protected:
    StreamlineTrackingFilter();
    ~StreamlineTrackingFilter() {}
    void PrintSelf(std::ostream& os, Indent indent) const;

    void CalculateNewPosition(itk::ContinuousIndex<double, 3>& pos, vnl_vector_fixed<double,3>& dir, typename InputImageType::IndexType& index);    ///< Calculate next integration step.
    double FollowStreamline(itk::ContinuousIndex<double, 3> pos, int dirSign, vtkPoints* points, std::vector< vtkIdType >& ids, int imageIdx);       ///< Start streamline in one direction.
    bool IsValidPosition(itk::ContinuousIndex<double, 3>& pos, typename InputImageType::IndexType& index, vnl_vector_fixed< double, 8 >& interpWeights, int imageIdx);   ///< Are we outside of the mask image? Is the FA too low?

    double RoundToNearest(double num);
    void BeforeThreadedGenerateData();
    void ThreadedGenerateData( const OutputImageRegionType &outputRegionForThread, ThreadIdType threadId);
    void AfterThreadedGenerateData();

    FiberPolyDataType AddPolyData(FiberPolyDataType poly1, FiberPolyDataType poly2);    ///< Combine tracking results generated by the individual threads.

    FiberPolyDataType               m_FiberPolyData;
    vtkSmartPointer<vtkPoints>      m_Points;
    vtkSmartPointer<vtkCellArray>   m_Cells;

    std::vector< ItkDoubleImgType::Pointer >         m_EmaxImage;    ///< Stores largest eigenvalues per voxel (one for each tensor)
    ItkFloatImgType::Pointer                        m_FaImage;      ///< FA image used to determine streamline termination.
    std::vector< ItkPDImgType::Pointer >            m_PdImage;      ///< Stores principal direction of each tensor in each voxel.
    std::vector< typename InputImageType::Pointer > m_InputImage;   ///< Input tensor images. For multi tensor tracking provide multiple tensor images.

    int     m_NumberOfInputs;
    double   m_FaThreshold;
    double   m_MinCurvatureRadius;
    double   m_StepSize;
    int     m_MaxLength;
    double   m_MinTractLength;
    int     m_SeedsPerVoxel;
    double   m_F;
    double   m_G;
    bool    m_Interpolate;
    double   m_PointPistance;
    bool    m_ResampleFibers;
    std::vector< int >          m_ImageSize;
    std::vector< double >        m_ImageSpacing;
    ItkUcharImgType::Pointer    m_SeedImage;
    ItkUcharImgType::Pointer    m_MaskImage;

    itk::VectorContainer< int, FiberPolyDataType >::Pointer m_PolyDataContainer;

  private:

  };

}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkStreamlineTrackingFilter.cpp"
#endif

#endif //__itkStreamlineTrackingFilter_h_

