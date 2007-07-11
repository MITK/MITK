/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile: mitk.cpp,v $
Language:  C++
Date:      $Date$
Version:   $Revision$
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#ifndef QmitkLightBoxReaderDialog_h_Included
#define QmitkLightBoxReaderDialog_h_Included

#include <qdialog.h>
#include <vector>
#include "mitkChiliPlugin.h"

class QLabel;
class QListBox;
class QPushButton;

class QmitkLightBoxReaderDialog : public QDialog
{
  Q_OBJECT

  public:

    QmitkLightBoxReaderDialog(QWidget* parent = 0, const char* name = 0);
    virtual ~QmitkLightBoxReaderDialog();

    void addSpacings( mitk::Vector3D spacing, int count );

    mitk::Vector3D GetSpacing();

  protected:

    QListBox*  m_Spacings;

    QPushButton* m_Ok;
    QPushButton* m_Abourt;

    int m_MaxCount;
    int m_Selection;
    std::vector<mitk::Vector3D> m_SpacingVector;

};

#endif

