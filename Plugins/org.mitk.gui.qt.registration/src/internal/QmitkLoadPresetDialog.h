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

#ifndef QmitkLoadPresetDialog_h_Included
#define QmitkLoadPresetDialog_h_Included

#include <qdialog.h>
#include <qlistwidget.h>
#include <list>

class QLabel;
class QLineEdit;
class QListWidget;
class QPushButton;

#include <mitkCommon.h>

/**
  \brief Dialog for QmitkRigidRegistration.

  \ingroup RigidRegistration

  This dialog is used to ask a user about a preset with transform, metric, optimizer and interpolator parameters to load.
*/
class QmitkLoadPresetDialog : public QDialog
{
  Q_OBJECT

  public:

    QmitkLoadPresetDialog(QWidget* parent, Qt::WindowFlags f, const char* name, std::list<std::string> presets);
    virtual ~QmitkLoadPresetDialog();

    std::string GetPresetName();

  signals:

  public slots:

  protected slots:

    void onPresetImmediatelySelected(QListWidgetItem * item);

  protected:

    QLabel*  lblPrompt;
    QListWidget*  lstPresets;

    QPushButton* btnOk;
};

#endif

