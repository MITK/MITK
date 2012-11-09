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

#ifndef mitkFillRegionTool_h_Included
#define mitkFillRegionTool_h_Included

#include "mitkSetRegionTool.h"
#include "SegmentationExports.h"

namespace mitk
{

/**
  \brief Fill the inside of a contour with 1

  \sa SetRegionTool

  \ingroup Interaction
  \ingroup ToolManagerEtAl

  Finds the outer contour of a shape in 2D (possibly including holes) and sets all
  the inside pixels to 1, filling holes in a segmentation.
  \warning Only to be instantiated by mitk::ToolManager.

  $Author$
*/
class Segmentation_EXPORT FillRegionTool : public SetRegionTool
{
  public:

    mitkClassMacro(FillRegionTool, SetRegionTool);
    itkNewMacro(FillRegionTool);

    virtual const char** GetXPM() const;
    virtual const char* GetName() const;

  protected:

    FillRegionTool(); // purposely hidden
    virtual ~FillRegionTool();

};

} // namespace

#endif


