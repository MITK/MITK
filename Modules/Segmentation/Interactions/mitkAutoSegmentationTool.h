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
#include "mitkOperation.h"
#include "mitkToolCommand.h"
#include "mitkSegTool3D.h"


namespace mitk
{

/**
  \brief Superclass for tools that create a new label without user interaction in render windows
*/
class Segmentation_EXPORT AutoSegmentationTool : public SegTool3D
{
  public:

    mitkClassMacro(AutoSegmentationTool, SegTool3D);

  protected:

    AutoSegmentationTool(); // purposely hidden
    virtual ~AutoSegmentationTool();

    virtual const char* GetGroup() const;
 };

} // namespace

#endif
