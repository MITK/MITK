/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkBinaryThresholdBaseTool_h
#define mitkBinaryThresholdBaseTool_h

#include <MitkSegmentationExports.h>

#include <mitkSegWithPreviewTool.h>

#include <mitkCommon.h>
#include <mitkDataNode.h>

#include <itkBinaryThresholdImageFilter.h>
#include <itkImage.h>

namespace mitk
{
  /**
   * \brief Base class for binary threshold segmentation tools.
   *
   * Provides the common logic for single-threshold and upper/lower-threshold
   * segmentation tools. The tool computes a preview segmentation by applying
   * an ITK binary threshold filter to the reference image.
   *
   * Subclasses should set m_LockedUpperThreshold to control single vs. range mode.
   *
   * \ingroup ToolManagerEtAl
   * \sa BinaryThresholdTool, BinaryThresholdULTool, SegWithPreviewTool
   */
  class MITKSEGMENTATION_EXPORT BinaryThresholdBaseTool : public SegWithPreviewTool
  {
  public:
    /** \brief Emitted when the sensible threshold interval borders change.
     * Parameters: lower border, upper border, whether upper threshold is locked. */
    Message3<double, double, bool> IntervalBordersChanged;

    /** \brief Emitted when the current thresholding values change.
     * Parameters: lower threshold, upper threshold. */
    Message2<ScalarType, ScalarType> ThresholdingValuesChanged;

    mitkClassMacro(BinaryThresholdBaseTool, SegWithPreviewTool);

    /**
     * \brief Sets the lower and upper threshold values for the segmentation.
     * \param[in] lower The lower threshold value.
     * \param[in] upper The upper threshold value.
     */
    virtual void SetThresholdValues(double lower, double upper);

  protected:
    BinaryThresholdBaseTool(); // purposely hidden
    ~BinaryThresholdBaseTool() override;

    itkSetMacro(LockedUpperThreshold, bool);
    itkGetMacro(LockedUpperThreshold, bool);
    itkBooleanMacro(LockedUpperThreshold);

    itkGetMacro(SensibleMinimumThreshold, ScalarType);
    itkGetMacro(SensibleMaximumThreshold, ScalarType);

    void InitiateToolByInput() override;
    void DoUpdatePreview(const Image* inputAtTimeStep, const Image* oldSegAtTimeStep, MultiLabelSegmentation* previewImage, TimeStepType timeStep) override;

    template <typename TPixel, unsigned int VImageDimension>
    void ITKThresholding(const itk::Image<TPixel, VImageDimension>* inputImage,
                         MultiLabelSegmentation *segmentation,
                         unsigned int timeStep);

  private:
    ScalarType m_SensibleMinimumThreshold;
    ScalarType m_SensibleMaximumThreshold;
    ScalarType m_LowerThreshold;
    ScalarType m_UpperThreshold;

    /** Indicates if the tool should behave like a single threshold tool (true)
      or like a upper/lower threshold tool (false)*/
    bool m_LockedUpperThreshold = false;

  };

} // namespace

#endif
