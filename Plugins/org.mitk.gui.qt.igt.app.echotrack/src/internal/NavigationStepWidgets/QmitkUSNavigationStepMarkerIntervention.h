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

#ifndef QMITKUSNAVIGATIONSTEPMARKERINTERVENTION_H
#define QMITKUSNAVIGATIONSTEPMARKERINTERVENTION_H

#include "QmitkUSAbstractNavigationStep.h"

namespace mitk
{
  class DataNode;
  class NeedleProjectionFilter;
  class NodeDisplacementFilter;
  class USNavigationGrabCutSegmentationUpdateFilter;
  class USNavigationTargetUpdateFilter;
  class USNavigationTargetOcclusionFilter;
  class USNavigationTargetIntersectionFilter;
  class USPointMarkInteractor;
  class LookupTableProperty;
  class Surface;
  class TextAnnotation2D;
  class USTargetPlacementQualityCalculator;
}

namespace Ui
{
  class QmitkUSNavigationStepMarkerIntervention;
}

class QmitkZoneProgressBar;

/**
 * \brief Navigation step for the actual marker placement.
 */
class QmitkUSNavigationStepMarkerIntervention : public QmitkUSAbstractNavigationStep
{
  Q_OBJECT

signals:
  void TargetReached(int);
  void TargetLeft(int);

protected slots:
  void OnTargetReached();
  void OnTargetLeft();
  void OnBackToLastTargetClicked();
  void OnFreeze(bool);
  void OnShowPlanningView(bool);
  void OnRiskZoneViolated(const mitk::DataNode *, mitk::Point3D);

public:
  explicit QmitkUSNavigationStepMarkerIntervention(QWidget *parent = 0);
  ~QmitkUSNavigationStepMarkerIntervention();

  virtual bool OnStartStep();
  virtual bool OnStopStep();
  virtual bool OnFinishStep();

  virtual bool OnActivateStep();
  virtual bool OnDeactivateStep();
  virtual void OnUpdate();

  virtual void OnSettingsChanged(const itk::SmartPointer<mitk::DataNode> settingsNode);

  virtual QString GetTitle();
  virtual bool GetIsRestartable();

  virtual FilterVector GetFilter();

protected:
  virtual void OnSetCombinedModality();

  void ClearZones();
  void UpdateTargetCoordinates(mitk::DataNode *);
  void UpdateBodyMarkerStatus(mitk::NavigationData::Pointer bodyMarker);
  void GenerateTargetColorLookupTable();
  void UpdateTargetColors();

  /**
   * \brief
   */
  void UpdateTargetScore();

  /**
   * \brief Updates display showing the number of the currently active target.
   */
  void UpdateTargetProgressDisplay();

  /**
   * \brief Updates color and distance of the progress bar for the planned target.
   * The intersection between needle path and target surface is calculated and
   * the color is got from the intersection point.
   */
  void UpdatePlannedTargetProgressDisplay();

  /**
   * \brief Tests for target violation and updates the display accordingly.
   * This method tests if the needle tip is inside the target surface.
   */
  void UpdateTargetViolationStatus();

  /**
   * \brief Calculates and display quality metrics if all three markers are placed.
   */
  void CalculateTargetPlacementQuality();

  void UpdateSensorsNames();

  unsigned int m_NumberOfTargets;

  QVector<itk::SmartPointer<mitk::DataNode>> m_PlannedTargetsNodes;
  QVector<itk::SmartPointer<mitk::DataNode>> m_ReachedTargetsNodes;

  QmitkZoneProgressBar *m_TargetProgressBar;
  QmitkZoneProgressBar *m_PlannedTargetProgressBar;

  int m_CurrentTargetIndex;
  bool m_CurrentTargetReached;

  mitk::ScalarType m_ActiveTargetColor[3];
  mitk::ScalarType m_InactiveTargetColor[3];
  mitk::ScalarType m_ReachedTargetColor[3];

  bool m_ShowPlanningColors;
  itk::SmartPointer<mitk::USPointMarkInteractor> m_PointMarkInteractor;

  itk::SmartPointer<mitk::DataNode> m_TargetNode;
  itk::SmartPointer<mitk::LookupTableProperty> m_TargetColorLookupTableProperty;

  itk::SmartPointer<mitk::Surface> m_TargetSurface;

  itk::SmartPointer<mitk::NeedleProjectionFilter> m_NeedleProjectionFilter;
  itk::SmartPointer<mitk::NodeDisplacementFilter> m_NodeDisplacementFilter;

  itk::SmartPointer<mitk::USNavigationTargetUpdateFilter> m_TargetUpdateFilter;
  itk::SmartPointer<mitk::USNavigationTargetOcclusionFilter> m_TargetOcclusionFilter;
  itk::SmartPointer<mitk::USNavigationTargetIntersectionFilter> m_TargetIntersectionFilter;
  itk::SmartPointer<mitk::USTargetPlacementQualityCalculator> m_PlacementQualityCalculator;

  itk::SmartPointer<mitk::TextAnnotation2D> m_TargetStructureWarnOverlay;

  std::string m_ReferenceSensorName;
  std::string m_NeedleSensorName;

  unsigned int m_ReferenceSensorIndex;
  unsigned int m_NeedleSensorIndex;

private:
  mitk::MessageDelegate1<QmitkUSNavigationStepMarkerIntervention, mitk::DataNode *> m_ListenerTargetCoordinatesChanged;

  Ui::QmitkUSNavigationStepMarkerIntervention *ui;
};

#endif // QMITKUSNAVIGATIONSTEPMARKERINTERVENTION_H
