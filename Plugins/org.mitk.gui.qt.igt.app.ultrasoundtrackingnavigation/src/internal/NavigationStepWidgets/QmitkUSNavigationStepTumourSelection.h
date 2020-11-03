/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QMITKUSNAVIGATIONSTEPTUMOURSELECTION_H
#define QMITKUSNAVIGATIONSTEPTUMOURSELECTION_H

#include "QmitkUSAbstractNavigationStep.h"

namespace itk {
template<class T> class SmartPointer;
}

namespace mitk {
class NavigationDataSource;
class DataStorage;
class DataNode;
class USZonesInteractor;
class NodeDisplacementFilter;
class Surface;
}

namespace Ui {
class QmitkUSNavigationStepTumourSelection;
}

class USNavigationMarkerPlacement;
class QmitkUSNavigationStepCombinedModality;

/**
 * \brief Navigation step for marking the tumor position and extent.
 * The user can mark the position by interacting with the render windows. The
 * tumor size can be changed afterwards and the tumor can be removed.
 */
class QmitkUSNavigationStepTumourSelection : public QmitkUSAbstractNavigationStep
{
  Q_OBJECT

protected slots:
  /**
   * \brief Activates or deactivates the ineractor for tumour creation.
   */
  void OnFreeze(bool freezed);

  /**
   * \brief Updates the surface of the tumor node according to the new size.
   */
  void OnTumourSizeChanged(int);

  /**
   * \brief Just restarts the navigation step for deleting the tumour.
   */
  void OnDeleteButtonClicked();

public:
  explicit QmitkUSNavigationStepTumourSelection(QWidget* parent = nullptr);
  ~QmitkUSNavigationStepTumourSelection() override;

  void SetTargetSelectionOptional (bool t);

  /**
   * \brief Initializes tumour and target surface.
   * \return always true
   */
  bool OnStartStep() override;

  /**
   * \brief Removes target surface and tumour node from the data storage.
   * Additionally an unfreeze is done and the node displacement filter is
   * resetted.
   * \return always true
   */
  bool OnStopStep() override;

  /**
   * \brief Reinitializes buttons and sliders in addition of calling the default implementation.
   * \return result of the superclass implementation
   */
  bool OnRestartStep() override;

  /**
   * \brief (Re)creates the target surface.
   * \return always true
   */
  bool OnFinishStep() override;

  /**
   * \brief Initializes (but not activates) the interactor for tumour selection.
   * \return always true
   */
  bool OnActivateStep() override;

  /**
   * \brief Deactivates the interactor for tumour selection
   * and removes data of the tumour node if selection wasn't finished yet.
   *
   * \return always true
   */
  bool OnDeactivateStep() override;

  /**
   * \brief Updates tracking validity status and checks tumour node for the end of tumour creation.
   */
  void OnUpdate() override;

  /**
   * The properties "settings.security-distance" and
   * "settings.interaction-concept" are used.
   */
  void OnSettingsChanged(const itk::SmartPointer<mitk::DataNode> settingsNode) override;

  QString GetTitle() override;

  /**
   * @return a node displacement filter for tumour and target surfaces
   */
  FilterVector GetFilter() override;

  void SetTumorColor(mitk::Color c);

  /** @return Returns the current NodeDisplacementFilter which ist used for updating the targets pose. */
  itk::SmartPointer<mitk::NodeDisplacementFilter> GetTumourNodeDisplacementFilter();

protected:
  void OnSetCombinedModality() override;

  void TumourNodeChanged(const mitk::DataNode*);
  itk::SmartPointer<mitk::Surface> CreateTargetSurface();

  void UpdateReferenceSensorName();

  itk::SmartPointer<mitk::NavigationDataSource>   m_NavigationDataSource;

  bool m_targetSelectionOptional;

  float                                           m_SecurityDistance;

  itk::SmartPointer<mitk::USZonesInteractor>      m_Interactor;
  itk::SmartPointer<mitk::DataNode>               m_TumourNode;
  itk::SmartPointer<mitk::DataNode>               m_TargetSurfaceNode;

  itk::SmartPointer<mitk::NodeDisplacementFilter> m_NodeDisplacementFilter;

  std::string                                     m_StateMachineFilename;
  std::string                                     m_ReferenceSensorName;
  unsigned int                                    m_ReferenceSensorIndex;

  mitk::Color                                     m_SphereColor;

private:
  mitk::MessageDelegate1<QmitkUSNavigationStepTumourSelection, const mitk::DataNode*> m_ListenerChangeNode;

  Ui::QmitkUSNavigationStepTumourSelection *ui;
};

#endif // QMITKUSNAVIGATIONSTEPTUMOURSELECTION_H
