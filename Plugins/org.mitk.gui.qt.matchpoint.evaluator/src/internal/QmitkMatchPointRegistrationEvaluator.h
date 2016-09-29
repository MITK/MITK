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


#ifndef __Q_MITK_MATCHPOINT_REGISTRATION_EVALUATOR_H
#define __Q_MITK_MATCHPOINT_REGISTRATION_EVALUATOR_H

#include <QmitkAbstractView.h>

#include "ui_QmitkMatchPointRegistrationEvaluator.h"

/*!
\brief QmitkMatchPointRegistrationEvaluator

\warning  This class is not yet documented. Use "git blame" and ask the author to provide basic documentation.

\sa QmitkFunctionality
\ingroup ${plugin_target}_internal
*/
class QmitkMatchPointRegistrationEvaluator : public QmitkAbstractView
{  
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:  

  static const std::string VIEW_ID;

  /**
  * Creates smartpointer typedefs
  */
  berryObjectMacro(QmitkMatchPointRegistrationEvaluator)

    QmitkMatchPointRegistrationEvaluator();

  virtual void CreateQtPartControl(QWidget *parent);

  protected slots:

    /// \brief Called when the user clicks the GUI button

    void OnComboStyleChanged(int);
    void OnBlend50Pushed();
    void OnBlendTargetPushed();
    void OnBlendMovingPushed();
    void OnBlendTogglePushed();
    void OnSlideBlendChanged(int);
    void OnSpinBlendChanged(int);
    void OnSpinCheckerChanged(int);
    void OnWipeStyleChanged();
    void OnContourStyleChanged();

protected:
  /// \brief called by QmitkFunctionality when DataManager's selection has changed
  virtual void OnSelectionChanged( berry::IWorkbenchPart::Pointer source,
    const QList<mitk::DataNode::Pointer>& nodes);

  virtual void SetFocus();

  Ui::MatchPointRegistrationEvaluatorControls m_Controls;

private:
  QWidget *m_Parent;

  void Error(QString msg);

  /** Methods returns a list of all eval nodes in the data manager.*/
  QList<mitk::DataNode::Pointer> GetEvalNodes();

  /**
  * Updates the state of controls regarding to selected eval object.*/
  void ConfigureControls();

  mitk::DataNode::Pointer m_selectedEvalNode;
  mitk::DataStorage::SetOfObjects::ConstPointer m_evalNodes;

  bool m_internalBlendUpdate;
};

#endif // MatchPoint_h

