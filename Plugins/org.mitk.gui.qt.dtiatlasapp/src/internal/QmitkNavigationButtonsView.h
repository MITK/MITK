/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2009-05-28 17:19:30 +0200 (Do, 28 Mai 2009) $
Version:   $Revision: 17495 $ 
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef _QMITKNavigationButtonsView_H_INCLUDED
#define _QMITKNavigationButtonsView_H_INCLUDED

#include <QmitkFunctionality.h>

#include <string>

#include "berryISelectionListener.h"
#include "berryIStructuredSelection.h"
#include "berryISizeProvider.h"

#include "ui_QmitkNavigationButtonsViewControls.h"

#include "mitkEnumerationProperty.h"

/*!
 * \ingroup org_mitk_gui_qt_diffusionquantification_internal
 *
 * \brief QmitkNavigationButtonsView
 *
 * Document your class here.
 *
 * \sa QmitkFunctionality
 */
class QmitkNavigationButtonsView : public QmitkFunctionality//, public berry::ISizeProvider
{

  friend struct CvpSelListener;

  // this is needed for all Qt objects that should have a MOC object (everything that derives from QObject)
  Q_OBJECT

  public:

  static const std::string VIEW_ID;

  QmitkNavigationButtonsView();
  QmitkNavigationButtonsView(const QmitkNavigationButtonsView& other);
  virtual ~QmitkNavigationButtonsView();

  virtual void CreateQtPartControl(QWidget *parent);

  /// \brief Creation of the connections of main and control widget
  virtual void CreateConnections();

  /// \brief Called when the functionality is activated
  virtual void Activated();

  virtual void Deactivated();

  virtual void StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget);
  virtual void StdMultiWidgetNotAvailable();

  mitk::DataStorage::SetOfObjects::Pointer ActiveSet(std::string);

  void SetBoolProp (mitk::DataStorage::SetOfObjects::Pointer,std::string,bool);
  void SetIntProp  (mitk::DataStorage::SetOfObjects::Pointer,std::string,int);
  void SetFloatProp(mitk::DataStorage::SetOfObjects::Pointer,std::string,float);
  void SetLevelWindowProp(mitk::DataStorage::SetOfObjects::Pointer,std::string,mitk::LevelWindow);
  void SetEnumProp (mitk::DataStorage::SetOfObjects::Pointer,std::string,mitk::EnumerationProperty::Pointer);

  virtual int GetSizeFlags(bool width);
  virtual int ComputePreferredSize(bool width, int availableParallel, int availablePerpendicular, int preferredResult);

protected slots:

  void DisplayIndexChanged(int);
  void TextIntON();
  void Reinit();

  void VisibleOdfsON(int view);
  void VisibleOdfsON_S();
  void VisibleOdfsON_T();
  void VisibleOdfsON_C();

  void ShowMaxNumberChanged();
  void NormalizationDropdownChanged(int);
  void ScalingFactorChanged(double);
  void AdditionalScaling(int);
  void IndexParam1Changed(double);
  void IndexParam2Changed(double);
  void OpacityChanged(double,double);
  void ScalingCheckbox();

  void OnThickSlicesModeSelected( QAction* action );
  void OnTSNumChanged(int num);
  void OnMenuAboutToShow ();

  void BundleRepresentationWire();
  void BundleRepresentationTube();
  void BundleRepresentationColor();
  void PlanarFigureFocus();

  void SetInteractor();

  void PFWidth(int);
  void PFColor();
  void PFColor3D();

  void LineWidthChanged(int);
  void TubeRadiusChanged(int);

  void Heatmap();
  void Welcome();

protected:

  Ui::QmitkNavigationButtonsViewControls* m_Controls;

  QmitkStdMultiWidget* m_MultiWidget;

  berry::ISelectionListener::Pointer m_SelListener;
  berry::IStructuredSelection::ConstPointer m_CurrentSelection;

  bool m_FoundSingleOdfImage;
  bool m_IsInitialized;

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
  mitk::DataNode* m_SelectedNode;
  mitk::DataNode* m_CurrentPickingNode;
};




#endif // _QMITKNavigationButtonsView_H_INCLUDED

