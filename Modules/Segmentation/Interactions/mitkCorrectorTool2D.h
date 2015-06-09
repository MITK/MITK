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

#ifndef mitkCorrectorTool2D_h_Included
#define mitkCorrectorTool2D_h_Included

#include "mitkCommon.h"
#include <MitkSegmentationExports.h>
#include "mitkFeedbackContourTool.h"

namespace us {
class ModuleResource;
}

namespace mitk
{

class Image;
class StateMachineAction;
class InteractionEvent;

/**
  \brief Corrector tool for 2D binary segmentations

  \sa FeedbackContourTool
  \sa ExtractImageFilter
  \sa OverwriteSliceImageFilter

  \ingroup Interaction
  \ingroup ToolManagerEtAl

  Lets the user draw a (multi-point) line and intelligently decides what to do. The underlying algorithm
  tests if the line begins and ends inside or outside a segmentation and either adds or subtracts a piece of
  segmentation.

  Algorithm is implemented in CorrectorAlgorithm (so that it could be reimplemented in a more modern fashion some time).

  \warning Only to be instantiated by mitk::ToolManager.

  $Author$
*/
class MITKSEGMENTATION_EXPORT CorrectorTool2D : public FeedbackContourTool
{
  public:

    mitkClassMacro(CorrectorTool2D, FeedbackContourTool);
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    virtual const char** GetXPM() const override;
    virtual us::ModuleResource GetCursorIconResource() const override;
    us::ModuleResource GetIconResource() const override;

    virtual const char* GetName() const override;

  protected:

    CorrectorTool2D(int paintingPixelValue = 1); // purposely hidden
    virtual ~CorrectorTool2D();

    void ConnectActionsAndFunctions() override;

    virtual void Activated() override;
    virtual void Deactivated() override;

    virtual bool OnMousePressed ( StateMachineAction*, InteractionEvent* );
    virtual bool OnMouseMoved   ( StateMachineAction*, InteractionEvent* );
    virtual bool OnMouseReleased( StateMachineAction*, InteractionEvent* );
    int m_PaintingPixelValue;

    Image::Pointer m_WorkingSlice;
};

} // namespace

#endif


