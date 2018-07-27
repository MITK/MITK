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



#include <berryISelectionListener.h>
#include <berryIStructuredSelection.h>

#include <QmitkAbstractView.h>
#include "ui_QmitkDipyReconstructionsViewControls.h"
#include <mitkImage.h>

#include <mitkDiffusionPropertyHelper.h>
#include <QmitkDataStorageComboBoxWithSelectNone.h>

/*!
\brief
*/
class QmitkDipyReconstructionsView : public QmitkAbstractView
{

  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:

  static const std::string VIEW_ID;

  typedef itk::VectorImage< short, 3 >        ItkDwiType;
  typedef mitk::GradientDirectionsProperty    GradProp;

  QmitkDipyReconstructionsView();
  virtual ~QmitkDipyReconstructionsView();

  virtual void CreateQtPartControl(QWidget *parent) override;
  void SetFocus() override;

protected slots:

  void StartFit();
  void UpdateGUI();             ///< update button activity etc. dpending on current datamanager selection


protected:

  /// \brief called by QmitkAbstractView when DataManager's selection has changed
  virtual void OnSelectionChanged(berry::IWorkbenchPart::Pointer part, const QList<mitk::DataNode::Pointer>& nodes) override;

  Ui::QmitkDipyReconstructionsViewControls* m_Controls;

private:

  void UpdateRegistrationStatus();  ///< update textual status display of the Registration process

  // the Qt parent of our GUI (NOT of this object)
  QWidget* m_Parent;

};
