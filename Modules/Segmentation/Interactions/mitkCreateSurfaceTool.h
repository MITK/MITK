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

#ifndef mitkCreateSurfaceTool_h_Included
#define mitkCreateSurfaceTool_h_Included

#include "mitkCommon.h"
#include "SegmentationExports.h"
#include "mitkSegmentationsProcessingTool.h"

namespace mitk
{

/**
  \brief Creates surface models from segmentations.
  \ingroup ToolManagerEtAl
  \sa mitk::Tool
  \sa QmitkInteractiveSegmentation

  Last contributor: $Author$
*/
class Segmentation_EXPORT CreateSurfaceTool : public SegmentationsProcessingTool
{
  public:

    mitkClassMacro(CreateSurfaceTool, SegmentationsProcessingTool);
    itkNewMacro(CreateSurfaceTool);

    virtual const char** GetXPM() const;
    virtual const char* GetName() const;

    void OnSurfaceCalculationDone();

  protected:

    virtual bool ProcessOneWorkingData( DataNode* node );
    virtual std::string GetErrorMessage();

    CreateSurfaceTool(); // purposely hidden
    virtual ~CreateSurfaceTool();
};

} // namespace

#endif

