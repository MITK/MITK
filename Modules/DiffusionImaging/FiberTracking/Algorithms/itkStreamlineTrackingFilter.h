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
#include <mitkStreamlineTractographyParameters.h>

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
  typedef itk::Image<unsigned int, 3>                 ItkUintImgType;
  typedef itk::Image<double, 3>                       ItkDoubleImgType;
  typedef itk::Image<float, 3>                        ItkFloatImgType;
  typedef vtkSmartPointer< vtkPolyData >              PolyDataType;
  typedef mitk::StreamlineTractographyParameters::EndpointConstraints EndpointConstraints;
  typedef mitk::StreamlineTractographyParameters::MODE MODE;

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

  void SetDicomProperties(mitk::FiberBundle::Pointer fib);

  itkGetMacro( OutputProbabilityMap, ItkDoubleImgType::Pointer)    ///< Output probability map
  itkGetMacro( FiberPolyData, PolyDataType )            ///< Output fibers

  itkSetMacro( SeedImage, ItkFloatImgType::Pointer)     ///< Seeds are only placed inside of this mask.
  itkSetMacro( MaskImage, ItkFloatImgType::Pointer)     ///< Tracking is only performed inside of this mask image.
  itkSetMacro( ExclusionRegions, ItkFloatImgType::Pointer)///< Fibers passing any of the ROIs in this image are discarded.

  itkSetMacro( StoppingRegions, ItkFloatImgType::Pointer) ///< Streamlines entering a stopping region will stop immediately
  itkSetMacro( TargetRegions, ItkFloatImgType::Pointer)    ///< Only streamline starting and ending in this mask are retained
  itkSetMacro( DemoMode, bool )
  itkSetMacro( Verbose, bool )                        ///< If true, output tracking progress (might be slower)
  itkSetMacro( StopTracking, bool )

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

  std::shared_ptr< mitk::StreamlineTractographyParameters > GetParameters() const;
  void SetParameters(std::shared_ptr< mitk::StreamlineTractographyParameters > Parameters);

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

  std::vector< vnl_vector_fixed<float,3> > CreateDirections(unsigned int NPoints);

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

  bool                                m_Verbose;
  bool                                m_DemoMode;
  std::vector< itk::Point<float> >    m_SeedPoints;
  unsigned int                        m_CurrentTracts;
  unsigned int                        m_Progress;
  bool                                m_StopTracking;

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

  std::shared_ptr< mitk::StreamlineTractographyParameters > m_Parameters;


  // Directional priors
  mitk::TrackingDataHandler*                  m_TrackingPriorHandler;

private:

};

}

//#ifndef ITK_MANUAL_INSTANTIATION
//#include "itkMLBSTrackingFilter.cpp"
//#endif

#endif //__itkMLBSTrackingFilter_h_

