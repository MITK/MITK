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
class QmitkControlVisualizationPropertiesView : public QObject, public QmitkFunctionality, public berry::ISizeProvider
{

  friend struct CvpSelListener;

  // this is needed for all Qt objects that should have a MOC object (everything that derives from QObject)
  Q_OBJECT

  public:

  static const std::string VIEW_ID;

  QmitkControlVisualizationPropertiesView();
  virtual ~QmitkControlVisualizationPropertiesView();

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
  void SetEnumProp (mitk::DataStorage::SetOfObjects::Pointer,std::string,mitk::EnumerationProperty::Pointer);

  virtual int GetSizeFlags(bool width);
  virtual int ComputePreferredSize(bool width, int availableParallel, int availablePerpendicular, int preferredResult);

protected slots:

  void DisplayIndexChanged(int);
  void TextIntON();
  void Reinit();

  void VisibleOdfsON();
  void ShowMaxNumberChanged(int);
  void NormalizationDropdownChanged(int);
  void ScalingFactorChanged(double);
  void AdditionalScaling(int);
  void IndexParam1Changed(double);
  void IndexParam2Changed(double);

protected:

  Ui::QmitkControlVisualizationPropertiesViewControls* m_Controls;

  QmitkStdMultiWidget* m_MultiWidget;

  berry::ISelectionListener::Pointer m_SelListener;
  berry::IStructuredSelection::ConstPointer m_CurrentSelection;

  bool m_FoundSingleOdfImage;
  bool m_IsInitialized;

  QIcon* m_IconTexOFF;
  QIcon* m_IconTexON;
  QIcon* m_IconGlyOFF;
  QIcon* m_IconGlyON;
  bool m_TexIsOn;
  bool m_GlyIsOn;

};




#endif // _QMITKControlVisualizationPropertiesView_H_INCLUDED

