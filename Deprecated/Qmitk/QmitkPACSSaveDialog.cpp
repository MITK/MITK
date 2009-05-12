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

#include "QmitkPACSSaveDialog.h"

#include "mitkColorProperty.h"
#include "mitkCoreObjectFactory.h"
#include "mitkMaterialProperty.h"
#include "mitkPACSPlugin.h"
#include "mitkPACSPluginEvents.h"
#include "mitkProperties.h"
#include "mitkRenderingManager.h"
#include "mitkSurface.h"

#include <qmessagebox.h>
#include <qlayout.h>
#include <qlistview.h>
#include <qtoolbutton.h>
#include <qtooltip.h>
#include <qpushbutton.h>

#include <itkCommand.h>

#include "btnSave.xpm"

std::vector<mitk::DataTreeNode*> QmitkPACSSaveDialog::m_DataTreeNodesToSave;

QmitkPACSSaveDialog::QmitkPACSSaveDialog(QWidget* parent, const char* name)
:QDialog(parent, name),
 m_AllowSeveralObjectsIntoOneSeries(false)
{
  m_Plugin = mitk::PACSPlugin::GetInstance();
  if (!m_Plugin) throw std::logic_error ("There should always be an instance of mitk::PACSPlugin.");

  itk::ReceptorMemberCommand<QmitkPACSSaveDialog>::Pointer command = itk::ReceptorMemberCommand<QmitkPACSSaveDialog>::New();
  command->SetCallbackFunction( this, &QmitkPACSSaveDialog::PluginEventNewStudySelected );
  m_ObserverTag = m_Plugin->AddObserver( mitk::PluginStudySelected(), command );

  QGridLayout* grid = new QGridLayout( this, 2, 2 ); // parent, rows, cols

  m_SeriesList = new QListView( this, "Series list" );
  m_SeriesList->setSelectionMode( QListView::Single );
  m_SeriesList->setAllColumnsShowFocus( true );
  m_SeriesList->setSortColumn(-1);
  m_SeriesList->addColumn("#");
  m_SeriesList->addColumn("Date");
  m_SeriesList->addColumn("Time");
  m_SeriesList->addColumn("Description");
  m_SeriesList->addColumn("Body Part Examined");
  grid->addMultiCellWidget( m_SeriesList, 0, 0, 0, 1);

  QToolButton* btnSaveAsNewSeries = new QToolButton( this );
  btnSaveAsNewSeries->setUsesTextLabel(true);
  btnSaveAsNewSeries->setTextLabel("Save data as NEW series."); // TODO then ask, whether we want one series per data or one for all data
  btnSaveAsNewSeries->setTextPosition( QToolButton::BesideIcon );
  QToolTip::add( btnSaveAsNewSeries, "Save data as NEW series");
  grid->addMultiCellWidget( btnSaveAsNewSeries, 1, 1, 0, 0 );
  btnSaveAsNewSeries->setPixmap( QPixmap(btnSave_xpm) );
  connect( btnSaveAsNewSeries, SIGNAL(clicked()), this, SLOT(SaveAsNewSeries()) );

  QToolButton* btnSaveIntoSelectedSeries = new QToolButton( this );
  btnSaveIntoSelectedSeries->setUsesTextLabel(true);
  btnSaveIntoSelectedSeries->setTextLabel("Save data into selected series."); // TODO then ask, whether we want one series per data or one for all data
  btnSaveIntoSelectedSeries->setTextPosition( QToolButton::BesideIcon );
  QToolTip::add( btnSaveIntoSelectedSeries, "Save data into selected series");
  grid->addMultiCellWidget( btnSaveIntoSelectedSeries, 1, 1, 1, 1 );
  connect( btnSaveIntoSelectedSeries, SIGNAL(clicked()), this, SLOT(SaveIntoSelectedSeries()) );

  m_DataTreeNodesToSave.clear();
}

QmitkPACSSaveDialog::~QmitkPACSSaveDialog()
{
  if( m_Plugin )
  {
    m_Plugin->RemoveObserver( m_ObserverTag );
  }
}
    
void QmitkPACSSaveDialog::exec()
{
  UpdateSeriesLists();
  QDialog::exec();
}

void QmitkPACSSaveDialog::PluginEventNewStudySelected( const itk::EventObject& )
{
  UpdateSeriesLists();
}
  
void QmitkPACSSaveDialog::UpdateSeriesLists()
{
  m_SeriesList->clear();
  m_Series.clear();

  mitk::PACSPlugin::SeriesInformationList seriesList = m_Plugin->GetSeriesInformationList();
  for ( mitk::PACSPlugin::SeriesInformationList::iterator seriesIter = seriesList.begin();
        seriesIter != seriesList.end();
        ++seriesIter )
  {
    m_Series.insert( std::make_pair(
    /*QListViewItem* seriesItem = */ new QListViewItem( m_SeriesList,
                                          QString("%1").arg(seriesIter->SeriesNumber), // integer
                                          seriesIter->SeriesDate.c_str(),
                                          seriesIter->SeriesTime.c_str(),
                                          seriesIter->SeriesDescription.c_str(),
                                          seriesIter->BodyPartExamined.c_str()
                                          ),
                                  *seriesIter ) );
  }
}
    
void QmitkPACSSaveDialog::SetDataTreeNodesToSave( std::vector<mitk::DataTreeNode*> nodes )
{
  m_DataTreeNodesToSave = nodes;
}

