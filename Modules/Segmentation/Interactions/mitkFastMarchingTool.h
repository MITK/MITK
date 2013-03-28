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
*/
class Segmentation_EXPORT FastMarchingTool : public FeedbackContourTool
{
  public:

    mitkClassMacro(FastMarchingTool, FeedbackContourTool);
    itkNewMacro(FastMarchingTool);


    /*
    typedefs for itk pipeline
    */
    typedef float                                                                               InternalPixelType;
    typedef itk::Image< InternalPixelType, 2 >                                                  InternalImageType;
    typedef unsigned char                                                                       OutputPixelType;
    typedef itk::Image< OutputPixelType, 2 >                                            OutputImageType;

    typedef itk::BinaryThresholdImageFilter< InternalImageType, OutputImageType >                       ThresholdingFilterType;
    typedef itk::CurvatureAnisotropicDiffusionImageFilter< InternalImageType, InternalImageType >       SmoothingFilterType;
    typedef itk::GradientMagnitudeRecursiveGaussianImageFilter< InternalImageType, InternalImageType >  GradientFilterType;
    typedef itk::SigmoidImageFilter< InternalImageType, InternalImageType >                             SigmoidFilterType;
    typedef itk::FastMarchingImageFilter< InternalImageType, InternalImageType >                        FastMarchingFilterType;
    typedef FastMarchingFilterType::NodeContainer                                                       NodeContainer;
    typedef FastMarchingFilterType::NodeType                                                            NodeType;


    /* icon */
    virtual const char** GetXPM() const;
    virtual const char* GetName() const;

  protected:

    FastMarchingTool(); // purposely hidden
    virtual ~FastMarchingTool();

    virtual float CanHandleEvent( StateEvent const *stateEvent) const;

    virtual void Activated();
    virtual void Deactivated();

    virtual bool OnMousePressed (Action*, const StateEvent*);
    virtual bool OnMouseMoved   (Action*, const StateEvent*);
    virtual bool OnMouseReleased(Action*, const StateEvent*);
    virtual bool OnAddPoint(Action*, const StateEvent*);
    virtual bool OnDelete(Action*, const StateEvent*);

    void UpdatePreviewImage();

    Image::Pointer m_ReferenceSlice;
    Image::Pointer m_WorkingSlice;

    ScalarType m_LowerThreshold;
    ScalarType m_UpperThreshold;
    ScalarType m_InitialLowerThreshold;
    ScalarType m_InitialUpperThreshold;

    Point2I m_LastScreenPosition;
    int m_ScreenYDifference;
    int m_ScreenXDifference;

  private:

    ScalarType m_MouseDistanceScaleFactor;

    float sigma;
    float alpha;
    float beta;

    NodeContainer::Pointer seeds;

    InternalImageType::Pointer m_SliceInITK;

    mitk::DataNode::Pointer m_ResultImageNode;

    ThresholdingFilterType::Pointer thresholder;
    SmoothingFilterType::Pointer smoothing;
    GradientFilterType::Pointer gradientMagnitude;
    SigmoidFilterType::Pointer sigmoid;
    FastMarchingFilterType::Pointer fastMarching;

};

} // namespace

#endif


