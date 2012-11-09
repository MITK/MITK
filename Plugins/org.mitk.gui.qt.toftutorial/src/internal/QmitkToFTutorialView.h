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

#ifndef QmitkToFTutorialView_h
#define QmitkToFTutorialView_h

#include <berryISelectionListener.h>

#include <QmitkFunctionality.h>

#include "ui_QmitkToFTutorialViewControls.h"



/*!
  \brief QmitkToFTutorialView is a tutorial showing the basic implementation techniques of MITK-ToF
  Step 1 shows how to acquire images from a tof camera
  Step 2 shows how to apply a processing filter to generate a surface from a range image

  \sa QmitkFunctionality
  \ingroup Functionalities
*/
class QmitkToFTutorialView : public QmitkFunctionality
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

  public:

    static const std::string VIEW_ID;

    QmitkToFTutorialView();
    virtual ~QmitkToFTutorialView();

    virtual void CreateQtPartControl(QWidget *parent);

    virtual void StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget);
    virtual void StdMultiWidgetNotAvailable();

  protected slots:

    /// \brief Called when the user clicks the Step 1 button
    void OnStep1();
    /// \brief Called when the user clicks the Step 2 button
    void OnStep2();

  protected:

    void RemoveAllNodesFromDataStorage();

    Ui::QmitkToFTutorialViewControls* m_Controls;

    QmitkStdMultiWidget* m_MultiWidget;

};



#endif // _QMITKTOFTUTORIALVIEW_H_INCLUDED

