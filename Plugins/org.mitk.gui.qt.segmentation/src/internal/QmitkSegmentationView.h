/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QMITKSEGMENTATIONVIEW_H
#define QMITKSEGMENTATIONVIEW_H

#include <QmitkAbstractView.h>
#include <mitkILifecycleAwarePart.h>
#include <mitkIRenderWindowPartListener.h>

#include <berryIBerryPreferences.h>

#include "ui_QmitkSegmentationControls.h"

class QmitkRenderWindow;

/**
* \ingroup ToolManagerEtAl
* \ingroup org_mitk_gui_qt_segmentation_internal
* \warning Implementation of this class is split up into two .cpp files to make things more compact. Check both this file and QmitkSegmentationOrganNamesHandling.cpp
*/
class QmitkSegmentationView : public QmitkAbstractView, public mitk::ILifecycleAwarePart, public mitk::IRenderWindowPartListener
{
  Q_OBJECT

public:

  QmitkSegmentationView();

  ~QmitkSegmentationView() override;

  typedef std::map<mitk::DataNode*, unsigned long> NodeTagMapType;

  void NewNodeObjectsGenerated(mitk::ToolManager::DataVectorType*);

  void Activated() override;
  void Deactivated() override;
  void Visible() override;
  void Hidden() override;

  ///
  /// Sets the focus to an internal widget.
  ///
  void SetFocus() override;

  void RenderWindowPartActivated(mitk::IRenderWindowPart* renderWindowPart) override;

  void RenderWindowPartDeactivated(mitk::IRenderWindowPart* renderWindowPart) override;

  // BlueBerry's notification about preference changes (e.g. from a dialog)
  void OnPreferencesChanged(const berry::IBerryPreferences* prefs) override;

  // observer to mitk::RenderingManager's RenderingManagerViewsInitializedEvent event
  void RenderingManagerReinitialized();

  static const std::string VIEW_ID;

  protected slots:

  void OnPatientComboBoxSelectionChanged(QList<mitk::DataNode::Pointer> nodes);
  void OnSegmentationComboBoxSelectionChanged(QList<mitk::DataNode::Pointer> nodes);

  // reaction to the button "New segmentation"
  void CreateNewSegmentation();

  void OnManualTool2DSelected(int id);

  void OnVisiblePropertyChanged();

  void OnBinaryPropertyChanged();

  void OnShowMarkerNodes(bool);

  void OnTabWidgetChanged(int);

protected:

  // a type for handling lists of DataNodes
  typedef std::vector<mitk::DataNode*> NodeList;

  // GUI setup
  void CreateQtPartControl(QWidget* parent) override;

  // initially set the tool manager selection from the combo boxes
  void InitToolManagerSelection(mitk::DataNode* referenceData, mitk::DataNode* workingData);

  // propagate BlueBerry selection to ToolManager for manual segmentation
  void SetToolManagerSelection(mitk::DataNode* referenceData, mitk::DataNode* workingData);

  // make sure all images/segmentations look as selected by the users in this view's preferences
  void ForceDisplayPreferencesUponAllImages();

  // decorates a DataNode according to the user preference settings
  void ApplyDisplayOptions(mitk::DataNode* node);

  void ResetMouseCursor();

  void SetMouseCursor(const us::ModuleResource&, int hotspotX, int hotspotY);

  void SetToolSelectionBoxesEnabled(bool);

  // If a contourmarker is selected, the plane in the related widget will be reoriented according to the marker`s geometry
  void OnContourMarkerSelected(const mitk::DataNode* node);

  void NodeRemoved(const mitk::DataNode* node) override;

  void NodeAdded(const mitk::DataNode *node) override;

  static bool CheckForSameGeometry(const mitk::DataNode*, const mitk::DataNode*);

  void UpdateWarningLabel(QString text/*, bool overwriteExistingText = true*/);

  // the Qt parent of our GUI (NOT of this object)
  QWidget* m_Parent;

  // our GUI
  Ui::QmitkSegmentationControls* m_Controls;

  mitk::IRenderWindowPart* m_RenderWindowPart;

  unsigned long m_VisibilityChangedObserverTag;

  bool m_MouseCursorSet;

  bool m_DataSelectionChanged;

  NodeTagMapType  m_WorkingDataObserverTags;

  NodeTagMapType  m_BinaryPropertyObserverTags;

  unsigned int m_RenderingManagerObserverTag;

  mitk::NodePredicateNot::Pointer m_IsNotAHelperObject;
  mitk::NodePredicateAnd::Pointer m_IsOfTypeImagePredicate;
  mitk::NodePredicateOr::Pointer m_IsASegmentationImagePredicate;
  mitk::NodePredicateAnd::Pointer m_IsAPatientImagePredicate;
};

#endif // QMITKSEGMENTATIONVIEW_H
