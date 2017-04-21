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

#ifndef QMITKUSNAVIGATIONSTEPPLACEMENTPLANNING_H
#define QMITKUSNAVIGATIONSTEPPLACEMENTPLANNING_H

#include "QmitkUSAbstractNavigationStep.h"

namespace Ui
{
  class QmitkUSNavigationStepPlacementPlanning;
}

namespace mitk
{
  class USNavigationTargetUpdateFilter;
  class USNavigationTargetIntersectionFilter;
  class USPointMarkInteractor;
  class NodeDisplacementFilter;
  class NeedleProjectionFilter;
  class LookupTableProperty;
  class TextAnnotation3D;
  class Surface;
  class PointSet;
  class USTargetPlacementQualityCalculator;
}

/**
 * \brief Navigation step for planning the positions for implanting markers.
 *
 * The planned targets are stored in the data storage under DATANAME_BASENODE ->
 * DATANAME_TARGETS and the needle path to the planned targets are stored under
 * DATANAME_BASENODE -> DATANAME_TARGETS_PATHS. The target structure is expected
 * to be stored under DATANAME_BASENODE -> DATANAME_TUMOR ->
 * DATANAME_TARGETSURFACE.
 *
 */
class QmitkUSNavigationStepPlacementPlanning : public QmitkUSAbstractNavigationStep
{
  Q_OBJECT

protected slots:
  /**
   * \brief Freezes or unfreezes the combined modality.
   * In freeze state an interactor is activated in the render window, so the
   * position of the currently active target can be planned by clicking into the
   * render window.
   */
  void OnFreeze(bool freezed);

  /**
   * \brief Plan target position at the intersection between needle path and target surface.
   */
  void OnPlaceTargetButtonClicked();

  /**
   * \brief Selects the previous target as active target.
   */
  void OnGoToPreviousTarget();

  /**
   * \brief Selects the next target as active target.
   */
  void OnGoToNextTarget();

  /**
   * \brief The currently active target is removed from the data storage.
   */
  void OnRemoveCurrentTargetClicked();

public:
  explicit QmitkUSNavigationStepPlacementPlanning(QWidget *parent = 0);
  ~QmitkUSNavigationStepPlacementPlanning();

  virtual QString GetTitle();
  virtual FilterVector GetFilter();

protected:
  virtual bool OnStartStep();
  virtual bool OnStopStep();
  virtual bool OnRestartStep();
  virtual bool OnFinishStep();

  virtual bool OnActivateStep();
  virtual bool OnDeactivateStep();
  virtual void OnUpdate();

  virtual void OnSettingsChanged(const itk::SmartPointer<mitk::DataNode> settingsNode);
  virtual void OnSetCombinedModality();

  void CreateTargetNodesIfNecessary();
  void UpdateTargetCoordinates(mitk::DataNode *);
  void UpdateTargetColors();
  void UpdateTargetDescriptions();
  void GenerateTargetColorLookupTable();
  void ReinitNodeDisplacementFilter();
  void CalculatePlanningQuality();
  itk::SmartPointer<mitk::DataNode> CalculatePlanningQuality(itk::SmartPointer<mitk::Surface> targetSurface,
                                                             itk::SmartPointer<mitk::PointSet>);
  itk::SmartPointer<mitk::Surface> CreateSphere(float radius);
  void UpdateBodyMarkerStatus(mitk::NavigationData::Pointer bodyMarker);
  void UpdateSensorsNames();

  int m_NumberOfTargets;
  int m_CurrentTargetIndex;
  bool m_BodyMarkerValid;

  itk::SmartPointer<mitk::USPointMarkInteractor> m_PointMarkInteractor;
  itk::SmartPointer<mitk::USNavigationTargetUpdateFilter> m_TargetUpdateFilter;
  itk::SmartPointer<mitk::NodeDisplacementFilter> m_NodeDisplacementFilter;
  itk::SmartPointer<mitk::NeedleProjectionFilter> m_NeedleProjectionFilter;
  itk::SmartPointer<mitk::USNavigationTargetIntersectionFilter> m_TargetIntersectionFilter;
  itk::SmartPointer<mitk::USTargetPlacementQualityCalculator> m_PlacementQualityCalculator;

  itk::SmartPointer<mitk::LookupTableProperty> m_TargetColorLookupTableProperty;

  itk::SmartPointer<mitk::DataNode> m_TargetNode;
  QVector<itk::SmartPointer<mitk::DataNode>> m_PlannedTargetNodes;
  QVector<itk::SmartPointer<mitk::DataNode>> m_PlannedNeedlePaths;

  itk::SmartPointer<mitk::TextAnnotation3D> m_CurrentTargetNodeOverlay;

  std::string m_ReferenceSensorName;
  std::string m_NeedleSensorName;

  unsigned int m_ReferenceSensorIndex;
  unsigned int m_NeedleSensorIndex;

private:
  mitk::MessageDelegate1<QmitkUSNavigationStepPlacementPlanning, mitk::DataNode *> m_ListenerTargetCoordinatesChanged;

  Ui::QmitkUSNavigationStepPlacementPlanning *ui;
};

#endif // QMITKUSNAVIGATIONSTEPPLACEMENTPLANNING_H
