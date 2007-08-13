/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile: mitk.cpp,v $
Language:  C++
Date:      $Date: 2007-07-11 10:05:39 +0200 (Mi, 11 Jul 2007) $
Version:   $Revision: 11237 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef QmitkChiliPluginDifferentStudieDialog_h_Included
#define QmitkChiliPluginDifferentStudieDialog_h_Included

#include <qdialog.h>
#include <mitkChiliPlugin.h>

class QPushButton;
class QGridLayout;
class QButtonGroup;

class QmitkChiliPluginDifferentStudiesDialog : public QDialog
{
  Q_OBJECT

  public:

    /** constructor and destructor */
    QmitkChiliPluginDifferentStudiesDialog( QWidget* parent = 0, const char* name = 0 );
    virtual ~QmitkChiliPluginDifferentStudiesDialog();

    /** this function add a study to the gridlayout
    if the study already shown, the function only add the nodeName to the gridlayout
    nodeName: the name of the mitk::DataTreeNode, so you can see which node came from which series
    */
    void addStudy( mitk::ChiliPlugin::StudyInformation newStudyInput, mitk::ChiliPlugin::PatientInformation newPatientInput, std::string nodeName );

    std::string GetSelectedStudy();

  protected:

    bool m_Done;
    std::vector<std::string> m_StudyOIDs;
    QGridLayout*  m_Series;
    QButtonGroup* m_ButtonGroup;
    QPushButton* m_Ok;
    QPushButton* m_Abort;
};

#endif

