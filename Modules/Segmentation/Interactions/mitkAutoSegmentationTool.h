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

#ifndef mitkAutoSegmentationTool_h_Included
#define mitkAutoSegmentationTool_h_Included

#include "mitkCommon.h"
#include "SegmentationExports.h"
#include "mitkTool.h"

namespace mitk
{

/**
  \brief Superclass for tool that create a new segmentation without user interaction in render windows

  This class is undocumented. Ask the creator ($Author$) to supply useful comments.
*/
class Segmentation_EXPORT AutoSegmentationTool : public Tool
{
  public:

    mitkClassMacro(AutoSegmentationTool, Tool);

  protected:

    AutoSegmentationTool(); // purposely hidden
    virtual ~AutoSegmentationTool();

    virtual const char* GetGroup() const;
 };

} // namespace

#endif

