/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 15780 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "QmitkPACSContentList.h"

#include "mitkPACSPlugin.h"
#include "mitkPACSPluginEvents.h"

#include <qlabel.h>
#include <qlayout.h>
#include <qlistview.h>

#include <itkCommand.h>

QmitkPACSContentList::QmitkPACSContentList(QWidget* parent, bool mainAppFunctionalityIntegration, const char* name)
:QWidget(parent, name)
{
  m_Plugin = mitk::PACSPlugin::GetInstance();
  if (!m_Plugin) throw std::logic_error ("There should always be an instance of mitk::PACSPlugin.");

  itk::ReceptorMemberCommand<QmitkPACSContentList>::Pointer command = itk::ReceptorMemberCommand<QmitkPACSContentList>::New();
  command->SetCallbackFunction( this, &QmitkPACSContentList::PluginEventNewStudySelected );
  m_ObserverTag = m_Plugin->AddObserver( mitk::PluginStudySelected(), command );
  
  QGridLayout* grid;
  if (mainAppFunctionalityIntegration)
    grid = new QGridLayout( this, 4, 1 ); // parent, rows, cols
  else
    grid = new QGridLayout( this, 3, 2 );

  //new QLabel( "Patients", this);
  m_PatientList = new QListView( this, "Patient list" );
  m_PatientList->setSelectionMode( QListView::Single );
  m_PatientList->setAllColumnsShowFocus( true );
  m_PatientList->setSortColumn(-1);
  m_PatientList->addColumn("ID");
  m_PatientList->addColumn("Name");
  m_PatientList->addColumn("Birth Date");
  m_PatientList->addColumn("Sex");
  m_PatientList->addColumn("Comment");
  if (mainAppFunctionalityIntegration)
    grid->addMultiCellWidget( m_PatientList, 0, 0, 0, 0); // widget, fromRow, toRow, fromCol, toCol
  else
    grid->addMultiCellWidget( m_PatientList, 0, 0, 0, 0);
  
  //new QLabel( "Studies", this);
  m_StudiesList = new QListView( this, "Studies list" );
  m_StudiesList->setSelectionMode( QListView::Single );
  m_StudiesList->setAllColumnsShowFocus( true );
  m_StudiesList->setSortColumn(-1);
  m_StudiesList->addColumn("Date");
  m_StudiesList->addColumn("Time");
  m_StudiesList->addColumn("Description");
  m_StudiesList->addColumn("Mod");
  m_StudiesList->addColumn("Ref. Physician");
  if (mainAppFunctionalityIntegration)
    grid->addMultiCellWidget( m_StudiesList, 1, 1, 0, 0); // widget, fromRow, toRow, fromCol, toCol
  else
    grid->addMultiCellWidget( m_StudiesList, 0, 0, 1, 1);

  //new QLabel( "Series", this);
  m_SeriesList = new QListView( this, "Series list" );
  m_SeriesList->setSelectionMode( QListView::Extended );
  m_SeriesList->setAllColumnsShowFocus( true );
  m_SeriesList->setSortColumn(-1);
  m_SeriesList->addColumn("#");
  m_SeriesList->addColumn("Date");
  m_SeriesList->addColumn("Time");
  m_SeriesList->addColumn("Description");
  m_SeriesList->addColumn("Body Part Examined");
  if (mainAppFunctionalityIntegration)
    grid->addMultiCellWidget( m_SeriesList, 2, 2, 0, 0); // widget, fromRow, toRow, fromCol, toCol
  else
    grid->addMultiCellWidget( m_SeriesList, 1, 1, 0, 1);

  //new QLabel( "Documents", this);
  m_DocumentList = new QListView( this, "Documents list" );
  m_DocumentList->setSelectionMode( QListView::Extended );
  m_DocumentList->setAllColumnsShowFocus( true );
  m_DocumentList->setSortColumn(-1);
  m_DocumentList->addColumn("#");
  m_DocumentList->addColumn("Date");
  m_DocumentList->addColumn("Title");
  m_DocumentList->addColumn("MIME-Type");
  if (mainAppFunctionalityIntegration)
    grid->addMultiCellWidget( m_DocumentList, 3, 3, 0, 0); // widget, fromRow, toRow, fromCol, toCol
  else
    grid->addMultiCellWidget( m_DocumentList, 2, 2, 0, 1);

  connect( m_PatientList, SIGNAL(selectionChanged()), this, SLOT(UpdateStudiesList()) );
  connect( m_StudiesList, SIGNAL(selectionChanged()), this, SLOT(UpdateSeriesList()) );
  connect( m_SeriesList,  SIGNAL(selectionChanged()), this, SLOT(UpdateDocumentList()) );
}

QmitkPACSContentList::~QmitkPACSContentList()
{
  /* functionality and widgets are deleted AFTER plugin, so do not access plugin anymore
  if( m_Plugin )
  {
    m_Plugin->RemoveObserver( m_ObserverTag );
  }
  */
}

