/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkFillRegionBaseTool_h
#define mitkFillRegionBaseTool_h

#include <mitkCommon.h>
#include <mitkContourModelUtils.h>
#include <mitkImage.h>
#include <mitkSegTool2D.h>
#include <MitkSegmentationExports.h>

#include <mitkDataNode.h>

#include <mitkImageCast.h>

namespace mitk
{
  /**
    \brief Base class for tools that fill a connected region of a 2D slice

    \sa Tool

    \ingroup Interaction
    \ingroup ToolManagerEtAl

    \warning Only to be instantiated by mitk::ToolManager.

    $Author: nolden $
  */
  class MITKSEGMENTATION_EXPORT FillRegionBaseTool : public SegTool2D
  {
  public:
    mitkClassMacro(FillRegionBaseTool, SegTool2D);

    /** \brief A single click triggers a connected-component flood/morphology that decides the
     * affected region, so Fill/Erase/Close are SEMIAUTOMATIC (covers all derived tools). */
    mitk::Label::AlgorithmType GetAlgorithmType() const override;

  protected:
    FillRegionBaseTool();             // purposely hidden
    ~FillRegionBaseTool() override;

    void ConnectActionsAndFunctions() override;

    /// \brief Add a control point and finish current segment.
    virtual void OnClick(StateMachineAction*, InteractionEvent* interactionEvent);

    /**
     * \brief Generates the mask image indicating which pixels should be filled.
     *
     * Filled pixels have value 1, untouched pixels have value 0.
     * The default implementation marks the connected region around the seed point
     * that has the same pixel value/label as the seed point.
     *
     * Reimplement this to change the fill region strategy.
     *
     * \param[in] workingSlice Part of the segmentation image used to determine the fill region.
     * \param[in] seedPoint The world coordinate where the user clicked.
     * \param[out] seedLabelValue Receives the label value at the seed point.
     * \return The binary fill mask, or nullptr if there is nothing to fill.
     */
    virtual Image::Pointer GenerateFillImage(const Image* workingSlice, Point3D seedPoint, mitk::Label::PixelType& seedLabelValue) const;

    /** Function that is called by OnClick before the filling is executed. If you want to do special
    * preparation (e.g. change m_FillLabelValue, you can overwrite this function. */
    virtual void PrepareFilling(const Image* workingSlice, Point3D seedPoint) = 0;

    /** \brief The modified label is the one written by the fill (e.g. the active label for Fill, the
     * seed label for Close); when filling with UNLABELED (Erase) the modified label is the clicked
     * seed label. This deviates from the active label, so the base default is overridden here. */
    MultiLabelSegmentation::LabelValueVectorType GetAffectedLabelValues() const override;

    Label::PixelType m_FillLabelValue = 0;
    Label::PixelType m_SeedLabelValue = 0;

    MultiLabelSegmentation::MergeStyle m_MergeStyle = MultiLabelSegmentation::MergeStyle::Replace;
  private:
  };

} // namespace

#endif
