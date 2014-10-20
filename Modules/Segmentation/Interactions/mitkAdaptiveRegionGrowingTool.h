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

    /**
     * @brief mitkClassMacro
     */
    mitkClassMacro(AdaptiveRegionGrowingTool, AutoSegmentationTool);
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    bool CanHandle(BaseData* referenceData) const;

    /**
     * @brief Get XPM
     * @return NULL
     */
    virtual const char** GetXPM() const;

    /**
     * @brief Get name
     * @return name of the Tool
     */
    virtual const char* GetName() const;

    /**
     * @brief Get icon resource
     * @return the resource Object of the Icon
     */
    us::ModuleResource GetIconResource() const;

    /**
     * @brief Adds interactor for the seedpoint and creates a seedpoint if neccessary.
     *
     *
     */
    virtual void Activated();

    /**
     * @brief Removes all set points and interactors.
     *
     *
     */
    virtual void Deactivated();

    /**
     * @brief get pointset node
     * @return the point set node
     */
    virtual DataNode::Pointer GetPointSetNode();

    /**
     * @brief get reference data
     * @return the current reference data.
     */
    mitk::DataNode* GetReferenceData();

    /**
     * @brief Get working data
     * @return a list of all working data.
     */
    mitk::DataNode* GetWorkingData();

    /**
     * @brief Get datastorage
     * @return the current data storage.
     */
    mitk::DataStorage* GetDataStorage();


  protected:

    /**
     * @brief constructor
     */
    AdaptiveRegionGrowingTool(); // purposely hidden

    /**
     * @brief destructor
     */
    virtual ~AdaptiveRegionGrowingTool();

  private:
    PointSet::Pointer m_PointSet;
    SinglePointDataInteractor::Pointer m_SeedPointInteractor;
    DataNode::Pointer m_PointSetNode;


  };

} // namespace

#endif
