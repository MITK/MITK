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
#include <TrackingHandlers/mitkTrackingDataHandler.h>
#include <MitkFiberTrackingExports.h>

namespace itk{

/**
* \brief Performs streamline tracking on the input image. Depending on the tracking handler this can be a tensor, peak or machine learning based tracking. */

class MITKFIBERTRACKING_EXPORT StreamlineTrackingFilter : public ProcessObject
{

public:

    typedef StreamlineTrackingFilter Self;
    typedef SmartPointer<Self>                      Pointer;
    typedef SmartPointer<const Self>                ConstPointer;
    typedef ProcessObject Superclass;

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
    itkSetMacro( FourTTImage, ItkUcharImgType::Pointer) ///<
    itkSetMacro( SeedsPerVoxel, int)                    ///< One seed placed in the center of each voxel or multiple seeds randomly placed inside each voxel.
    itkSetMacro( StepSize, double)                      ///< Integration step size in mm
    itkSetMacro( MinTractLength, double )               ///< Shorter tracts are discarded.
    itkSetMacro( MaxTractLength, double )               ///< Streamline progression stops if tract is longer than specified.
    itkSetMacro( AngularThreshold, double )             ///< Probabilities for directions with larger angular deviation from previous direction is set to 0
    itkSetMacro( SamplingDistance, double )             ///< Maximum distance of sampling points in mm
    itkSetMacro( UseStopVotes, bool )                   ///< Frontal sampling points can vote for stopping the streamline even if the remaining sampling points keep pushing
    itkSetMacro( OnlyForwardSamples, bool )             ///< Don't use sampling points behind the current position in progression direction
    itkSetMacro( DeflectionMod, double )                 ///< Deflection distance modifier
    itkSetMacro( StoppingRegions, ItkUcharImgType::Pointer) ///< Streamlines entering a stopping region will stop immediately
    itkSetMacro( DemoMode, bool )
    itkSetMacro( SeedOnlyGm, bool )
    itkSetMacro( NumberOfSamples, unsigned int )        ///< Number of neighborhood sampling points
    itkSetMacro( AposterioriCurvCheck, bool )           ///< Checks fiber curvature (angular deviation across 5mm) is larger than 30°. If yes, the streamline progression is stopped.
    itkSetMacro( AvoidStop, bool )                      ///< Use additional sampling points to avoid premature streamline termination
    itkSetMacro( RandomSampling, bool )                 ///< If true, the sampling points are distributed randomly around the current position, not sphericall in the specified sampling distance.
    itkSetMacro( NumPreviousDirections, unsigned int )  ///< How many "old" steps do we want to consider in our decision where to go next?

    void SetTrackingHandler( mitk::TrackingDataHandler* h )   ///<
    {
        m_TrackingHandler = h;
    }

    virtual void Update() override{
        this->GenerateData();
    }

protected:

    void GenerateData() override;

        StreamlineTrackingFilter();
    ~StreamlineTrackingFilter() {}

    void InitGrayMatterEndings();
    void CheckFiberForGmEnding(FiberType* fib);

    void CalculateNewPosition(itk::Point<double, 3>& pos, vnl_vector_fixed<double,3>& dir);    ///< Calculate next integration step.
    double FollowStreamline(itk::Point<double, 3> pos, vnl_vector_fixed<double,3> dir, FiberType* fib, double tractLength, bool front);       ///< Start streamline in one direction.
    bool IsValidPosition(itk::Point<double, 3>& pos);   ///< Are we outside of the mask image?
    vnl_vector_fixed<double,3> GetNewDirection(itk::Point<double, 3>& pos, std::deque< vnl_vector_fixed<double,3> >& olddirs, itk::Index<3>& oldIndex); ///< Determine new direction by sample voting at the current position taking the last progression direction into account.

    double GetRandDouble(double min=-1, double max=1);
    std::vector< vnl_vector_fixed<double,3> > CreateDirections(int NPoints);

    void BeforeTracking();
    void AfterTracking();

    PolyDataType                        m_FiberPolyData;
    vtkSmartPointer<vtkPoints>          m_Points;
    vtkSmartPointer<vtkCellArray>       m_Cells;
    BundleType                          m_Tractogram;
    BundleType                          m_GmStubs;

    double                              m_AngularThreshold;
    double                              m_StepSize;
    int                                 m_MaxLength;
    double                              m_MinTractLength;
    double                              m_MaxTractLength;
    int                                 m_SeedsPerVoxel;
    bool                                m_RandomSampling;
    double                              m_SamplingDistance;
    double                              m_DeflectionMod;
    bool                                m_OnlyForwardSamples;
    bool                                m_UseStopVotes;
    unsigned int                        m_NumberOfSamples;
    unsigned int                        m_NumPreviousDirections;
    int                                 m_WmLabel;
    int                                 m_GmLabel;
    bool                                m_SeedOnlyGm;

    ItkUcharImgType::Pointer            m_StoppingRegions;
    ItkUcharImgType::Pointer            m_SeedImage;
    ItkUcharImgType::Pointer            m_MaskImage;
    ItkUcharImgType::Pointer            m_FourTTImage;

    bool                                m_AposterioriCurvCheck;
    bool                                m_AvoidStop;
    bool                                m_DemoMode;
    void BuildFibers(bool check);
    int CheckCurvature(FiberType* fib, bool front);

    // decision forest
    mitk::TrackingDataHandler*          m_TrackingHandler;
    std::vector< PolyDataType >         m_PolyDataContainer;

    std::chrono::time_point<std::chrono::system_clock> m_StartTime;
    std::chrono::time_point<std::chrono::system_clock> m_EndTime;

private:

};

}

//#ifndef ITK_MANUAL_INSTANTIATION
//#include "itkMLBSTrackingFilter.cpp"
//#endif

#endif //__itkMLBSTrackingFilter_h_