void QmitkPACSContentList::UpdateLists()
{
  DumpPACSContent(); // perhaps useful for debugging

  UpdatePatientList();
  UpdateStudiesList();
  UpdateSeriesList();
  UpdateDocumentList();
}

void QmitkPACSContentList::UpdatePatientList()
{
  m_PatientList->clear();
  m_Patients.clear();

  mitk::PACSPlugin::PatientInformationList patientList = m_Plugin->GetPatientInformationList();
  for ( mitk::PACSPlugin::PatientInformationList::iterator patientIter = patientList.begin();
        patientIter != patientList.end();
        ++patientIter )
  {
    m_Patients.insert( std::make_pair(
    /*QListViewItem* patientItem = */ new QListViewItem( m_PatientList,
                                          patientIter->PatientID,
                                          patientIter->PatientsName,
                                          patientIter->PatientsBirthDate,
                                          patientIter->PatientsSex,
                                          patientIter->PatientComments
                                          ),
                                  *patientIter ) );
  }
}

void QmitkPACSContentList::UpdateStudiesList()
{
  m_StudiesList->clear();
  m_Studies.clear();

  mitk::PACSPlugin::StudyInformationList studyList = m_Plugin->GetStudyInformationList( m_Patients[ m_PatientList->selectedItem() ] );
  for ( mitk::PACSPlugin::StudyInformationList::iterator studyIter = studyList.begin();
        studyIter != studyList.end();
        ++studyIter )
  {
    m_Studies.insert( std::make_pair(
    /*QListViewItem* studyItem = */ new QListViewItem( m_StudiesList,
                                          studyIter->StudyDate,
                                          studyIter->StudyTime,
                                          studyIter->StudyDescription,
                                          studyIter->ModalitiesInStudy,
                                          studyIter->ReferringPhysician
                                          ),
                                  *studyIter ) );
  }
}

void QmitkPACSContentList::UpdateSeriesList()
{
  m_SeriesList->clear();
  m_Series.clear();

  std::string studyInstanceUID;
  std::map<QListViewItem*, mitk::PACSPlugin::StudyInformation>::iterator studySearch = m_Studies.find( m_StudiesList->selectedItem() );
  if ( studySearch != m_Studies.end() )
  {
    studyInstanceUID = studySearch->second.StudyInstanceUID;
  }
  
  mitk::PACSPlugin::SeriesInformationList seriesList = m_Plugin->GetSeriesInformationList( studyInstanceUID );
  for ( mitk::PACSPlugin::SeriesInformationList::iterator seriesIter = seriesList.begin();
        seriesIter != seriesList.end();
        ++seriesIter )
  {
    m_Series.insert( std::make_pair(
    /*QListViewItem* seriesItem = */ new QListViewItem( m_SeriesList,
                                          QString("%1").arg(seriesIter->SeriesNumber), // integer
                                          seriesIter->SeriesDate,
                                          seriesIter->SeriesTime,
                                          seriesIter->SeriesDescription,
                                          seriesIter->BodyPartExamined
                                          ),
                                  *seriesIter ) );
  }
}

void QmitkPACSContentList::UpdateDocumentList()
{
  m_DocumentList->clear();
  m_Documents.clear();

  QListViewItemIterator listIter( m_SeriesList );
  while ( listIter.current() )
  {
    if ( listIter.current()->isSelected() )
    {
      std::map<QListViewItem*, mitk::PACSPlugin::SeriesInformation>::iterator seriesSearch = m_Series.find( listIter.current() );
      if ( seriesSearch != m_Series.end() )
      {
        std::string seriesInstanceUID = seriesSearch->second.SeriesInstanceUID;
        mitk::PACSPlugin::DocumentInformationList documentList = m_Plugin->GetDocumentInformationList( seriesInstanceUID );
        for ( mitk::PACSPlugin::DocumentInformationList::iterator documentIter = documentList.begin();
              documentIter != documentList.end();
              ++documentIter )
        {
          m_Documents.insert( std::make_pair(
          /*QListViewItem* documentItem = */ new QListViewItem( m_DocumentList,
                                                  QString("%1").arg(documentIter->InstanceNumber), // unsigned integer
                                                  documentIter->ContentDate,
                                                  documentIter->DocumentTitle,
                                                  documentIter->MimeType
                                                  ),
                                      *documentIter ) );
        }
      }
    }
    ++listIter;
  }
}

void QmitkPACSContentList::PluginEventNewStudySelected( const itk::EventObject& )
{
  this->UpdateLists();
}

