/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkToFDeviceGeneration_h
#define QmitkToFDeviceGeneration_h


#include <ui_QmitkToFDeviceGenerationControls.h>
#include <QmitkAbstractView.h>

#include <QStringList>

#include <ui_QmitkToFUtilViewControls.h>


/*!
  \brief QmitkToFDeviceGeneration
*/
class QmitkToFDeviceGeneration : public QmitkAbstractView
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

  public:

    static const std::string VIEW_ID;

    QmitkToFDeviceGeneration();
    ~QmitkToFDeviceGeneration() override;

    void SetFocus() override;

    void CreateQtPartControl(QWidget *parent) override;


  protected slots:

    /*!
    \brief Slot called when the "Create Device" button of the ConnectionWidget is pressed
    */
    void OnToFCameraConnected();

  protected:

    Ui::QmitkToFDeviceGenerationControls m_Controls;

  private:

};

#endif
