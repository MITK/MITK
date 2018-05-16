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
#include <mitkFiberBundle.h>
#include <mitkPeakImage.h>

namespace itk{

/**
* \brief Performs streamline tracking on the input image. Depending on the tracking handler this can be a tensor, peak or machine learning based tracking. */

class MITKFIBERTRACKING_EXPORT StreamlineTrackingFilter : public ProcessObject
{

public:

  enum EndpointConstraints {
    NONE,                     ///< No constraints on endpoint locations
    EPS_IN_TARGET,            ///< Both EPs are required to be located in the target image
    EPS_IN_TARGET_LABELDIFF,  ///< Both EPs are required to be located in the target image and the image values at the respective position needs to be distinct
    EPS_IN_SEED_AND_TARGET,   ///< One EP is required to be located in the seed image and one in the target image
    MIN_ONE_EP_IN_TARGET,     ///< At least one EP is required to be located in the target image
    ONE_EP_IN_TARGET,         ///< Exactly one EP is required to be located in the target image
    NO_EP_IN_TARGET           ///< No EP is allowed to be located in the target image
  };

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
  typedef itk::Image<unsigned int, 3>                 ItkUintImgType;
  typedef itk::Image<double, 3>                       ItkDoubleImgType;
  typedef itk::Image<float, 3>                        ItkFloatImgType;
  typedef vtkSmartPointer< vtkPolyData >              PolyDataType;

  typedef std::deque< vnl_vector_fixed<float,3> > DirectionContainer;
  typedef std::deque< itk::Point<float> > FiberType;
  typedef std::vector< FiberType > BundleType;

  volatile bool    m_PauseTracking;
  bool    m_AbortTracking;
  bool    m_BuildFibersFinished;
  int     m_BuildFibersReady;
  volatile bool m_Stop;
  mitk::PointSet::Pointer             m_SamplingPointset;
  mitk::PointSet::Pointer             m_StopVotePointset;
  mitk::PointSet::Pointer             m_AlternativePointset;

  void SetStepSize(float v)           ///< Integration step size in voxels, default is 0.5 * voxel
  { m_StepSizeVox = v; }
  void SetAngularThreshold(float v)   ///< Angular threshold per step (in degree), default is 90deg x stepsize
  { m_AngularThresholdDeg = v; }
  void SetSamplingDistance(float v)   ///< Maximum distance of sampling points in voxels, default is 0.25 * voxel
  { m_SamplingDistanceVox = v; }

  void SetDicomProperties(mitk::FiberBundle::Pointer fib);

  itkGetMacro( OutputProbabilityMap, ItkDoubleImgType::Pointer)    ///< Output probability map
  itkGetMacro( FiberPolyData, PolyDataType )            ///< Output fibers
  itkGetMacro( UseOutputProbabilityMap, bool)
  itkGetMacro( MinVoxelSize, float)
  itkGetMacro( EndpointConstraint, EndpointConstraints)

  itkSetMacro( SeedImage, ItkFloatImgType::Pointer)     ///< Seeds are only placed inside of this mask.
  itkSetMacro( MaskImage, ItkFloatImgType::Pointer)     ///< Tracking is only performed inside of this mask image.
  itkSetMacro( ExclusionRegions, ItkFloatImgType::Pointer)///< Fibers passing any of the ROIs in this image are discarded.
  itkSetMacro( SeedsPerVoxel, int)                      ///< One seed placed in the center of each voxel or multiple seeds randomly placed inside each voxel.
  itkSetMacro( MinTractLength, float )                  ///< Shorter tracts are discarded.
  itkSetMacro( MaxTractLength, float )                  ///< Streamline progression stops if tract is longer than specified.
  itkSetMacro( EndpointConstraint, EndpointConstraints) ///< Determines what fibers are accepted based on their endpoint location

  itkSetMacro( UseStopVotes, bool )                   ///< Frontal sampling points can vote for stopping the streamline even if the remaining sampling points keep pushing
  itkSetMacro( OnlyForwardSamples, bool )             ///< Don't use sampling points behind the current position in progression direction
  itkSetMacro( DeflectionMod, float )                 ///< Deflection distance modifier
  itkSetMacro( StoppingRegions, ItkFloatImgType::Pointer) ///< Streamlines entering a stopping region will stop immediately
  itkSetMacro( TargetRegions, ItkFloatImgType::Pointer)    ///< Only streamline starting and ending in this mask are retained
  itkSetMacro( DemoMode, bool )
  itkSetMacro( NumberOfSamples, unsigned int )        ///< Number of neighborhood sampling points
  itkSetMacro( LoopCheck, float )                     ///< Checks fiber curvature (angular deviation across 5mm) is larger than 30°. If yes, the streamline progression is stopped.
  itkSetMacro( AvoidStop, bool )                      ///< Use additional sampling points to avoid premature streamline termination
  itkSetMacro( RandomSampling, bool )                 ///< If true, the sampling points are distributed randomly around the current position, not sphericall in the specified sampling distance.
  itkSetMacro( NumPreviousDirections, unsigned int )  ///< How many "old" steps do we want to consider in our decision where to go next?
  itkSetMacro( MaxNumTracts, int )                    ///< Tracking is stopped if the maximum number of tracts is exceeded
  itkSetMacro( Random, bool )                         ///< If true, seedpoints are shuffled randomly before tracking
  itkSetMacro( Verbose, bool )                        ///< If true, output tracking progress (might be slower)
  itkSetMacro( UseOutputProbabilityMap, bool)         ///< If true, no tractogram but a probability map is created as output.
  itkSetMacro( StopTracking, bool )
  itkSetMacro( InterpolateMasks, bool )
  itkSetMacro( TrialsPerSeed, unsigned int )          ///< When using probabilistic tractography, each seed point is used N times until a valid streamline that is compliant with all thresholds etc. is found
  itkSetMacro( TrackingPriorWeight, float)            ///< Weight between prior and data [0-1]. One mean tracking only on the prior peaks, zero only on the data.
  itkSetMacro( TrackingPriorAsMask, bool)             ///< If true, data directions in voxels where prior directions are invalid are set to zero
  itkSetMacro( IntroduceDirectionsFromPrior, bool)    ///< If false, prior voxels with invalid data voxel are ignored

