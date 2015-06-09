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

#ifndef mitkPaintContourTool_h_Included
#define mitkPaintContourTool_h_Included

#include "mitkPaintbrushTool.h"
#include <MitkSegmentationExports.h>

namespace us {
class ModuleResource;
}

namespace mitk
{

 /**
  \brief Paintbrush tool for InteractiveSegmentation

  \sa FeedbackContourTool
  \sa ExtractImageFilter
  \sa OverwriteSliceImageFilter

  \ingroup Interaction
  \ingroup ToolManagerEtAl

  Simple paintbrush drawing tool. Right now there are only circular pens of varying size.

  This class specified only the drawing "color" for the super class PaintbrushTool.

  \warning Only to be instantiated by mitk::ToolManager.
  $Author: maleike $
*/
class MITKSEGMENTATION_EXPORT DrawPaintbrushTool : public PaintbrushTool
{
  public:

    mitkClassMacro(DrawPaintbrushTool, PaintbrushTool);
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    virtual const char** GetXPM() const override;
    virtual us::ModuleResource GetCursorIconResource() const override;
    us::ModuleResource GetIconResource() const override;

    virtual const char* GetName() const override;

  protected:

    DrawPaintbrushTool(); // purposely hidden
    virtual ~DrawPaintbrushTool();

};

} // namespace

#endif

