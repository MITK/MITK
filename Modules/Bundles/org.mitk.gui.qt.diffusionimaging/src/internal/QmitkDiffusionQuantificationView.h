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

#ifndef _QMITKDIFFUSIONQUANTIFICATIONVIEW_H_INCLUDED
#define _QMITKDIFFUSIONQUANTIFICATIONVIEW_H_INCLUDED

#include <QmitkFunctionality.h>

#include <string>

#include "berryISelectionListener.h"
#include "berryIStructuredSelection.h"

#include "ui_QmitkDiffusionQuantificationViewControls.h"



/*!
 * \ingroup org_mitk_gui_qt_diffusionquantification_internal
 *
 * \brief QmitkDiffusionQuantificationView
 *
 * Document your class here.
 *
 * \sa QmitkFunctionality
 */
class QmitkDiffusionQuantificationView : public QObject, public QmitkFunctionality
{

  friend struct DqSelListener;

  // this is needed for all Qt objects that should have a MOC object (everything that derives from QObject)
  Q_OBJECT

  public:

  static const std::string VIEW_ID;

  QmitkDiffusionQuantificationView();
  virtual ~QmitkDiffusionQuantificationView();

  virtual void CreateQtPartControl(QWidget *parent);

  /// \brief Creation of the connections of main and control widget
  virtual void CreateConnections();

  /// \brief Called when the functionality is activated
  virtual void Activated();

  virtual void Deactivated();

  virtual void StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget);
  virtual void StdMultiWidgetNotAvailable();

protected slots:

  void GFACheckboxClicked();
  
  void GFA();
  void Curvature();
  void FA();
  void RA();

  void QBIQuantify(int method);
  void QBIQuantification(mitk::DataStorage::SetOfObjects::Pointer inImages, 
    int method) ;

  void TensorQuantify(int method);
  void TensorQuantification(mitk::DataStorage::SetOfObjects::Pointer inImages, 
    int method) ;

protected:

  Ui::QmitkDiffusionQuantificationViewControls* m_Controls;

  QmitkStdMultiWidget* m_MultiWidget;

  berry::ISelectionListener::Pointer m_SelListener;
  berry::IStructuredSelection::ConstPointer m_CurrentSelection;

  static const float m_ScaleDAIValues;
};




#endif // _QMITKDIFFUSIONQUANTIFICATIONVIEW_H_INCLUDED

