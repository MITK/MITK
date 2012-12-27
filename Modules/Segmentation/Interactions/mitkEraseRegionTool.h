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

#ifndef mitkEraseRegionTool_h_Included
#define mitkEraseRegionTool_h_Included

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
  the inside pixels to 0 (erasing a segmentation).

  \warning Only to be instantiated by mitk::ToolManager.

  $Author$
*/
class Segmentation_EXPORT EraseRegionTool : public SetRegionTool
{
  public:

    mitkClassMacro(EraseRegionTool, SetRegionTool);
    itkNewMacro(EraseRegionTool);

    virtual const char** GetXPM() const;
    virtual const char* GetName() const;

  protected:

    EraseRegionTool(); // purposely hidden
    virtual ~EraseRegionTool();

};

} // namespace

#endif


