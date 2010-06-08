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

#ifndef _QMITKTENSORRECONSTRUCTIONVIEW_H_INCLUDED
#define _QMITKTENSORRECONSTRUCTIONVIEW_H_INCLUDED

#include <QmitkFunctionality.h>

#include <string>

#include "ui_QmitkTensorReconstructionViewControls.h"

#include "mitkDiffusionImage.h"

#include <berryIPartListener.h>
#include <berryISelectionListener.h>
#include <berryIStructuredSelection.h>

typedef short DiffusionPixelType;

struct TrSelListener;

/*!
 * \ingroup org_mitk_gui_qt_tensorreconstruction_internal
 *
 * \brief QmitkTensorReconstructionView
 *
 * Document your class here.
 *
 * \sa QmitkFunctionality
 */
class QmitkTensorReconstructionView : public QObject, public QmitkFunctionality
{

  friend struct TrSelListener;

  // this is needed for all Qt objects that should have a MOC object (everything that derives from QObject)
  Q_OBJECT

  public:

  static const std::string VIEW_ID;

  QmitkTensorReconstructionView();
  virtual ~QmitkTensorReconstructionView();

  virtual void CreateQtPartControl(QWidget *parent);

  /// \brief Creation of the connections of main and control widget
  virtual void CreateConnections();

  /// \brief Called when the functionality is activated
  virtual void Activated();

  virtual void Deactivated();

  virtual void StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget);
  virtual void StdMultiWidgetNotAvailable();

  static const int nrconvkernels;

protected slots:

  void TeemCheckboxClicked();
  void Advanced1CheckboxClicked();
  void Advanced2CheckboxClicked();
  void ManualThresholdClicked();
  void MethodChoosen(int method);
  void Reconstruct(int method);
  void TeemReconstruction();
  void ItkReconstruction();
  void ItkTensorReconstruction
    (mitk::DataStorage::SetOfObjects::Pointer inImages);
  void TeemTensorReconstruction
    (mitk::DataStorage::SetOfObjects::Pointer inImages);

protected:

  Ui::QmitkTensorReconstructionViewControls* m_Controls;

  QmitkStdMultiWidget* m_MultiWidget;

  template<int L>
  void TemplatedAnalyticalTensorReconstruction(mitk::DiffusionImage<DiffusionPixelType>* vols, 
    float lambda, std::string nodename, std::vector<mitk::DataNode::Pointer>* nodes, int normalization);

  void SetDefaultNodeProperties(mitk::DataNode::Pointer node, std::string name);

  berry::ISelectionListener::Pointer m_SelListener;
  berry::IStructuredSelection::ConstPointer m_CurrentSelection;

};




#endif // _QMITKTENSORRECONSTRUCTIONVIEW_H_INCLUDED

