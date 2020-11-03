/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkAdaptiveRegionGrowingTool_h_Included
#define mitkAdaptiveRegionGrowingTool_h_Included

#include "mitkAutoSegmentationTool.h"
#include "mitkCommon.h"
#include "mitkDataStorage.h"
#include "mitkPointSet.h"
#include "mitkSinglePointDataInteractor.h"
#include <MitkSegmentationExports.h>

namespace us
{
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
  class MITKSEGMENTATION_EXPORT AdaptiveRegionGrowingTool : public AutoSegmentationTool
  {
  public:
    /**
     * @brief mitkClassMacro
     */
    mitkClassMacro(AdaptiveRegionGrowingTool, AutoSegmentationTool);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    bool CanHandle(const BaseData* referenceData, const BaseData* workingData) const override;

    /**
     * @brief Get XPM
     * @return nullptr
     */
    const char **GetXPM() const override;

    /**
     * @brief Get name
     * @return name of the Tool
     */
    const char *GetName() const override;

    /**
     * @brief Get icon resource
     * @return the resource Object of the Icon
     */
    us::ModuleResource GetIconResource() const override;

    /**
     * @brief Adds interactor for the seedpoint and creates a seedpoint if neccessary.
     *
     *
     */
    void Activated() override;

    /**
     * @brief Removes all set points and interactors.
     *
     *
     */
    void Deactivated() override;

    /**
     * @brief get pointset node
     * @return the point set node
     */
    virtual DataNode::Pointer GetPointSetNode();

    /**
     * @brief get reference data
     * @return the current reference data.
     */
    mitk::DataNode *GetReferenceData();

    /**
     * @brief Get working data
     * @return a list of all working data.
     */
    mitk::DataNode *GetWorkingData();

    /**
     * @brief Get datastorage
     * @return the current data storage.
     */
    mitk::DataStorage *GetDataStorage();

    void ConfirmSegmentation();

  protected:
    /**
     * @brief constructor
     */
    AdaptiveRegionGrowingTool(); // purposely hidden

    /**
     * @brief destructor
     */
    ~AdaptiveRegionGrowingTool() override;

  private:
    PointSet::Pointer m_PointSet;
    SinglePointDataInteractor::Pointer m_SeedPointInteractor;
    DataNode::Pointer m_PointSetNode;
  };

} // namespace

#endif
