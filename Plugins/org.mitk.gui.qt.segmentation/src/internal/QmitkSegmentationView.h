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

#include "QmitkFunctionality.h"

#include <berryIBerryPreferences.h>

#include "ui_QmitkSegmentationControls.h"

class QmitkRenderWindow;
class QCompleter;

/**
 * \ingroup ToolManagerEtAl
 * \ingroup org_mitk_gui_qt_segmentation_internal
 * \warning Implementation of this class is split up into two .cpp files to make things more compact.
 *  Check both this file and QmitkSegmentationOrganNamesHandling.cpp
 */
class QmitkSegmentationView : public QmitkFunctionality
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
  virtual void OnSelectionChanged(std::vector<mitk::DataNode*> nodes);

  // QmitkFunctionality's activate/deactivate
  virtual void Activated();
  virtual void Deactivated();
  virtual void Visible();

  // QmitkFunctionality's changes regarding THE QmitkStdMultiWidget
  virtual void StdMultiWidgetAvailable(QmitkStdMultiWidget& stdMultiWidget);
  virtual void StdMultiWidgetNotAvailable();
  virtual void StdMultiWidgetClosed(QmitkStdMultiWidget& stdMultiWidget);

  // BlueBerry's notification about preference changes (e.g. from a dialog)
  virtual void OnPreferencesChanged(const berry::IBerryPreferences* prefs);

  // observer to mitk::RenderingManager's RenderingManagerViewsInitializedEvent event
  void RenderingManagerReinitialized(const itk::EventObject&);

  // observer to mitk::SliceController's SliceRotation event
  void SliceRotation(const itk::EventObject&);

  static const std::string VIEW_ID;

protected slots:

  void OnReferenceSelectionChanged(const mitk::DataNode* node);

  void OnSegmentationSelectionChanged(const mitk::DataNode* node);

  void OnSurfaceStamp();

  void OnGoToLabel( const mitk::Point3D& pos );

  void OnManualTool2DSelected(int id);

  void OnShowMarkerNodes(bool);

  void OnTabWidgetChanged(int);

protected:

  // a type for handling lists of DataNodes
  typedef std::vector<mitk::DataNode*> NodeList;

  // set available multiwidget
  void SetMultiWidget(QmitkStdMultiWidget* multiWidget);

  // actively query the current selection of data manager
  //void PullCurrentDataManagerSelection();

  // reactions to selection events from data manager (and potential other senders)
  mitk::DataNode::Pointer FindFirstRegularImage( std::vector<mitk::DataNode*> nodes );
  mitk::DataNode::Pointer FindFirstSegmentation( std::vector<mitk::DataNode*> nodes );

  // checks if given render window aligns with the slices of given image
  bool IsRenderWindowAligned(QmitkRenderWindow* renderWindow, mitk::Image* image);

  // make sure all images/segmentations look as selected by the users in this view's preferences
  void ForceDisplayPreferencesUponAllImages();

  // GUI setup
  void CreateQtPartControl(QWidget* parent);

  void ResetMouseCursor( );

  void SetMouseCursor(const mitk::ModuleResource, int hotspotX, int hotspotY );

  bool m_MouseCursorSet;

  // If a contourmarker is selected, the plane in the related widget will be reoriented according to the marker`s geometry
  void OnContourMarkerSelected (const mitk::DataNode* node);

  void NodeRemoved(const mitk::DataNode* node);

  void NodeAdded(const mitk::DataNode *node);

  bool CheckForSameGeometry(const mitk::DataNode*, const mitk::DataNode*) const;

  void UpdateWarningLabel(QString text/*, bool overwriteExistingText = true*/);

  // the Qt parent of our GUI (NOT of this object)
  QWidget* m_Parent;

  // our GUI
  Ui::QmitkSegmentationControls * m_Controls;

  // THE currently existing QmitkStdMultiWidget
  QmitkStdMultiWidget * m_MultiWidget;

  unsigned long m_VisibilityChangedObserverTag;

  bool m_DataSelectionChanged;

  NodeTagMapType  m_WorkingDataObserverTags;

  NodeTagMapType  m_BinaryPropertyObserverTags;

  mitk::NodePredicateAnd::Pointer m_ReferencePredicate;
  mitk::NodePredicateAnd::Pointer m_SegmentationPredicate;
};

#endif /*QMITKsegmentationVIEW_H_*/
