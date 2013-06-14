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

#ifndef mitkAdaptiveRegionGrowingTool_h_Included
#define mitkAdaptiveRegionGrowingTool_h_Included

#include "mitkCommon.h"
#include "SegmentationExports.h"
#include "mitkAutoSegmentationTool.h"
#include "mitkDataStorage.h"

namespace mitk
{

  /**
  \brief Dummy Tool for AdaptiveRegionGrowingToolGUI to get Tool functionality for AdaptiveRegionGrowing.
  The actual logic is implemented in QmitkAdaptiveRegionGrowingToolGUI.

  \ingroup ToolManagerEtAl
  \sa mitk::Tool
  \sa QmitkInteractiveSegmentation

  */
  class Segmentation_EXPORT AdaptiveRegionGrowingTool : public AutoSegmentationTool
  {
  public:

    mitkClassMacro(AdaptiveRegionGrowingTool, AutoSegmentationTool);
    itkNewMacro(AdaptiveRegionGrowingTool);

    virtual const char** GetXPM() const;
    virtual const char* GetName() const;
    virtual std::string GetIconPath() const;

    virtual void Activated();
    virtual void Deactivated();

    mitk::DataNode* GetReferenceData();
    mitk::DataNode* GetWorkingData();
    mitk::DataStorage* GetDataStorage();


  protected:

    AdaptiveRegionGrowingTool(); // purposely hidden
    virtual ~AdaptiveRegionGrowingTool();

  };

} // namespace

#endif
