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

#ifndef mitkAdaptiveRegionGrowing_h_Included
#define mitkAdaptiveRegionGrowing_h_Included

#include "mitkCommon.h"
#include "SegmentationExports.h"
#include "mitkAutoSegmentationTool.h"


namespace mitk
{

  /**
  \brief Dummy Tool for AdaptiveRegionGrowingWidget.

  \ingroup ToolManagerEtAl
  \sa mitk::Tool
  \sa QmitkInteractiveSegmentation

  */
  class Segmentation_EXPORT AdaptiveRegionGrowing : public AutoSegmentationTool
  {
  public:

    mitkClassMacro(AdaptiveRegionGrowing, AutoSegmentationTool);
    itkNewMacro(AdaptiveRegionGrowing);

    virtual const char** GetXPM() const;
    virtual const char* GetName() const;
    virtual std::string GetIconPath() const;

    virtual void Activated();
    virtual void Deactivated();


  protected:

    AdaptiveRegionGrowing(); // purposely hidden
    virtual ~AdaptiveRegionGrowing();

  };

} // namespace

#endif
