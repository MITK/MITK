/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkFastMarchingTool3D_h_Included
#define mitkFastMarchingTool3D_h_Included

#include "mitkAutoSegmentationWithPreviewTool.h"
#include "mitkDataNode.h"
#include "mitkPointSet.h"
#include "mitkPointSetDataInteractor.h"
#include "mitkToolCommand.h"

#include "itkImage.h"
#include "itkFastMarchingImageFilter.h"

#include <MitkSegmentationExports.h>

namespace us
{
  class ModuleResource;
}

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
  class MITKSEGMENTATION_EXPORT FastMarchingTool3D : public AutoSegmentationWithPreviewTool
  {
  public:
    mitkClassMacro(FastMarchingTool3D, AutoSegmentationWithPreviewTool);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    bool CanHandle(const BaseData* referenceData, const BaseData* workingData) const override;

    /* icon stuff */
    const char **GetXPM() const override;
    const char *GetName() const override;
    us::ModuleResource GetIconResource() const override;

    void Activated() override;
    void Deactivated() override;

    /// \brief Set parameter used in Threshold filter.
    void SetUpperThreshold(double);

    /// \brief Set parameter used in Threshold filter.
    void SetLowerThreshold(double);

    /// \brief Set parameter used in Fast Marching filter.
    void SetStoppingValue(double);

    /// \brief Set parameter used in Gradient Magnitude filter.
    void SetSigma(double);

    /// \brief Set parameter used in Fast Marching filter.
    void SetAlpha(double);

    /// \brief Set parameter used in Fast Marching filter.
    void SetBeta(double);

    /// \brief Clear all seed points.
    void ClearSeeds();

  protected:
    FastMarchingTool3D();
    ~FastMarchingTool3D() override;

    /// \brief Add point action of StateMachine pattern
    virtual void OnAddPoint();

    /// \brief Delete action of StateMachine pattern
    virtual void OnDelete();

    void UpdatePrepare() override;
    void UpdateCleanUp() override;
    void DoUpdatePreview(const Image* inputAtTimeStep, Image* previewImage, TimeStepType timeStep) override;

    template <typename TPixel, unsigned int VImageDimension>
    void DoITKFastMarching(const itk::Image<TPixel, VImageDimension>* inputImage,
      mitk::Image* segmentation, unsigned int timeStep);

    float m_LowerThreshold; // used in Threshold filter
    float m_UpperThreshold; // used in Threshold filter
    float m_StoppingValue;  // used in Fast Marching filter
    float m_Sigma;          // used in GradientMagnitude filter
    float m_Alpha;          // used in Sigmoid filter
    float m_Beta;           // used in Sigmoid filter

    typedef float InternalPixelType;
    typedef itk::Image<InternalPixelType, 3> InternalImageType;
    typedef itk::FastMarchingImageFilter<InternalImageType, InternalImageType> FastMarchingFilterType;
    typedef FastMarchingFilterType::NodeContainer NodeContainer;
    typedef FastMarchingFilterType::NodeType NodeType;

    NodeContainer::Pointer m_SeedContainer; // seed points for FastMarching

    mitk::DataNode::Pointer m_SeedsAsPointSetNode; // used to visualize the seed points
    mitk::PointSet::Pointer m_SeedsAsPointSet;
    mitk::PointSetDataInteractor::Pointer m_SeedPointInteractor;
    unsigned int m_PointSetAddObserverTag;
    unsigned int m_PointSetRemoveObserverTag;
  };

} // namespace

#endif
