/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkUSNavigationStepZoneMarking_h
#define QmitkUSNavigationStepZoneMarking_h

#include "QmitkUSAbstractNavigationStep.h"

namespace itk {
template<class T> class SmartPointer;
}

namespace mitk {
class NodeDisplacementFilter;
class NavigationDataSource;
}

namespace Ui {
class QmitkUSNavigationStepZoneMarking;
}

/**
 * \brief Navigation step for marking risk structures.
 * The user can add risk structures by interacting with the render windows. The
 * risk structures are organized in an embedded table view.
 *
 * The risk structures are stored under DATANAME_BASENODE -> DATANAME_ZONES.
 *
 * This step is ready for the next step directly after activating. All actions
 * to be done in this step are optional.
 */
class QmitkUSNavigationStepZoneMarking : public QmitkUSAbstractNavigationStep
{
  Q_OBJECT

protected slots:
  void OnFreeze(bool freezed);

  /**
   * \brief Triggered when a risk zone was added.
   * Adds the zone to a member variable and to the node displacement filter.
   */
  void OnZoneAdded();

  /**
   * \brief Triggered when a risk zone was removed.
   * Removes the zone from a member variable and from the node displacement
   * filter.
   */
  void OnZoneRemoved();

  void OnShowListClicked(int state);

public:
  explicit QmitkUSNavigationStepZoneMarking(QWidget *parent = nullptr);
  ~QmitkUSNavigationStepZoneMarking() override;

  /**
   * \brief Initialization of the data storage nodes.
   * \return always true
   */
  bool OnStartStep() override;

  /**
   * \brief Resets widget and filter and removes nodes from the data storage.
   * \return always true
   */
  bool OnStopStep() override;

  /**
   * \brief There is nothing to be done.
   * \return always true
   */
  bool OnFinishStep() override;

  /**
   * \brief Selects input for the node displacement filter and emits "ReadyForNextStep" signal.
   * The input selection cannot be done earlier.
   * \return
   */
  bool OnActivateStep() override;

  bool OnDeactivateStep() override;

  /**
   * \brief Updates just the tracking validity status.
   */
  void OnUpdate() override;

  /**
   * The property "settings.interaction-concept" is used.
   */
  void OnSettingsChanged(const itk::SmartPointer<mitk::DataNode> settingsNode) override;

  QString GetTitle() override;

  /**
   * @return a node displacement filter for the zone surfaces
   */
  FilterVector GetFilter() override;

protected:
  void OnSetCombinedModality() override;

  void UpdateReferenceSensorName();

  itk::SmartPointer<mitk::NavigationDataSource>   m_NavigationDataSource;
  itk::SmartPointer<mitk::NodeDisplacementFilter> m_ZoneDisplacementFilter;
  std::vector<itk::SmartPointer<mitk::DataNode> > m_ZoneNodes;

  std::string                                     m_StateMachineFilename;
  std::string                                     m_ReferenceSensorName;
  unsigned int                                    m_ReferenceSensorIndex;

  bool                                            m_CurrentlyAddingZone;

private:
  Ui::QmitkUSNavigationStepZoneMarking *ui;
};

#endif
