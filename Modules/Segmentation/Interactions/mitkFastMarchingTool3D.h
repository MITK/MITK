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

#ifndef mitkFastMarchingTool3D_h_Included
#define mitkFastMarchingTool3D_h_Included

#include "mitkFeedbackContourTool.h"
#include "mitkLegacyAdaptors.h"
#include "SegmentationExports.h"
#include "mitkDataNode.h"
#include "mitkPointSet.h"
#include "mitkToolCommand.h"

#include "itkImage.h"

//itk filter
#include "itkFastMarchingImageFilter.h"
#include "itkBinaryThresholdImageFilter.h"
#include "itkGradientMagnitudeRecursiveGaussianImageFilter.h"
#include "itkSigmoidImageFilter.h"
#include "itkCurvatureAnisotropicDiffusionImageFilter.h"


namespace mitk
{

/**
  \brief FastMarching semgentation tool.

  The segmentation is done by setting one or more seed points on the image
  and adapting the time range and threshold. The pipeline is:
    Smoothing->GradientMagnitude->SigmoidFunction->FastMarching->Threshold
  The resulting binary image is seen as a segmentation of an object.

  For detailed documentation see ITK Software Guide section 9.3.1 Fast Marching Segmentation.
*/
class Segmentation_EXPORT FastMarchingTool3D : public FeedbackContourTool
{
  public:

    mitkClassMacro(FastMarchingTool3D, FeedbackContourTool);
    itkNewMacro(FastMarchingTool3D);

    /* typedefs for itk pipeline */
    typedef float                                     InternalPixelType;
    typedef itk::Image< InternalPixelType, 3 >        InternalImageType;
    typedef unsigned char                             OutputPixelType;
    typedef itk::Image< OutputPixelType, 3 >          OutputImageType;

    typedef itk::BinaryThresholdImageFilter< InternalImageType, OutputImageType >                       ThresholdingFilterType;
    typedef itk::CurvatureAnisotropicDiffusionImageFilter< InternalImageType, InternalImageType >       SmoothingFilterType;
    typedef itk::GradientMagnitudeRecursiveGaussianImageFilter< InternalImageType, InternalImageType >  GradientFilterType;
    typedef itk::SigmoidImageFilter< InternalImageType, InternalImageType >                             SigmoidFilterType;
    typedef itk::FastMarchingImageFilter< InternalImageType, InternalImageType >                        FastMarchingFilterType;
    typedef FastMarchingFilterType::NodeContainer                                                       NodeContainer;
    typedef FastMarchingFilterType::NodeType                                                            NodeType;


    /* icon stuff */
    virtual const char** GetXPM() const;
    virtual const char* GetName() const;
    virtual std::string GetIconPath() const;

    void SetUpperThreshold(double);
    void SetLowerThreshold(double);
    void SetBeta(double);
    void SetStoppingValue(double);
    void SetAlpha(double);

    virtual void ConfirmSegmentation();

    virtual void SetCurrentTimeStep(int t);

    /// \brief Clear all seed point.
    void ClearSeeds();

    /// \brief Updates the itk pipeline and shows the result of FastMarching.
    void Update();

  protected:

    FastMarchingTool3D();
    virtual ~FastMarchingTool3D();

    virtual float CanHandleEvent( StateEvent const *stateEvent) const;

    virtual void Activated();
    virtual void Deactivated();
    virtual void Initialize();

    /*Actions of StateMachine pattern*/
    virtual bool OnAddPoint     (Action*, const StateEvent*);
    virtual bool OnDelete       (Action*, const StateEvent*);

    /// \brief Reset all relevant inputs of the itk pipeline.
    void Reset();

    mitk::ToolCommand::Pointer m_ProgressCommand;

    Image::Pointer m_ReferenceSlice;
    Image::Pointer m_WorkingSlice;

    ScalarType m_LowerThreshold;
    ScalarType m_UpperThreshold;
    ScalarType m_InitialLowerThreshold;
    ScalarType m_InitialUpperThreshold;
    ScalarType m_InitialStoppingValue;
    ScalarType m_StoppingValue;

    bool m_NeedUpdate;

    int m_CurrentTimeStep;

    float m_Sigma;//used in GradientFilter
    float m_Alpha;//used in SigmoidFilter
    float m_Beta;//used in SigmoidFilter

    NodeContainer::Pointer m_SeedContainer; //seed points for FastMarching

    InternalImageType::Pointer m_ReferenceImageAsITK; //the reference image as itk::Image

    mitk::DataNode::Pointer m_ResultImageNode;//holds the result as a preview image

    mitk::DataNode::Pointer m_SeedsAsPointSetNode;//used to visualize the seed points
    mitk::PointSet::Pointer m_SeedsAsPointSet;

    ThresholdingFilterType::Pointer m_ThresholdFilter;
    SmoothingFilterType::Pointer m_SmoothFilter;
    GradientFilterType::Pointer m_GradientMagnitudeFilter;
    SigmoidFilterType::Pointer m_SigmoidFilter;
    FastMarchingFilterType::Pointer m_FastMarchingFilter;

};

} // namespace

#endif
