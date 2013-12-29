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

#include "mitkSegTool2D.h"
#include "mitkLegacyAdaptors.h"
#include "SegmentationExports.h"
#include "mitkDataNode.h"
#include "mitkPointSet.h"
#include "mitkToolCommand.h"
#include "mitkStateEvent.h"
#include "mitkLabelSetImage.h"

#include "itkImage.h"

//itk filter
#include "itkFastMarchingImageFilter.h"
#include "itkBinaryThresholdImageFilter.h"
#include "itkGradientMagnitudeRecursiveGaussianImageFilter.h"
#include "itkSigmoidImageFilter.h"
#include "itkCurvatureAnisotropicDiffusionImageFilter.h"

namespace us {
class ModuleResource;
}

namespace mitk
{

  class StateMachineAction;
  class InteractionEvent;

/**
  \brief FastMarching semgentation tool.

  The processing pipeline is triggered upon placing the first seed and updated every time
  a new seed is added or a parameter is changed.
  The pipeline is as follows:
  Smoothing->GradientMagnitude->SigmoidFunction->FastMarching->Thresholding
  As with most segmentation tools, a preview image is shown as a green contour, which the user
  can accept or reject.
  If the preview image is accepted, its contents will be transferred to the active label.


  For detailed documentation see ITK Software Guide section 9.3.1 Fast Marching Segmentation.
*/
class Segmentation_EXPORT FastMarchingTool : public SegTool2D
{
    mitkNewMessageMacro(Ready);

  public:

    mitkClassMacro(FastMarchingTool, SegTool2D);
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
    typedef itk::FastMarchingImageFilter< InternalImageType, InternalImageType >                          FastMarchingFilterType;
    typedef FastMarchingFilterType::NodeContainer                                                       NodeContainer;
    typedef FastMarchingFilterType::NodeType                                                            NodeType;

    /* icon stuff */
    virtual const char** GetXPM() const;
    virtual const char* GetName() const;

    virtual us::ModuleResource GetCursorIconResource() const;
    us::ModuleResource GetIconResource() const;

    /// \brief Creates a new label out of the the current preview image.
    virtual void CreateNewLabel(const std::string& name, const mitk::Color& color);

    /// \brief Stops the running operation and disables the tool
    void Cancel();

    /// \brief Set parameter used in Threshold filter.
    void SetUpperThreshold(double);
    double GetUpperThreshold();

    /// \brief Set parameter used in Threshold filter.
    void SetLowerThreshold(double);
    double GetLowerThreshold();

    /// \brief Set parameter used in Fast Marching filter.
    void SetStoppingValue(double);
    double GetStoppingValue();

    /// \brief Set parameter used in Gradient Magnitude filter.
    void SetSigma(double);
    double GetSigma();

    /// \brief Set parameter used in Fast Marching filter.
    void SetAlpha(double);
    double GetAlpha();

    /// \brief Set parameter used in Fast Marching filter.
    void SetBeta(double);
    double GetBeta();

    /// \brief Adds the feedback image to the current working image.
    void AcceptPreview();

    /// \brief Updates the tool and generates a new preview image
    void Run();

    /// \brief Clear all seed points.
    void ClearSeeds();

  protected:

    FastMarchingTool();
    virtual ~FastMarchingTool();

    void ConnectActionsAndFunctions();

    //virtual float CanHandleEvent( StateEvent const *stateEvent) const;

    virtual void Activated();
    virtual void Deactivated();

    /// \brief Add point action of StateMachine pattern
    virtual bool OnAddPoint ( StateMachineAction*, InteractionEvent* interactionEvent );

    /// \brief Delete action of StateMachine pattern
    virtual bool OnDelete ( StateMachineAction*, InteractionEvent* interactionEvent );

    mitk::ToolCommand::Pointer m_ProgressCommand;

    Image::Pointer m_ReferenceImageSlice;

    bool m_NeedUpdate;

    bool m_Initialized;

    int m_CurrentTimeStep;

    template < typename ImageType >
    void InternalRun( ImageType* input );

    mitk::InteractionPositionEvent::Pointer m_PositionEvent;

    int m_OverwritePixelValue;

    float m_LowerThreshold; //used in Threshold filter
    float m_UpperThreshold; //used in Threshold filter
    float m_StoppingValue; //used in Fast Marching filter
    float m_Sigma; //used in GradientMagnitude filter
    float m_Alpha; //used in Sigmoid filter
    float m_Beta; //used in Sigmoid filter

    //used to visualize the preview segmentation
    DataNode::Pointer  m_PreviewNode;
    Image::Pointer     m_PreviewImage;

    //seed points for FastMarching
    NodeContainer::Pointer m_SeedContainer;

    //used to visualize the seed points
    mitk::DataNode::Pointer             m_SeedsAsPointSetNode;
    mitk::PointSet::Pointer             m_SeedsAsPointSet;

    ThresholdingFilterType::Pointer     m_ThresholdFilter;
    SmoothingFilterType::Pointer        m_SmoothFilter;
    GradientFilterType::Pointer         m_GradientMagnitudeFilter;
    SigmoidFilterType::Pointer          m_SigmoidFilter;
    FastMarchingFilterType::Pointer     m_FastMarchingFilter;

};

} // namespace

#endif
