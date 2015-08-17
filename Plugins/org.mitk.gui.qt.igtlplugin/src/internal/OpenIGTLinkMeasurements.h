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


#ifndef OpenIGTLinkMeasurements_h
#define OpenIGTLinkMeasurements_h

#include <berryISelectionListener.h>

#include <QmitkAbstractView.h>

#include "ui_OpenIGTLinkMeasurementsControls.h"
#include "mitkIGTLMeasurements.h"

/**
  \brief OpenIGTLinkMeasurements

  \warning  This class is not yet documented. Use "git blame" and ask the author to provide basic documentation.

  \sa QmitkAbstractView
  \ingroup ${plugin_target}_internal
*/
class OpenIGTLinkMeasurements : public QmitkAbstractView
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

  public:
    ~OpenIGTLinkMeasurements();

    static const std::string VIEW_ID;

  protected slots:

    void StartStopButtonClicked();
    void ExportButtonClicked();


  protected:
    virtual void SetFocus() override {};

    virtual void CreateQtPartControl(QWidget *parent) override;

    Ui::OpenIGTLinkMeasurementsControls m_Controls;

    mitk::IGTLMeasurements* m_Measurements;
};

#endif // OpenIGTLinkMeasurements_h