void QmitkPACSSaveDialog::SaveAsNewSeries()
{
  std::cout << m_DataTreeNodesToSave.size() << " objects to save" << std::endl;
  mitk::DataStorage::SetOfObjects::Pointer nodeSet = mitk::DataStorage::SetOfObjects::New();

  bool separateSeriesForEachObject = !m_AllowSeveralObjectsIntoOneSeries;
  if (m_AllowSeveralObjectsIntoOneSeries)
  {
    int answer = 0;
    if ( m_DataTreeNodesToSave.size() > 1 )
    {
      // only ask when more than one object is given to save
      answer = QMessageBox::question( NULL, "Data export to PACS", 
                                      QString("Do you want to save each object in a separate series?"),
                                      "Each object separate",
                                      "Same series for all data",
                                      QString::null,
                                      0, 1 );
    }
  
    separateSeriesForEachObject = answer == 0;
  }

  
  QmitkPACSSaveUserDialog questionDialog;
  std::pair< int, std::string > userInput = questionDialog.AskUserForSeriesDescriptionAndNumber();
  int seriesNumber = userInput.first;
  std::string seriesDescription = userInput.second;

  mitk::PACSPlugin::StudyInformation study = m_Plugin->GetStudyInformation();
    
  for ( std::vector<mitk::DataTreeNode*>::iterator iter = m_DataTreeNodesToSave.begin();
        iter != m_DataTreeNodesToSave.end();
        ++iter )
  {
    nodeSet->push_back( *iter );

    if (separateSeriesForEachObject)
    {
      std::cout << "Save node to a new series" << std::endl;
      mitk::DataStorage::SetOfObjects::Pointer singleNodeSet = mitk::DataStorage::SetOfObjects::New();
      singleNodeSet->push_back( *iter );
      m_Plugin->SaveAsNewSeries( singleNodeSet.GetPointer(), study.StudyInstanceUID, seriesNumber++, seriesDescription );
    }
  }
      
  if (!separateSeriesForEachObject)
  {
    std::cout << "Save all nodes into one new series to study " << study.StudyInstanceUID << std::endl;
    m_Plugin->SaveAsNewSeries( nodeSet.GetPointer(), study.StudyInstanceUID, seriesNumber, seriesDescription );
  }

  QDialog::accept();
}

void QmitkPACSSaveDialog::SaveIntoSelectedSeries()
{
  std::string seriesInstanceUID;
  std::string seriesDescription;
  std::map<QListViewItem*, mitk::PACSPlugin::SeriesInformation>::iterator seriesSearch = m_Series.find( m_SeriesList->selectedItem() );
  if ( seriesSearch != m_Series.end() )
  {
    seriesInstanceUID = seriesSearch->second.SeriesInstanceUID;
    seriesDescription = seriesSearch->second.SeriesDescription;
  }

  if ( seriesInstanceUID.empty() )
  {
    QMessageBox::information( NULL, "Data export to PACS", "Please select series before saving to it.",
                              QMessageBox::Ok );
    return;
  }
    
  int answer = QMessageBox::question( NULL, "Data export to PACS", 
                                      QString("Are you sure you want to save data into series\n\n%1").arg(seriesDescription.c_str()),
                                      "Save into named series",
                                      "Cancel, do not save",
                                      QString::null,
                                      0, 1 );
  if ( answer == 0 ) // save into this series
  {
    mitk::DataStorage::SetOfObjects::Pointer nodeSet = mitk::DataStorage::SetOfObjects::New();
    
    for ( std::vector<mitk::DataTreeNode*>::iterator iter = m_DataTreeNodesToSave.begin();
          iter != m_DataTreeNodesToSave.end();
          ++iter )
    {
      nodeSet->push_back( *iter );
    }
    
    std::cout << "Save into selected series " << seriesInstanceUID << std::endl;
    m_Plugin->SaveToSeries( nodeSet.GetPointer(), seriesInstanceUID, true );

    QDialog::accept();
  }
  else // cancel
  {
  }
}


void QmitkPACSSaveDialog::setAllowSeveralObjectsIntoOneSeries(bool on)
{
  m_AllowSeveralObjectsIntoOneSeries = on;
}


/******************************* second tiny helper widget ***********************************/

QmitkPACSSaveUserDialog::QmitkPACSSaveUserDialog(QWidget* parent, const char* name)
:QDialog(parent,name)
{
  QGridLayout* grid = new QGridLayout( this, 2, 2 ); // parent, rows, cols

  lineedit = new QLineEdit( this, "Enter series description here" );
  grid->addWidget( lineedit, 0, 0 );

  spinbox = new QSpinBox( 42, 10000, 1, this );
  grid->addWidget( spinbox, 0, 1 );

  QPushButton* btnOk = new QPushButton("OK", this);
  grid->addWidget( btnOk, 1, 0 );
  connect( btnOk, SIGNAL(clicked()), this, SLOT(accept()) );

  QPushButton* btnCancel = new QPushButton("Cancel", this);
  grid->addWidget( btnCancel, 1, 1 );
  connect( btnCancel, SIGNAL(clicked()), this, SLOT(reject()) );
}

QmitkPACSSaveUserDialog::~QmitkPACSSaveUserDialog()
{
}

std::pair< int, std::string > 
QmitkPACSSaveUserDialog::AskUserForSeriesDescriptionAndNumber()
{
  this->exec();

  std::string description( lineedit->text().ascii() );
  int seriesNumber = spinbox->value();

  return std::make_pair( seriesNumber, description );
  
}
    
