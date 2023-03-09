/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkUSNavigationStepPunctuationIntervention_h
#define QmitkUSNavigationStepPunctuationIntervention_h

#include "QmitkUSAbstractNavigationStep.h"
#include <mitkPointSet.h>
#include <mitkNavigationTool.h>
#include <vtkSmartPointer.h>
#include <vtkSphereSource.h>
#include <vtkLine.h>
#include <vtkOBBTree.h>

namespace mitk {
class DataNode;
class NeedleProjectionFilter;
class NodeDisplacementFilter;
class USNavigationGrabCutSegmentationUpdateFilter;
class USNavigationTargetUpdateFilter;
class USNavigationTargetOcclusionFilter;
class USPointMarkInteractor;
class LookupTableProperty;
class Surface;
}

namespace Ui {
class QmitkUSNavigationStepPunctuationIntervention;
}

class QmitkZoneProgressBar;

/**
 * \brief Navigations step for the actual punctuation intervention.
 * The needle path is projected onto the image plane and the distances to all
 * risk structures are displayed in the widget.
 *
 * The risk structures are expected to be in the data storage under
 * DATANAME_BASENODE -> DATANAME_ZONES. They should contain a property named
 * "zone.size" and it is assumed that they are spherical.
 */
class QmitkUSNavigationStepPunctuationIntervention : public QmitkUSAbstractNavigationStep
{
  Q_OBJECT

public:

  /** Sets the navigation tool of the needle for the meta data (tool axis etc.)*/
  void SetNeedleMetaData(mitk::NavigationTool::Pointer needleNavigationTool);
  explicit QmitkUSNavigationStepPunctuationIntervention(QWidget *parent = nullptr);
  ~QmitkUSNavigationStepPunctuationIntervention() override;

  bool OnStartStep() override;
  bool OnRestartStep() override;
  bool OnFinishStep() override;

  bool OnActivateStep() override;
  void OnUpdate() override;

  void OnSettingsChanged(const itk::SmartPointer<mitk::DataNode>) override;

  QString GetTitle() override;
  bool GetIsRestartable() override;

  FilterVector GetFilter() override;

signals:

  void AddAblationZoneClicked(int);

  void AblationZoneChanged(int,int);

protected slots:

  void OnAddAblationZoneClicked();
  void OnEnableAblationZoneMarkingClicked();
  void OnAblationZoneSizeSliderChanged(int size);
  void OnShowToolAxisEnabled(int enabled);

protected:
  void OnSetCombinedModality() override;

  void ClearZones();
  void UpdateBodyMarkerStatus(mitk::NavigationData::Pointer bodyMarker);
  /** Updates the critical structures which means that the distance to the needle tip is updated
      and also the color changes to red if the path projection intersects the critical structure. */
  void UpdateCriticalStructures(mitk::NavigationData::Pointer needle, mitk::PointSet::Pointer path);
  /** Checks if the given line intersects the given sphere. */
  bool CheckSphereLineIntersection(mitk::Point3D& sphereOrigin, float& sphereRadius, mitk::Point3D& lineStart, mitk::Point3D& lineEnd);

private:
  Ui::QmitkUSNavigationStepPunctuationIntervention *m_Ui;

  mitk::DataStorage::SetOfObjects::ConstPointer m_ZoneNodes;

  /** \brief Creates a Pointset that projects the needle's path */
  itk::SmartPointer<mitk::NeedleProjectionFilter> m_NeedleProjectionFilter;

  /** holds the navigation tool of the needle for the meta data (tool axis etc.)*/
  mitk::NavigationTool::Pointer m_NeedleNavigationTool;

  std::map<mitk::DataNode::Pointer,mitk::Color> m_OldColors; //stores the original color of the critical structrue nodes

  //some help variables for the CheckSphereLineIntersection()-Method
  vtkSmartPointer<vtkSphereSource> m_SphereSource;
  vtkSmartPointer<vtkOBBTree> m_OBBTree;
  vtkSmartPointer<vtkPoints> m_IntersectPoints;
};

#endif
