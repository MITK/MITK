/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkToFTutorialView_h
#define QmitkToFTutorialView_h

#include <berryISelectionListener.h>

#include <QmitkAbstractView.h>

#include "ui_QmitkToFTutorialViewControls.h"



/*!
  \brief QmitkToFTutorialView is a tutorial showing the basic implementation techniques of MITK-ToF
  Step 1 shows how to acquire images from a tof camera
  Step 2 shows how to apply a processing filter to generate a surface from a range image
*/
class QmitkToFTutorialView : public QmitkAbstractView
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

  public:

    static const std::string VIEW_ID;

    QmitkToFTutorialView();
    ~QmitkToFTutorialView() override;

    void CreateQtPartControl(QWidget *parent) override;

    ///
    /// Sets the focus to an internal widget.
    ///
    void SetFocus() override;

  protected slots:

    /// \brief Called when the user clicks the Step 1 button
    void OnStep1();
    /// \brief Called when the user clicks the Step 2 button
    void OnStep2();

  protected:

    void RemoveAllNodesFromDataStorage();

    Ui::QmitkToFTutorialViewControls* m_Controls;

};



#endif
