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

#include <QmitkFunctionality.h>

#include <string>

#include "berryISelectionListener.h"
#include "berryIStructuredSelection.h"
#include "berryISizeProvider.h"

#include "ui_QmitkControlVisualizationPropertiesViewControls.h"

#include "mitkEnumerationProperty.h"

/*!
 * \ingroup org_mitk_gui_qt_diffusionquantification_internal
 *
 * \brief QmitkControlVisualizationPropertiesView
 *
 * Document your class here.
 *
 * \sa QmitkFunctionality
 */
class QmitkControlVisualizationPropertiesView : public QmitkFunctionality//, public berry::ISizeProvider
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

  /// \brief Called when the functionality is activated
  virtual void Activated() override;

  virtual void Deactivated() override;

  virtual void StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget) override;
  virtual void StdMultiWidgetNotAvailable() override;

protected slots:

  void VisibleOdfsON_S();
  void VisibleOdfsON_T();
  void VisibleOdfsON_C();

  void ShowMaxNumberChanged();
  void NormalizationDropdownChanged(int);
  void ScalingFactorChanged(double);
  void AdditionalScaling(int);
  void ScalingCheckbox();

  void OnThickSlicesModeSelected( QAction* action );
  void OnTSNumChanged(int num);

  void BundleRepresentationResetColoring();
  void PlanarFigureFocus();
  void Fiber2DfadingEFX();
  void FiberSlicingThickness2D();
  void FiberSlicingUpdateLabel(int);
  void LineWidthChanged();
  void TubeRadiusChanged();

  void SetInteractor();

  void Welcome();

protected:

  virtual void NodeRemoved(const mitk::DataNode* node) override;

  /// \brief called by QmitkFunctionality when DataManager's selection has changed
  virtual void OnSelectionChanged( std::vector<mitk::DataNode*> nodes ) override;

  virtual void NodeAdded(const mitk::DataNode *node) override;
  void SetFiberBundleCustomColor(const itk::EventObject& /*e*/);
  void SetFiberBundleOpacity(const itk::EventObject& /*e*/);
  bool IsPlaneRotated();

  void SliceRotation(const itk::EventObject&);

  Ui::QmitkControlVisualizationPropertiesViewControls* m_Controls;

  QmitkStdMultiWidget* m_MultiWidget;

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

  unsigned long m_SlicesRotationObserverTag1;
  unsigned long m_SlicesRotationObserverTag2;
  unsigned long m_FiberBundleObserverTag;
  unsigned long m_FiberBundleObserveOpacityTag;
  mitk::ColorProperty::Pointer m_Color;
  mitk::FloatProperty::Pointer m_Opacity;
};




#endif // _QMITKControlVisualizationPropertiesView_H_INCLUDED