void QmitkPACSContentList::DumpPACSContent()
{
  std::stringstream s;

  mitk::PACSPlugin::PatientInformationList patientList = m_Plugin->GetPatientInformationList();
  for ( mitk::PACSPlugin::PatientInformationList::iterator patientIter = patientList.begin();
        patientIter != patientList.end();
        ++patientIter )
  {
    s << "===== Patient: " << patientIter->PatientID << std::endl;
    s << "PatientsName:      " << patientIter->PatientsName << std::endl;
    s << "PatientsBirthDate: " << patientIter->PatientsBirthDate << std::endl;
    s << "PatientsBirthTime: " << patientIter->PatientsBirthTime << std::endl;
    s << "PatientsSex:       " << patientIter->PatientsSex << std::endl;
    s << "PatientComments:   " << patientIter->PatientComments << std::endl;

  
    mitk::PACSPlugin::StudyInformationList studyList = m_Plugin->GetStudyInformationList( *patientIter );
    for ( mitk::PACSPlugin::StudyInformationList::iterator studyIter = studyList.begin();
          studyIter != studyList.end();
          ++studyIter )
    {
      s << "  ===== Study: " << studyIter->StudyInstanceUID << std::endl;
      s << "  StudyID:             " << studyIter->StudyID << std::endl;
      s << "  StudyDate:           " << studyIter->StudyDate << std::endl;
      s << "  StudyTime:           " << studyIter->StudyTime << std::endl;
      s << "  AccessionNumber:     " << studyIter->AccessionNumber << std::endl;
      s << "  ModalitiesInStudy:   " << studyIter->ModalitiesInStudy << std::endl;
      s << "  ReferringPhysician:  " << studyIter->ReferringPhysician << std::endl;
      s << "  StudyDescription:    " << studyIter->StudyDescription << std::endl;

      mitk::PACSPlugin::SeriesInformationList seriesList = m_Plugin->GetSeriesInformationList( studyIter->StudyInstanceUID );
      for ( mitk::PACSPlugin::SeriesInformationList::iterator seriesIter = seriesList.begin();
            seriesIter != seriesList.end();
            ++seriesIter )
      {
        s << "    ===== Series: " << seriesIter->SeriesInstanceUID << std::endl;
        s << "    SeriesNumber:      " << seriesIter->SeriesNumber << std::endl;
        s << "    SeriesDate:        " << seriesIter->SeriesDate << std::endl;
        s << "    SeriesTime:        " << seriesIter->SeriesTime << std::endl;
        s << "    SeriesDescription: " << seriesIter->SeriesDescription << std::endl;
        s << "    BodyPartExamined:  " << seriesIter->BodyPartExamined << std::endl;

        mitk::PACSPlugin::DocumentInformationList docList = m_Plugin->GetDocumentInformationList( seriesIter->SeriesInstanceUID );
        for ( mitk::PACSPlugin::DocumentInformationList::iterator docIter = docList.begin();
              docIter != docList.end();
              ++docIter )
        {
          s << "      ===== Document: " << docIter->InstanceNumber << std::endl;
          s << "      MimeType:      " << docIter->MimeType << std::endl;
          s << "      ContentDate:   " << docIter->ContentDate << std::endl;
          s << "      DocumentTitle: " << docIter->DocumentTitle << std::endl;
        }
        
      } // series
    } // study
  } // patient

  std::cout << "------------- PACS content info ------------- " << std::endl;
  std::cout << s.str();
}
    
std::list< std::string > QmitkPACSContentList::GetSelectedSeriesInstanceUIDs(bool returnAll)
{
  std::list< std::string > returnValue;

  QListViewItemIterator listIter( m_SeriesList );
  while ( listIter.current() )
  {
    if ( listIter.current()->isSelected() || returnAll )
    {
      std::map<QListViewItem*, mitk::PACSPlugin::SeriesInformation>::iterator seriesSearch = m_Series.find( listIter.current() );
      if ( seriesSearch != m_Series.end() )
      {
        std::string seriesInstanceUID = seriesSearch->second.SeriesInstanceUID;
        returnValue.push_back( seriesInstanceUID );
      }
    }
    ++listIter;
  }

  return returnValue;
}

std::list< std::pair<std::string,unsigned int> > QmitkPACSContentList::GetSelectedSeriesInstanceUIDsAndDocumentInstanceNumbers(bool returnAll)
{
  std::list< std::pair<std::string,unsigned int> > returnValue;

  QListViewItemIterator listIter( m_DocumentList );
  while ( listIter.current() )
  {
    if ( listIter.current()->isSelected() || returnAll )
    {
      std::map<QListViewItem*, mitk::PACSPlugin::DocumentInformation>::iterator documentSearch = m_Documents.find( listIter.current() );
      if ( documentSearch != m_Documents.end() )
      {
        std::string seriesInstanceUID = documentSearch->second.SeriesInstanceUID;
        unsigned int docInstanceNumber = documentSearch->second.InstanceNumber;
        returnValue.push_back( std::make_pair(seriesInstanceUID, docInstanceNumber) );
      }
    }
    ++listIter;
  }

  return returnValue;
}

