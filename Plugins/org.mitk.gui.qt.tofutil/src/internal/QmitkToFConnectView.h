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

#ifndef QmitkToFConnectView_h
#define QmitkToFConnectView_h


#include <ui_QmitkToFConnectViewControls.h>
#include <QmitkAbstractView.h>

#include <QStringList>

#include <ui_QmitkToFUtilViewControls.h>


/*!
  \brief QmitkToFConnectView

  \sa QmitkFunctionality
  \ingroup Functionalities
*/
class QmitkToFConnectView : public QmitkAbstractView
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

  public:

    static const std::string VIEW_ID;

    QmitkToFConnectView();
    ~QmitkToFConnectView();

    void SetFocus();

    virtual void CreateQtPartControl(QWidget *parent);


  protected slots:

    /*!
    \brief Slot called when the "Create Device" button of the ConnectionWidget is pressed
    */
    void OnToFCameraConnected();

  protected:

    Ui::QmitkToFConnectViewControls m_Controls;

  private:

};

#endif // _QmitkToFConnectView_H_INCLUDED
