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

#ifndef mitkAddContourTool_h_Included
#define mitkAddContourTool_h_Included

#include "mitkContourTool.h"
#include <MitkSegmentationExports.h>

namespace us {
class ModuleResource;
}

namespace mitk
{

/**
  \brief Fill the inside of a contour with 1

  \sa ContourTool

  \ingroup Interaction
  \ingroup ToolManagerEtAl

  Fills a visible contour (from FeedbackContourTool) during mouse dragging. When the mouse button
  is released, AddContourTool tries to extract a slice from the working image and fill in
  the (filled) contour as a binary image. All inside pixels are set to 1.

  While holding the CTRL key, the contour changes color and the pixels on the inside would be
  filled with 0.

  \warning Only to be instantiated by mitk::ToolManager.

  $Author$
*/
class MITKSEGMENTATION_EXPORT AddContourTool : public ContourTool
{
  public:

    mitkClassMacro(AddContourTool, ContourTool);
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    virtual const char** GetXPM() const override;
    virtual us::ModuleResource GetCursorIconResource() const override;
    us::ModuleResource GetIconResource() const override;

    virtual const char* GetName() const override;

  protected:

    AddContourTool(); // purposely hidden
    virtual ~AddContourTool();

};

} // namespace

#endif


