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

#ifndef mitkFastMarchingTool_h_Included
#define mitkFastMarchingTool_h_Included

#include "mitkFeedbackContourTool.h"
#include "mitkLegacyAdaptors.h"
#include "SegmentationExports.h"
#include "mitkDataNode.h"
#include "mitkPointSet.h"
#include "mitkPositionEvent.h"

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

  The segmentation is done by setting one or more seed point within the image and adapting
  time range plus threshold. The pipeline is:
    Smoothing->GradientMagnitude->SigmoidFunction->FastMarching->Threshold
  The resulting binary image is seen as a segmentation of an object.

  For detailed documentation see ITK Software Guide section 9.3.1 Fast Marching Segmentation.
*/
class Segmentation_EXPORT FastMarchingTool : public FeedbackContourTool
{
  public:

    mitkClassMacro(FastMarchingTool, FeedbackContourTool);
    itkNewMacro(FastMarchingTool);


    /* typedefs for itk pipeline */
    typedef float                                     InternalPixelType;
    typedef itk::Image< InternalPixelType, 2 >        InternalImageType;
    typedef unsigned char                             OutputPixelType;
    typedef itk::Image< OutputPixelType, 2 >          OutputImageType;

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

    /* parameters of itk filters */
    void SetUpperThreshold(float value);
    void SetLowerThreshold(float value);
    void SetMu(float value);
    void SetStoppingValue(float value);
    void SetStandardDeviation(float value);

    /// \brief Fill preview in segmentation image.
    virtual void ConfirmSegmentation();

    /// \brief Select update preview image.
    virtual void SetLivePreviewEnabled(bool enabled);

    /// \brief Clear all seed point.
    void ClearSeeds();

  protected:

    FastMarchingTool();
    virtual ~FastMarchingTool();

    virtual float CanHandleEvent( StateEvent const *stateEvent) const;

    virtual void Activated();
    virtual void Deactivated();

    /*Actions of StateMachine pattern*/
    virtual bool OnAddPoint     (Action*, const StateEvent*);
    virtual bool OnDelete       (Action*, const StateEvent*);

    /// \brief Updates the itk pipeline and shows the result of FastMarching.
    void UpdatePreviewImage();


    /// \brief Reset all relevant inputs of the itk pipeline.
    void ResetFastMarching(const PositionEvent* positionEvent);

    /// \brief Reinit filters.
    void ResetToInitialState();

    mitk::BaseRenderer* m_LastEventSender;
    unsigned int m_LastEventSlice;
    mitk::PositionEvent* m_PositionEvent;

    Image::Pointer m_ReferenceSlice;
    Image::Pointer m_WorkingSlice;

    ScalarType m_LowerThreshold;
    ScalarType m_UpperThreshold;
    ScalarType m_StoppingValue;

    bool m_IsLivePreviewEnabled;//update preview image or not

    float sigma;//used for GradientFilter
    float alpha;//used for SigmoidFilter
    float beta;//used for SigmoidFilter

    NodeContainer::Pointer seeds;//seed point for FastMarching

    InternalImageType::Pointer m_SliceInITK;//the reference slice as itk::Image

    mitk::DataNode::Pointer m_ResultImageNode;//holds the result as a preview image

    mitk::DataNode::Pointer m_SeedsAsPointSetNode;//used to visualize the seed points
    mitk::PointSet::Pointer m_SeedsAsPointSet;//seed points as PointSet

    /* ITK filters */
    ThresholdingFilterType::Pointer thresholder;
    SmoothingFilterType::Pointer smoothing;
    GradientFilterType::Pointer gradientMagnitude;
    SigmoidFilterType::Pointer sigmoid;
    FastMarchingFilterType::Pointer fastMarching;

};

} // namespace

#endif
