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

#ifndef InteractionTests_h
#define InteractionTests_h

#include <berryISelectionListener.h>

#include <QmitkFunctionality.h>
#include <mitkTestInteractor.h>

#include "ui_InteractionTestsControls.h"

/*!
 \brief InteractionTests

 \warning  This class is not yet documented. Use "git blame" and ask the author to provide basic documentation.

 \sa QmitkFunctionality
 \ingroup ${plugin_target}_internal
 */
class InteractionTests: public QmitkFunctionality
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
Q_OBJECT

public:

  static const std::string VIEW_ID;

  virtual void CreateQtPartControl(QWidget *parent);

protected slots:

  /// \brief Called when the user clicks the GUI button
  void DrawLines();
  void ToggleConfig();
  void GrabInput();

protected:

  virtual void SetFocus();

  /// \brief called by QmitkFunctionality when DataManager's selection has changed
  virtual void OnSelectionChanged(berry::IWorkbenchPart::Pointer source, const QList<mitk::DataNode::Pointer>& nodes);

  Ui::InteractionTestsControls m_Controls;
private:
  mitk::DataNode::Pointer m_CurrentDataNode;
  mitk::TestInteractor::Pointer m_CurrentInteractor;
  mitk::DataNode::Pointer m_CurrentImage;
  bool m_ConfigChoice;
};

#endif // InteractionTests_h
