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
#include "SegmentationExports.h"

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
class Segmentation_EXPORT AddContourTool : public ContourTool
{
  public:

    mitkClassMacro(AddContourTool, ContourTool);
    itkNewMacro(AddContourTool);

    virtual const char** GetXPM() const;
    virtual const char* GetName() const;

  protected:

    AddContourTool(); // purposely hidden
    virtual ~AddContourTool();

};

} // namespace

#endif


