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
#include <MitkSegmentationExports.h>
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
class MITKSEGMENTATION_EXPORT CalculateVolumetryTool : public SegmentationsProcessingTool
{
  public:

    mitkClassMacro(CalculateVolumetryTool, SegmentationsProcessingTool);
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    virtual const char** GetXPM() const override;
    virtual const char* GetName() const override;

  protected:

    virtual bool ProcessOneWorkingData( DataNode* node ) override;
    virtual std::string GetErrorMessage() override;

    virtual void FinishProcessingAllData() override;

    CalculateVolumetryTool(); // purposely hidden
    virtual ~CalculateVolumetryTool();
};

} // namespace

#endif

