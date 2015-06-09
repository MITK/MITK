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
#ifndef __itkMLBSTrackingFilter_h_
#define __itkMLBSTrackingFilter_h_

#include <itkImageToImageFilter.h>
#include <itkVectorContainer.h>
#include <itkVectorImage.h>
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkCellArray.h>
#include <vtkPoints.h>
#include <vtkPolyLine.h>
#include <vtkCleanPolyData.h>
#include <itkOrientationDistributionFunction.h>
#include <itkMersenneTwisterRandomVariateGenerator.h>
#include <itkAnalyticalDiffusionQballReconstructionImageFilter.h>
#include <itkSimpleFastMutexLock.h>
#include <mitkDiffusionPropertyHelper.h>

// classification includes
#include <vigra/random_forest.hxx>
#include <vigra/multi_array.hxx>
#include <vigra/random_forest_hdf5_impex.hxx>

namespace itk{

/**
* \brief Performes deterministic streamline tracking on the input tensor image.   */

template< int NumImageFeatures=100 >
class MLBSTrackingFilter : public ImageToImageFilter< VectorImage< short, 3 >, Image< double, 3 > >
{

public:

    typedef MLBSTrackingFilter Self;
    typedef SmartPointer<Self>                      Pointer;
    typedef SmartPointer<const Self>                ConstPointer;
    typedef ImageToImageFilter< VectorImage< short, 3 >, Image< double, 3 > > Superclass;

    typedef vigra::RandomForest<int>                        DecisionForestType;
    typedef typename Superclass::InputImageType             InputImageType;
    typedef typename Superclass::InputImageRegionType       InputImageRegionType;
    typedef Image< Vector< float, NumImageFeatures > , 3 >     FeatureImageType;

    /** Method for creation through the object factory. */
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    /** Runtime information support. */
    itkTypeMacro(MLBSTrackingFilter, ImageToImageFilter)

    typedef itk::Image<unsigned char, 3>                ItkUcharImgType;
    typedef itk::Image<double, 3>                       ItkDoubleImgType;
    typedef itk::Image<float, 3>                        ItkFloatImgType;
    typedef vtkSmartPointer< vtkPolyData >              PolyDataType;

    typedef std::deque< itk::Point<double> > FiberType;
    typedef std::vector< FiberType > BundleType;

    bool    m_PauseTracking;
    bool    m_AbortTracking;
    bool    m_BuildFibersFinished;
    int     m_BuildFibersReady;
    bool    m_Stop;
//    void RequestFibers(){ m_Stop=true; m_BuildFibersReady=0; m_BuildFibersFinished=false; }

    itkGetMacro( FiberPolyData, PolyDataType )          ///< Output fibers
    itkSetMacro( SeedImage, ItkUcharImgType::Pointer)   ///< Seeds are only placed inside of this mask.
    itkSetMacro( MaskImage, ItkUcharImgType::Pointer)   ///< Tracking is only performed inside of this mask image.
    itkSetMacro( SeedsPerVoxel, int)                    ///< One seed placed in the center of each voxel or multiple seeds randomly placed inside each voxel.
    itkSetMacro( StepSize, double)                      ///< Integration step size in mm
    itkSetMacro( MinTractLength, double )               ///< Shorter tracts are discarded.
    itkSetMacro( MaxTractLength, double )
    itkSetMacro( AngularThreshold, double )
    itkSetMacro( UseDirection, bool )
    itkSetMacro( SamplingDistance, double )
    itkSetMacro( NumberOfSamples, int )
    itkSetMacro( StoppingRegions, ItkUcharImgType::Pointer)
    itkSetMacro( B_Value, float )
    itkSetMacro( GradientDirections, mitk::DiffusionPropertyHelper::GradientDirectionsContainerType::Pointer )
    itkSetMacro( DemoMode, bool )
    itkSetMacro( RemoveWmEndFibers, bool )
    itkSetMacro( AposterioriCurvCheck, bool )
    itkSetMacro( AvoidStop, bool )

