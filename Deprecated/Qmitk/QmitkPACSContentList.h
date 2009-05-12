/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 15236 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#ifndef QmitkPACSContentList_h_Included
#define QmitkPACSContentList_h_Included

#include "mitkCommon.h"
#include "mitkPACSPlugin.h"

#include <qwidget.h>

class QListView;
class QListViewItem;

/**
 * \brief Lists PACS patients, studies series
 *
 * Observes mitk::PACSPlugin and displays its
 * contents in listboxes.
 */
class QMITK_EXPORT QmitkPACSContentList : public QWidget
{
  Q_OBJECT

  public:
    
    QmitkPACSContentList(QWidget* parent = 0, bool mainAppFunctionalityIntegration = false, const char* name = 0);

    virtual ~QmitkPACSContentList();

    std::list< std::string >                         GetSelectedSeriesInstanceUIDs(bool returnAll = false);
    std::list< std::pair<std::string,unsigned int> > GetSelectedSeriesInstanceUIDsAndDocumentInstanceNumbers(bool returnAll = false);

  signals:

  public slots:

    /**
     * \brief Trigger lists update
     *
     * Should not be neccessary, widget should listen to all neccessary
     * events thrown by mitk::PACSPlugin.
     */
    void UpdateLists();

  protected slots:

    void UpdatePatientList();
    void UpdateStudiesList();
    void UpdateSeriesList();
    void UpdateDocumentList();
    
    void DumpPACSContent();

  protected:
  
    void PluginEventNewStudySelected(const itk::EventObject&);

    mitk::PACSPlugin* m_Plugin;
  
    unsigned long m_ObserverTag;

    QListView* m_PatientList;
    QListView* m_StudiesList;
    QListView* m_SeriesList;
    QListView* m_DocumentList;
  
    std::map<QListViewItem*, mitk::PACSPlugin::PatientInformation>  m_Patients;
    std::map<QListViewItem*, mitk::PACSPlugin::StudyInformation>    m_Studies;
    std::map<QListViewItem*, mitk::PACSPlugin::SeriesInformation>   m_Series;
    std::map<QListViewItem*, mitk::PACSPlugin::DocumentInformation> m_Documents;
};

#endif

