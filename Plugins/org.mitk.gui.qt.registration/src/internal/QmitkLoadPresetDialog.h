/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2008-02-13 14:07:17 +0100 (Mi, 13 Feb 2008) $
Version:   $Revision: 13599 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

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

