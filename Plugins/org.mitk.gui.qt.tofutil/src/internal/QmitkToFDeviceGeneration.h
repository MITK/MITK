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

#ifndef QmitkToFDeviceGeneration_h
#define QmitkToFDeviceGeneration_h


#include <ui_QmitkToFDeviceGenerationControls.h>
#include <QmitkAbstractView.h>

#include <QStringList>

#include <ui_QmitkToFUtilViewControls.h>


/*!
  \brief QmitkToFDeviceGeneration

  \sa QmitkFunctionality
  \ingroup Functionalities
*/
class QmitkToFDeviceGeneration : public QmitkAbstractView
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

  public:

    static const std::string VIEW_ID;

    QmitkToFDeviceGeneration();
    ~QmitkToFDeviceGeneration();

    void SetFocus();

    virtual void CreateQtPartControl(QWidget *parent);


  protected slots:

    /*!
    \brief Slot called when the "Create Device" button of the ConnectionWidget is pressed
    */
    void OnToFCameraConnected();

  protected:

    Ui::QmitkToFDeviceGenerationControls m_Controls;

  private:

};

#endif // _QmitkToFDeviceGeneration_H_INCLUDED
