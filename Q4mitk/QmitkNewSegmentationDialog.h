/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2008-09-12 15:46:48 +0200 (Fr, 12 Sep 2008) $
Version:   $Revision: 15236 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#ifndef QmitkNewSegmentationDialog_h_Included
#define QmitkNewSegmentationDialog_h_Included

#include <qdialog.h>

class QLabel;
class QLineEdit;
class Q3ListBox;
class QPushButton;

#include <mitkCommon.h>

/**
  \brief Dialog for QmitkInteractiveSegmentation.

  \ingroup Reliver
  \ingroup Widgets

  This dialog is used to ask a user about the type of a newly created segmentation and a name for it.

  \warning Will not create a new organ type in the OrganTypeProperty. Client has to do that.

  Last contribution by $Author: maleike $.
*/
class QMITK_EXPORT QmitkNewSegmentationDialog : public QDialog
{
  Q_OBJECT

  public:
    
    QmitkNewSegmentationDialog(QWidget* parent = 0);
    virtual ~QmitkNewSegmentationDialog();

    const char* GetSegmentationName();
    const char* GetOrganType();

  signals:

  public slots:

    void setPrompt( const QString& prompt );
    void setSegmentationName( const QString& name );
  
  protected slots:

    void onOrganImmediatelySelected(const QString& organ);
    void onOrganSelected(const QString& organ);
    void onOrganSelected(int index);
    void onNewOrganNameChanged(const QString&);

  protected:

    QLabel*  lblPrompt;
    Q3ListBox*  lstOrgans;
    QLineEdit* edtName;

    QPushButton* btnOk;

    QLineEdit* edtNewOrgan;

    QString selectedOrgan;

    bool newOrganEntry;
};

#endif

