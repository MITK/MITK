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

#ifndef QmitkMultiLabelSegmentationView_h
#define QmitkMultiLabelSegmentationView_h

#include <berryISizeProvider.h>
#include <berryIBerryPreferences.h>

#include <QmitkAbstractView.h>
#include <mitkIRenderWindowPartListener.h>

#include "ui_QmitkMultiLabelSegmentationControls.h"

class QmitkRenderWindow;

/**
 * \ingroup ToolManagerEtAl
 * \ingroup org_mitk_gui_qt_multilabelsegmentation_internal
 */
class QmitkMultiLabelSegmentationView : public QmitkAbstractView,
  public mitk::IRenderWindowPartListener, public berry::ISizeProvider
{
  Q_OBJECT

public:

  static const std::string VIEW_ID;

  QmitkMultiLabelSegmentationView();
  virtual ~QmitkMultiLabelSegmentationView();

  typedef std::map<mitk::DataNode*, unsigned long> NodeTagMapType;

  // GUI setup
  void CreateQtPartControl(QWidget* parent);

  virtual int GetSizeFlags(bool width);
  virtual int ComputePreferredSize(bool width, int /*availableParallel*/, int /*availablePerpendicular*/, int preferredResult);

protected slots:

  void OnReferenceSelectionChanged(const mitk::DataNode* node);

  void OnSegmentationSelectionChanged(const mitk::DataNode *node);

  void OnGoToLabel(const mitk::Point3D& pos);

  void OnManualTool2DSelected(int id);

  void OnShowMarkerNodes(bool);

  void On2DInterpolationEnabled(bool);

  void OnTabWidgetChanged(int);

  void OnSurfaceStamp();

  void OnMaskStamp();

protected:

  void SetFocus();

  void RenderWindowPartActivated(mitk::IRenderWindowPart *renderWindowPart);
  void RenderWindowPartDeactivated(mitk::IRenderWindowPart *renderWindowPart);

  void ResetMouseCursor();

  void SetMouseCursor(const us::ModuleResource, int hotspotX, int hotspotY );

  /**
  * Reimplemented from QmitkAbstractView
  */
  virtual void NodeAdded(const mitk::DataNode* node);

  /**
  * Reimplemented from QmitkAbstractView
  */
  virtual void NodeRemoved(const mitk::DataNode* node);

  // the Qt parent of our GUI (NOT of this object)
  QWidget* m_Parent;

  // our GUI
  Ui::QmitkMultiLabelSegmentationControls m_Controls;

  mitk::IRenderWindowPart* m_IRenderWindowPart;

  mitk::ToolManager* m_ToolManager;

  mitk::DataNode::Pointer m_ReferenceNode;
  mitk::DataNode::Pointer m_WorkingNode;

  mitk::NodePredicateAnd::Pointer m_ReferencePredicate;
  mitk::NodePredicateAnd::Pointer m_SegmentationPredicate;
  mitk::NodePredicateAnd::Pointer m_SurfacePredicate;
  mitk::NodePredicateAnd::Pointer m_MaskPredicate;

  bool m_MouseCursorSet;
};

#endif // QmitkMultiLabelSegmentationView_h
