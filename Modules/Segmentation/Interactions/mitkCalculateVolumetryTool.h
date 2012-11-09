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

#ifndef mitkCalculateVolumetryTool_h_Included
#define mitkCalculateVolumetryTool_h_Included

#include "mitkCommon.h"
#include "SegmentationExports.h"
#include "mitkSegmentationsProcessingTool.h"

namespace mitk
{

/**
  \brief Calculates the segmented volumes for binary images.

  \ingroup ToolManagerEtAl
  \sa mitk::Tool
  \sa QmitkInteractiveSegmentation

  Last contributor: $Author$
*/
class Segmentation_EXPORT CalculateVolumetryTool : public SegmentationsProcessingTool
{
  public:

    mitkClassMacro(CalculateVolumetryTool, SegmentationsProcessingTool);
    itkNewMacro(CalculateVolumetryTool);

    virtual const char** GetXPM() const;
    virtual const char* GetName() const;

  protected:

    virtual bool ProcessOneWorkingData( DataNode* node );
    virtual std::string GetErrorMessage();

    virtual void FinishProcessingAllData();

    CalculateVolumetryTool(); // purposely hidden
    virtual ~CalculateVolumetryTool();
};

} // namespace

#endif

