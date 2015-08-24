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
#include <mitkPointSet.h>
#include <chrono>
#include <mitkTrackingForestHandler.h>

// classification includes
#include <vigra/random_forest.hxx>
#include <vigra/multi_array.hxx>
#include <vigra/random_forest_hdf5_impex.hxx>

namespace itk{

/**
* \brief Performes deterministic streamline tracking on the input tensor image.   */

template<  int ShOrder=6, int NumImageFeatures=100 >
class MLBSTrackingFilter : public ImageToImageFilter< VectorImage< short, 3 >, Image< double, 3 > >
{

public:

    typedef MLBSTrackingFilter Self;
    typedef SmartPointer<Self>                      Pointer;
    typedef SmartPointer<const Self>                ConstPointer;
    typedef ImageToImageFilter< VectorImage< short, 3 >, Image< double, 3 > > Superclass;

    typedef typename Superclass::InputImageType             InputImageType;
    typedef typename Superclass::InputImageRegionType       InputImageRegionType;
    typedef Image< Vector< float, NumImageFeatures > , 3 >  FeatureImageType;

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

    volatile bool    m_PauseTracking;
    bool    m_AbortTracking;
    bool    m_BuildFibersFinished;
    int     m_BuildFibersReady;
    volatile bool m_Stop;
    mitk::PointSet::Pointer             m_SamplingPointset;
    mitk::PointSet::Pointer             m_AlternativePointset;

    itkGetMacro( FiberPolyData, PolyDataType )          ///< Output fibers
    itkSetMacro( SeedImage, ItkUcharImgType::Pointer)   ///< Seeds are only placed inside of this mask.
    itkSetMacro( MaskImage, ItkUcharImgType::Pointer)   ///< Tracking is only performed inside of this mask image.
    itkSetMacro( SeedsPerVoxel, int)                    ///< One seed placed in the center of each voxel or multiple seeds randomly placed inside each voxel.
    itkSetMacro( StepSize, double)                      ///< Integration step size in mm
    itkSetMacro( MinTractLength, double )               ///< Shorter tracts are discarded.
    itkSetMacro( MaxTractLength, double )               ///< Streamline progression stops if tract is longer than specified.
    itkSetMacro( AngularThreshold, double )             ///< Probabilities for directions with larger angular deviation from previous direction is set to 0
    itkSetMacro( SamplingDistance, double )             ///< Maximum distance of sampling points in mm
    itkSetMacro( NumberOfSamples, int )                 ///< Number of sampling points
    itkSetMacro( StoppingRegions, ItkUcharImgType::Pointer) ///< Streamlines entering a stopping region will stop immediately
    itkSetMacro( DemoMode, bool )
    itkSetMacro( RemoveWmEndFibers, bool )              ///< Checks if fiber ending is located in the white matter. If this is the case, the streamline is discarded.
    itkSetMacro( AposterioriCurvCheck, bool )           ///< Checks fiber curvature (angular deviation across 5mm) is larger than 30°. If yes, the streamline progression is stopped.
    itkSetMacro( AvoidStop, bool )                      ///< Use additional sampling points to avoid premature streamline termination
    itkSetMacro( RandomSampling, bool )                 ///< If true, the sampling points are distributed randomly around the current position, not sphericall in the specified sampling distance.

    void SetForestHandler( mitk::TrackingForestHandler<ShOrder> fh )   ///< Stores random forest classifier and performs actual classification
    {
        m_ForestHandler = fh;
    }

    protected:
        MLBSTrackingFilter();
    ~MLBSTrackingFilter() {}

    void CalculateNewPosition(itk::Point<double, 3>& pos, vnl_vector_fixed<double,3>& dir);    ///< Calculate next integration step.
    double FollowStreamline(itk::Point<double, 3> pos, vnl_vector_fixed<double,3> dir, FiberType* fib, double tractLength, bool front);       ///< Start streamline in one direction.
    bool IsValidPosition(itk::Point<double, 3>& pos);   ///< Are we outside of the mask image?
    vnl_vector_fixed<double,3> GetNewDirection(itk::Point<double, 3>& pos, vnl_vector_fixed<double,3>& olddir); ///< Determine new direction by sample voting at the current position taking the last progression direction into account.

    double GetRandDouble(double min=-1, double max=1);
    double RoundToNearest(double num);

    void BeforeThreadedGenerateData() override;
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
    bool                                m_RandomSampling;
    double                              m_SamplingDistance;
    int                                 m_NumberOfSamples;

    SimpleFastMutexLock                 m_Mutex;
    ItkUcharImgType::Pointer            m_StoppingRegions;
    ItkUcharImgType::Pointer            m_SeedImage;
    ItkUcharImgType::Pointer            m_MaskImage;

    bool                                m_AposterioriCurvCheck;
    bool                                m_RemoveWmEndFibers;
    bool                                m_AvoidStop;
    int                                 m_Threads;
    bool                                m_DemoMode;
    void BuildFibers(bool check);
    int CheckCurvature(FiberType* fib, bool front);

    // decision forest
    mitk::TrackingForestHandler<ShOrder>       m_ForestHandler;
    typename InputImageType::Pointer    m_InputImage;


    std::vector< PolyDataType >         m_PolyDataContainer;

    std::chrono::time_point<std::chrono::system_clock> m_StartTime;
    std::chrono::time_point<std::chrono::system_clock> m_EndTime;

private:

};

}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkMLBSTrackingFilter.cpp"
#endif

#endif //__itkMLBSTrackingFilter_h_