  ///< Use manually defined points in physical space as seed points instead of seed image
  void SetSeedPoints( const std::vector< itk::Point<float> >& sP) {
    m_SeedPoints = sP;
  }

  void SetTrackingHandler( mitk::TrackingDataHandler* h )   ///<
  {
    m_TrackingHandler = h;
  }

  void Update() override{
    this->GenerateData();
  }

  std::string GetStatusText();

  void SetTrackingPriorHandler(mitk::TrackingDataHandler *TrackingPriorHandler);

protected:

  void GenerateData() override;

  StreamlineTrackingFilter();
  ~StreamlineTrackingFilter() override {}

  bool IsValidFiber(FiberType* fib);  ///< Check endpoints
  void FiberToProbmap(FiberType* fib);
  void GetSeedPointsFromSeedImage();
  void CalculateNewPosition(itk::Point<float, 3>& pos, vnl_vector_fixed<float,3>& dir);    ///< Calculate next integration step.
  float FollowStreamline(itk::Point<float, 3> start_pos, vnl_vector_fixed<float,3> dir, FiberType* fib, DirectionContainer* container, float tractLength, bool front, bool& exclude);       ///< Start streamline in one direction.
  vnl_vector_fixed<float,3> GetNewDirection(const itk::Point<float, 3>& pos, std::deque< vnl_vector_fixed<float,3> >& olddirs, itk::Index<3>& oldIndex); ///< Determine new direction by sample voting at the current position taking the last progression direction into account.

  std::vector< vnl_vector_fixed<float,3> > CreateDirections(int NPoints);

  void BeforeTracking();
  void AfterTracking();

  PolyDataType                        m_FiberPolyData;
  vtkSmartPointer<vtkPoints>          m_Points;
  vtkSmartPointer<vtkCellArray>       m_Cells;
  BundleType                          m_Tractogram;
  BundleType                          m_GmStubs;

  ItkFloatImgType::Pointer            m_StoppingRegions;
  ItkFloatImgType::Pointer            m_TargetRegions;
  ItkFloatImgType::Pointer            m_SeedImage;
  ItkFloatImgType::Pointer            m_MaskImage;
  ItkFloatImgType::Pointer            m_ExclusionRegions;
  ItkDoubleImgType::Pointer           m_OutputProbabilityMap;

  float                               m_MinVoxelSize;
  float                               m_AngularThresholdDeg;
  float                               m_StepSizeVox;
  float                               m_SamplingDistanceVox;
  float                               m_AngularThreshold;
  float                               m_StepSize;
  int                                 m_MaxLength;
  float                               m_MinTractLength;
  float                               m_MaxTractLength;
  int                                 m_SeedsPerVoxel;

  bool                                m_AvoidStop;
  bool                                m_RandomSampling;
  float                               m_SamplingDistance;
  float                               m_DeflectionMod;
  bool                                m_OnlyForwardSamples;
  bool                                m_UseStopVotes;
  unsigned int                        m_NumberOfSamples;

  unsigned int                        m_NumPreviousDirections;
  int                                 m_MaxNumTracts;

  bool                                m_Verbose;
  float                               m_LoopCheck;
  bool                                m_DemoMode;
  bool                                m_Random;
  bool                                m_UseOutputProbabilityMap;
  std::vector< itk::Point<float> >    m_SeedPoints;
  unsigned int                        m_CurrentTracts;
  unsigned int                        m_Progress;
  bool                                m_StopTracking;
  bool                                m_InterpolateMasks;
  unsigned int                        m_TrialsPerSeed;
  EndpointConstraints                 m_EndpointConstraint;

  void BuildFibers(bool check);
  float CheckCurvature(DirectionContainer *fib, bool front);

  // decision forest
  mitk::TrackingDataHandler*          m_TrackingHandler;
  std::vector< PolyDataType >         m_PolyDataContainer;

  std::chrono::time_point<std::chrono::system_clock> m_StartTime;
  std::chrono::time_point<std::chrono::system_clock> m_EndTime;

  itk::LinearInterpolateImageFunction< ItkFloatImgType, float >::Pointer   m_MaskInterpolator;
  itk::LinearInterpolateImageFunction< ItkFloatImgType, float >::Pointer   m_StopInterpolator;
  itk::LinearInterpolateImageFunction< ItkFloatImgType, float >::Pointer   m_TargetInterpolator;
  itk::LinearInterpolateImageFunction< ItkFloatImgType, float >::Pointer   m_SeedInterpolator;
  itk::LinearInterpolateImageFunction< ItkFloatImgType, float >::Pointer   m_ExclusionInterpolator;
  bool                                                                     m_SeedImageSet;
  bool                                                                     m_TargetImageSet;


  // Directional priors
  bool                                        m_IntroduceDirectionsFromPrior;
  bool                                        m_TrackingPriorAsMask;
  float                                       m_TrackingPriorWeight;
  mitk::TrackingDataHandler*                  m_TrackingPriorHandler;

private:

};

}

//#ifndef ITK_MANUAL_INSTANTIATION
//#include "itkMLBSTrackingFilter.cpp"
//#endif

#endif //__itkMLBSTrackingFilter_h_

