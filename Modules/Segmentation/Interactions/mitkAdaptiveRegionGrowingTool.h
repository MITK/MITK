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
#include <MitkSegmentationExports.h>
#include "mitkAutoSegmentationTool.h"
#include "mitkDataStorage.h"
#include "mitkSinglePointDataInteractor.h"
#include "mitkPointSet.h"

namespace us {
class ModuleResource;
}

namespace mitk
{

  /**
  \brief Dummy Tool for AdaptiveRegionGrowingToolGUI to get Tool functionality for AdaptiveRegionGrowing.
  The actual logic is implemented in QmitkAdaptiveRegionGrowingToolGUI.

  \ingroup ToolManagerEtAl
  \sa mitk::Tool
  \sa QmitkInteractiveSegmentation

  */
  class MitkSegmentation_EXPORT AdaptiveRegionGrowingTool : public AutoSegmentationTool
  {
  public:

    mitkClassMacro(AdaptiveRegionGrowingTool, AutoSegmentationTool);
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    virtual const char** GetXPM() const;
    virtual const char* GetName() const;
    us::ModuleResource GetIconResource() const;


    virtual void Activated();
    virtual void Deactivated();

    virtual DataNode::Pointer GetPointSetNode();

    mitk::DataNode* GetReferenceData();
    mitk::DataNode* GetWorkingData();
    mitk::DataStorage* GetDataStorage();


  protected:

    AdaptiveRegionGrowingTool(); // purposely hidden
    virtual ~AdaptiveRegionGrowingTool();

  private:
    PointSet::Pointer m_PointSet;
    SinglePointDataInteractor::Pointer m_SeedPointInteractor;
    DataNode::Pointer m_PointSetNode;


  };

} // namespace

#endif
