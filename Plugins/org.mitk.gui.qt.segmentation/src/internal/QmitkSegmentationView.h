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

#ifndef QmitkSegmentationView_h
#define QmitkSegmentationView_h

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

  virtual ~QmitkSegmentationView();

  typedef std::map<mitk::DataNode*, unsigned long> NodeTagMapType;

  /*!
  \brief Invoked when the DataManager selection changed
  */
  virtual void OnSelectionChanged(mitk::DataNode* node);
  virtual void OnSelectionChanged(berry::IWorkbenchPart::Pointer part, const QList<mitk::DataNode::Pointer>& nodes) override;

  // reaction to new segmentations being created by segmentation tools
  void NewNodesGenerated();
  void NewNodeObjectsGenerated(mitk::ToolManager::DataVectorType*);

  virtual void Activated() override;
  virtual void Deactivated() override;
  bool IsActivated() const;
  virtual void Visible() override;
  virtual void Hidden() override;

  ///
  /// Sets the focus to an internal widget.
  ///
  virtual void SetFocus() override;

  virtual void RenderWindowPartActivated(mitk::IRenderWindowPart* renderWindowPart) override;

  virtual void RenderWindowPartDeactivated(mitk::IRenderWindowPart* renderWindowPart) override;

  // BlueBerry's notification about preference changes (e.g. from a dialog)
  virtual void OnPreferencesChanged(const berry::IBerryPreferences* prefs) override;

  // observer to mitk::RenderingManager's RenderingManagerViewsInitializedEvent event
  void RenderingManagerReinitialized();

  // observer to mitk::SliceController's SliceRotation event
  void SliceRotation(const itk::EventObject&);

  static const std::string VIEW_ID;

  protected slots:

    void OnPatientComboBoxSelectionChanged(const mitk::DataNode* node);
    void OnSegmentationComboBoxSelectionChanged(const mitk::DataNode* node);

    // reaction to the button "New segmentation"
    void CreateNewSegmentation();

    void OnManualTool2DSelected(int id);

    void OnWorkingNodeVisibilityChanged();

    // called if a node's binary property has changed
    void OnBinaryPropertyChanged();

    void OnShowMarkerNodes(bool);

    void OnTabWidgetChanged(int);

protected:

  // a type for handling lists of DataNodes
  typedef std::vector<mitk::DataNode*> NodeList;

  // actively query the current selection of data manager
  //void PullCurrentDataManagerSelection();

  // reactions to selection events from data manager (and potential other senders)
  //void BlueBerrySelectionChanged(berry::IWorkbenchPart::Pointer sourcepart, berry::ISelection::ConstPointer selection);
  mitk::DataNode::Pointer FindFirstRegularImage( std::vector<mitk::DataNode*> nodes );
  mitk::DataNode::Pointer FindFirstSegmentation( std::vector<mitk::DataNode*> nodes );

  // propagate BlueBerry selection to ToolManager for manual segmentation
  void SetToolManagerSelection(const mitk::DataNode* referenceData, const mitk::DataNode* workingData);

  // checks if given render window aligns with the slices of given image
  bool IsRenderWindowAligned(QmitkRenderWindow* renderWindow, mitk::Image* image);

  // make sure all images/segmentations look as selected by the users in this view's preferences
  void ForceDisplayPreferencesUponAllImages();

  // decorates a DataNode according to the user preference settings
  void ApplyDisplayOptions(mitk::DataNode* node);

  // GUI setup
  void CreateQtPartControl(QWidget* parent) override;

  void ResetMouseCursor( );

  void SetMouseCursor(const us::ModuleResource&, int hotspotX, int hotspotY );

  void SetToolSelectionBoxesEnabled(bool);

  bool m_MouseCursorSet;

  // If a contourmarker is selected, the plane in the related widget will be reoriented according to the marker`s geometry
  void OnContourMarkerSelected (const mitk::DataNode* node);

  void NodeRemoved(const mitk::DataNode* node) override;

  void NodeAdded(const mitk::DataNode *node) override;

  bool CheckForSameGeometry(const mitk::DataNode*, const mitk::DataNode*) const;

  void UpdateWarningLabel(QString text/*, bool overwriteExistingText = true*/);

  // the Qt parent of our GUI (NOT of this object)
  QWidget* m_Parent;

  // our GUI
  Ui::QmitkSegmentationControls * m_Controls;

  unsigned long m_VisibilityChangedObserverTag;

  bool m_DataSelectionChanged;

  NodeTagMapType  m_WorkingDataObserverTags;

  NodeTagMapType  m_BinaryPropertyObserverTags;

  unsigned int m_RenderingManagerObserverTag;

  bool m_AutoSelectionEnabled;

  mitk::NodePredicateOr::Pointer m_IsOfTypeImagePredicate;
  mitk::NodePredicateProperty::Pointer m_IsBinaryPredicate;
  mitk::NodePredicateNot::Pointer m_IsNotBinaryPredicate;
  mitk::NodePredicateAnd::Pointer m_IsNotABinaryImagePredicate;
  mitk::NodePredicateAnd::Pointer m_IsABinaryImagePredicate;

  mitk::NodePredicateOr::Pointer m_IsASegmentationImagePredicate;
  mitk::NodePredicateAnd::Pointer m_IsAPatientImagePredicate;

  bool m_Activated;

};

#endif /*QMITKsegmentationVIEW_H_*/
