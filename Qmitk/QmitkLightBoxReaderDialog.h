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
#include "mitkChiliPlugin.h"  //using mitk::Vector3D

class QLabel;
class QListBox;
class QPushButton;

class QmitkLightBoxReaderDialog : public QDialog
{
  Q_OBJECT

  public:

    /** constructor and destructor */
    QmitkLightBoxReaderDialog(QWidget* parent = 0, const char* name = 0);
    virtual ~QmitkLightBoxReaderDialog();

    /** add a spacing to the listbox, this function mark the spacing with the highest count */
    void addSpacings( mitk::Vector3D spacing, int count );

    /** return the selected spacing */
    mitk::Vector3D GetSpacing();

  protected:

    /** the listbox to show the different spacings */
    QListBox*  m_Spacings;
    /** the buttons to quit the dialog */
    QPushButton* m_Ok;
    QPushButton* m_Abort;
    /** we want to select the spacing with the maximum count */
    int m_MaxCount;
    int m_Selection;
    /** all set spacings */
    std::vector< mitk::Vector3D > m_SpacingVector;
};

#endif
