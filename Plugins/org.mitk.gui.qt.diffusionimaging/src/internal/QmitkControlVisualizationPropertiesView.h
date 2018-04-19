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

#ifndef _QMITKControlVisualizationPropertiesView_H_INCLUDED
#define _QMITKControlVisualizationPropertiesView_H_INCLUDED

#include <QmitkAbstractView.h>

#include <string>

#include "berryISelectionListener.h"
#include "berryIStructuredSelection.h"
#include "berryISizeProvider.h"

#include "ui_QmitkControlVisualizationPropertiesViewControls.h"

#include "mitkEnumerationProperty.h"
#include <mitkILifecycleAwarePart.h>
#include <QmitkSliceNavigationListener.h>

/*!
 * \ingroup org_mitk_gui_qt_diffusionquantification_internal
 *
 * \brief QmitkControlVisualizationPropertiesView
 *
 * Document your class here.
 */
class QmitkControlVisualizationPropertiesView : public QmitkAbstractView, public mitk::ILifecycleAwarePart
{

  friend struct CvpSelListener;

  // this is needed for all Qt objects that should have a MOC object (everything that derives from QObject)
  Q_OBJECT

  public:

  static const std::string VIEW_ID;

  QmitkControlVisualizationPropertiesView();
  virtual ~QmitkControlVisualizationPropertiesView();

  virtual void CreateQtPartControl(QWidget *parent) override;

  /// \brief Creation of the connections of main and control widget
  virtual void CreateConnections();


protected slots:

  void VisibleOdfsON_S();
  void VisibleOdfsON_T();
  void VisibleOdfsON_C();
  void ShowMaxNumberChanged();
  void NormalizationDropdownChanged(int);
  void ScalingFactorChanged(double);
  void AdditionalScaling(int);
  void OnThickSlicesModeSelected( QAction* action );
  void OnTSNumChanged(int num);
  void ResetColoring();
  void PlanarFigureFocus();
  void Fiber2DfadingEFX();
  void FiberSlicingThickness2D();
  void FiberSlicingUpdateLabel(int);
  void LineWidthChanged();
  void TubeRadiusChanged();
  void RibbonWidthChanged();
  void SetInteractor();
  void Toggle3DClipping(bool enabled=true);
  void FlipPeaks();
  void Welcome();
  void OnSliceChanged();
  void SetColor();
  void Toggle3DPeaks();

  /// \brief Slot function for switching colourisation mode of glyphs.
  void OnColourisationModeChanged();

protected:

  virtual void SetFocus() override;
  virtual void Activated() override;
  virtual void Deactivated() override;
  virtual void Visible() override;
  virtual void Hidden() override;

  virtual void NodeRemoved(const mitk::DataNode* node) override;

  /// \brief called by QmitkAbstractView when DataManager's selection has changed
  virtual void OnSelectionChanged(berry::IWorkbenchPart::Pointer part, const QList<mitk::DataNode::Pointer>& nodes) override;

  virtual void NodeAdded(const mitk::DataNode *node) override;
  void SetCustomColor(const itk::EventObject& /*e*/);
  bool IsPlaneRotated();

  void SliceRotation(const itk::EventObject&);
  void Set3DClippingPlane(bool disable, mitk::DataNode *node, std::string plane);

  Ui::QmitkControlVisualizationPropertiesViewControls* m_Controls;

  QScopedPointer<berry::ISelectionListener> m_SelListener;
  berry::IStructuredSelection::ConstPointer m_CurrentSelection;

  mitk::DataNode::Pointer m_NodeUsedForOdfVisualization;

  QIcon* m_IconTexOFF;
  QIcon* m_IconTexON;
  QIcon* m_IconGlyOFF_T;
  QIcon* m_IconGlyON_T;
  QIcon* m_IconGlyOFF_C;
  QIcon* m_IconGlyON_C;
  QIcon* m_IconGlyOFF_S;
  QIcon* m_IconGlyON_S;

  bool m_TexIsOn;
  bool m_GlyIsOn_T;
  bool m_GlyIsOn_C;
  bool m_GlyIsOn_S;

  int currentThickSlicesMode;
  QLabel* m_TSLabel;
  QMenu* m_MyMenu;

  // for planarfigure and bundle handling:
  mitk::DataNode::Pointer m_SelectedNode;
  mitk::DataNode* m_CurrentPickingNode;

  unsigned long m_ColorPropertyObserverTag;
  unsigned long m_OpacityPropertyObserverTag;
  mitk::ColorProperty::Pointer m_Color;
  mitk::FloatProperty::Pointer m_Opacity;

  QmitkSliceNavigationListener  m_SliceChangeListener;
};




#endif // _QMITKControlVisualizationPropertiesView_H_INCLUDED

