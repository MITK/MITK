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

#ifndef QmitkNewPerspectiveDialog_h_Included
#define QmitkNewPerspectiveDialog_h_Included

#include "mitkColorProperty.h"
#include <MitkSegmentationUIExports.h>

#include <qdialog.h>

#include <QCompleter>

class QLabel;
class QLineEdit;
class Q3ListBox;
class QPushButton;

#include <mitkCommon.h>

/**
  \brief Dialog for QmitkInteractiveSegmentation.

  \ingroup ToolManagerEtAl
  \ingroup Widgets

  This dialog is used to ask a user about the type of a newly created segmentation and a name for it.

  \warning Will not create a new organ type in the OrganTypeProperty. Client has to do that.

  Last contribution by $Author: maleike $.
*/
class QmitkNewPerspectiveDialog : public QDialog
{
    Q_OBJECT

public:

    QmitkNewPerspectiveDialog(QWidget* parent = 0);
    virtual ~QmitkNewPerspectiveDialog();

    const QString GetPerspectiveName();
    void SetPerspectiveName(QString name);

signals:

protected slots:

    void OnAcceptClicked();
    void OnPerspectiveNameChanged(const QString&);

protected:

    QLineEdit*      m_PerspectiveNameLineEdit;
    QPushButton*    m_AcceptNameButton;
    QString         m_PerspectiveName;
};

#endif