    void SetDecisionForest( DecisionForestType* forest )
    {
        m_DecisionForest = forest;
    }

    itkGetMacro( WmImage, ItkDoubleImgType::Pointer )
    itkGetMacro( NotWmImage, ItkDoubleImgType::Pointer )
    itkGetMacro( AvoidStopImage, ItkDoubleImgType::Pointer )

    protected:
        MLBSTrackingFilter();
    ~MLBSTrackingFilter() {}

    void CalculateNewPosition(itk::Point<double, 3>& pos, vnl_vector_fixed<double,3>& dir);    ///< Calculate next integration step.
    double FollowStreamline(ThreadIdType threadId, itk::Point<double, 3> pos, vnl_vector_fixed<double,3> dir, FiberType* fib, double tractLength, bool front);       ///< Start streamline in one direction.
    bool IsValidPosition(itk::Point<double, 3>& pos);   ///< Are we outside of the mask image?
    vnl_vector_fixed<double,3> GetNewDirection(itk::Point<double, 3>& pos, vnl_vector_fixed<double,3>& olddir);
    vnl_vector_fixed<double,3> Classify(itk::Point<double, 3>& pos, int& candidates, vnl_vector_fixed<double,3>& olddir, double angularThreshold, double& prob, bool avoidStop=false);

    typename FeatureImageType::PixelType GetImageValues(itk::Point<float, 3> itkP);
    double GetRandDouble(double min=-1, double max=1);
    double RoundToNearest(double num);

    void BeforeThreadedGenerateData() override;
    void PreprocessRawData();
    void ThreadedGenerateData( const InputImageRegionType &outputRegionForThread, ThreadIdType threadId) override;
    void AfterThreadedGenerateData() override;

    PolyDataType                        m_FiberPolyData;
    vtkSmartPointer<vtkPoints>          m_Points;
    vtkSmartPointer<vtkCellArray>       m_Cells;
    BundleType                          m_Tractogram;

    double                              m_AngularThreshold;
    double                              m_StepSize;
    int                                 m_MaxLength;
    double                              m_MinTractLength;
    double                              m_MaxTractLength;
    int                                 m_SeedsPerVoxel;
    bool                                m_UseDirection;
    double                              m_SamplingDistance;
    int                                 m_NumberOfSamples;
    std::vector< int >                  m_ImageSize;
    std::vector< double >               m_ImageSpacing;

    SimpleFastMutexLock                 m_Mutex;
    ItkUcharImgType::Pointer            m_StoppingRegions;
    ItkDoubleImgType::Pointer           m_WmImage;
    ItkDoubleImgType::Pointer           m_NotWmImage;
    ItkDoubleImgType::Pointer           m_AvoidStopImage;
    ItkUcharImgType::Pointer            m_SeedImage;
    ItkUcharImgType::Pointer            m_MaskImage;
    typename FeatureImageType::Pointer  m_FeatureImage;
    typename InputImageType::Pointer    m_InputImage;
    mitk::DiffusionPropertyHelper::GradientDirectionsContainerType::Pointer m_GradientDirections;
    float                               m_B_Value;

    bool                                m_AposterioriCurvCheck;
    bool                                m_RemoveWmEndFibers;
    bool                                m_AvoidStop;

    int                                 m_Threads;
    bool                                m_DemoMode;
    void BuildFibers(bool check);
    int CheckCurvature(FiberType* fib, bool front);

    // decision forest
    DecisionForestType*                                                 m_DecisionForest;
    itk::OrientationDistributionFunction< double, NumImageFeatures*2 >  m_ODF;
    std::vector< int >                                                  m_DirectionIndices;

    std::vector< PolyDataType >         m_PolyDataContainer;

private:

};

}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkMLBSTrackingFilter.cpp"
#endif

#endif //__itkMLBSTrackingFilter_h_

