/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: $
Version:   $Revision: $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#include "QmitkChiliPluginSaveDialog.h"
#include "QmitkPluginListViewItem.h"

#include <qlistview.h>
#include <qlineedit.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qmessagebox.h>

#include <sstream>

/** The constructor create the widget. */
QmitkChiliPluginSaveDialog::QmitkChiliPluginSaveDialog( QWidget* parent, const char* name )
:QDialog( parent, name, true )
{
  QDialog::setCaption("Save To Chili");

  QGridLayout* gridLayout = new QGridLayout( this );
  gridLayout->setMargin( 3 );
  gridLayout->setSpacing( 10 );

  gridLayout->addWidget( new QLabel( "SELECT DESTINATION - STUDY", this ), 0, 0 );
  gridLayout->addWidget( new QLabel( "SELECT DESTINATION - SERIES", this ), 0, 1 );
  gridLayout->addWidget( new QLabel( "NODES TO SAVE", this ), 2, 0 );
  gridLayout->addWidget( new QLabel( "SAVE-OPTIONS", this ), 2, 1 );

  //Study-Listview
  m_StudyListView = new QListView( this );
  m_StudyListView->setMargin( 3 );
  m_StudyListView->addColumn( "Patient Name" );
  m_StudyListView->addColumn( "Patient ID" );
  m_StudyListView->addColumn( "Study Description" );
  m_StudyListView->setAllColumnsShowFocus ( true );
  connect( ( QObject* )( m_StudyListView ), SIGNAL( selectionChanged() ), ( QObject* ) this, SLOT( UpdateView() ) );
  gridLayout->addWidget( m_StudyListView, 1, 0 );

  //Series-Listview
  m_SeriesListView = new QListView( this );
  m_SeriesListView->setMargin( 3 );
  m_SeriesListView->addColumn( "Series Number" );
  m_SeriesListView->addColumn( "Series Description" );
  m_SeriesListView->setAllColumnsShowFocus ( true );
  connect( ( QObject* )( m_SeriesListView ), SIGNAL( selectionChanged() ), ( QObject* ) this, SLOT( SetSeries() ) );
  gridLayout->addWidget( m_SeriesListView, 1, 1 );

  //Node
  m_NodeLayout = new QVBoxLayout( 0 );
  m_NodeLayout->setMargin(3);
  gridLayout->addLayout( m_NodeLayout, 3, 0 );

  //Save-Options
  QVBoxLayout* buttonGroupLayout = new QVBoxLayout( 0 );
  buttonGroupLayout->setMargin(3);
  gridLayout->addLayout( buttonGroupLayout, 3, 1 );

  QButtonGroup* buttonGroup = new QButtonGroup();
  //override-radiobutton
  m_Override = new QRadioButton( this, "override" );
  m_Override->setText( "override all data in the selected series" );
  m_Override->setChecked ( true );
  connect( ( QObject* )( m_Override ), SIGNAL( clicked() ), ( QObject* ) this, SLOT( SetNodesByButtonGroup() ) );
  buttonGroup->insert( m_Override );
  buttonGroupLayout->addWidget( m_Override );
  //add-radiobutton
  m_Add = new QRadioButton( this, "add" );
  m_Add->setText( "add all new nodes to the selected series" );
  m_Add->setChecked ( false );
  connect( ( QObject* )( m_Add ), SIGNAL( clicked() ), ( QObject* ) this, SLOT( SetNodesByButtonGroup() ) );
  buttonGroup->insert( m_Add );
  buttonGroupLayout->addWidget( m_Add );
  //newseries-radiobutton
  m_New = new QRadioButton( this, "createNew" );
  m_New->setText( "create a new series for all nodes" );
  m_New->setChecked ( false );
  connect( ( QObject* )( m_New ), SIGNAL( clicked() ), ( QObject* ) this, SLOT( SetNodesByButtonGroup() ) );
  buttonGroup->insert( m_New );
  buttonGroupLayout->addWidget( m_New );
  //input for the seriesnumber and description
  m_SeriesNumber = new QLineEdit( "enter SeriesNumber", this );
  m_SeriesDescription = new QLineEdit( "enter SeriesDescription", this );
  buttonGroupLayout->addWidget( m_SeriesNumber );
  buttonGroupLayout->addWidget( m_SeriesDescription );
  m_SeriesNumber->hide();
  m_SeriesDescription->hide();

  QPushButton* okButton = new QPushButton( tr("OK"), this );
  okButton->setDefault(true);
  okButton->setFocus();
  connect( okButton, SIGNAL( clicked() ), this, SLOT( CheckOutputs() ) );

  QPushButton* abortButton = new QPushButton( tr("Cancel"), this );
  connect( abortButton, SIGNAL( clicked() ), this, SLOT( reject() ) );

  QPushButton* helpButton = new QPushButton( tr("Help"), this );
  connect( helpButton, SIGNAL( clicked() ), this, SLOT( ShowHelp() ) );

  QBoxLayout * horizontalLayout = new QHBoxLayout( 0 );
  horizontalLayout->setSpacing(5);
  horizontalLayout->addStretch();
  horizontalLayout->addWidget( okButton );
  horizontalLayout->addWidget( abortButton );
  horizontalLayout->addWidget( helpButton );

  gridLayout->addLayout( horizontalLayout, 4, 1 );

  m_SeriesInputs.clear();
  m_NodeInputs.clear();
}

/** Destructor */
QmitkChiliPluginSaveDialog::~QmitkChiliPluginSaveDialog()
{
}


/** This function return the user-selection. */
QmitkChiliPluginSaveDialog::ReturnValue QmitkChiliPluginSaveDialog::GetSelection()
{
  ReturnValue newReturnValue;
  //studyOID
  QmitkPluginListViewItem* entry = dynamic_cast<QmitkPluginListViewItem*>( m_StudyListView->selectedItem() );
  if( entry )
  {
    std::string temp = entry->GetChiliOID().ascii();
    newReturnValue.StudyOID = temp.c_str();
  }
  //seriesOID
  entry = dynamic_cast<QmitkPluginListViewItem*>( m_SeriesListView->selectedItem() );
  if( entry )
  {
    std::string temp = entry->GetChiliOID().ascii();
    newReturnValue.SeriesOID = temp.c_str();
  }
  //user selection
  if( m_Override->isChecked() )
    newReturnValue.UserDecision = OverrideAll;
  else
  {
    if( m_Add->isChecked() )
      newReturnValue.UserDecision = AddOnlyNew;
    else
      newReturnValue.UserDecision = CreateNew;
  }
  return newReturnValue;
}

/** If the user want to create a new series, he can enter a SeriesNumber and -Description. This function return them. */
QmitkChiliPluginSaveDialog::NewSeriesInformation QmitkChiliPluginSaveDialog::GetSeriesInformation()
{
  NewSeriesInformation newSeriesInformation;
  std::string seriesDescription = m_SeriesDescription->text().ascii();
  newSeriesInformation.SeriesDescription = seriesDescription.c_str();
  if( m_SeriesNumber->text().ascii() == "" )
    newSeriesInformation.SeriesNumber = 0;
  else
  {
    std::stringstream ssStream;
    ssStream << m_SeriesNumber->text().ascii();
    ssStream >> newSeriesInformation.SeriesNumber;
  }
  return newSeriesInformation;
}

/** This function add a study to the dialog. If you want to add the current selected study as target to save, then use this function. */
void QmitkChiliPluginSaveDialog::AddStudy( std::string studyOID, std::string patientName, std::string patientID, std::string studyDescription )
{
  if( studyOID != "" )
  {
    //search if the study always exist
    QListViewItem* walkThroughStudies = m_StudyListView->firstChild();
    QmitkPluginListViewItem* entry = dynamic_cast<QmitkPluginListViewItem*>( walkThroughStudies );

    while( walkThroughStudies != 0 )
    {
      if( entry )
      {
        std::string savedOID = entry->GetChiliOID().ascii();
        if( savedOID != studyOID )
        {
          walkThroughStudies = walkThroughStudies->itemBelow();
          entry = dynamic_cast<QmitkPluginListViewItem*>( walkThroughStudies );
        }
        else break;
      }
      else break;
    }

    if( walkThroughStudies == 0 )
      new QmitkPluginListViewItem( studyOID.c_str(), "", m_StudyListView, patientName.c_str(), patientID.c_str(), studyDescription.c_str() );
  }
}

/** This function add a study and series to the dialog. If you want to add the current selected study and series as target to save, then use this function. */
void QmitkChiliPluginSaveDialog::AddStudyAndSeries( std::string studyOID, std::string patientName, std::string patientID, std::string studyDescription, std::string seriesOID, int seriesNumber, std::string seriesDescription )
{
  if( studyOID != "" && seriesOID != "" )
  {
    //search if the study and series always exist
    bool entryExist = false;
    for( std::list< SeriesInputs >::iterator iter = m_SeriesInputs.begin(); iter != m_SeriesInputs.end(); iter ++ )
    {
      if( iter->StudyOID == studyOID && iter->SeriesOID == seriesOID )
      {
        entryExist = true;
        break;
      }
    }
    if( !entryExist )
    {
      SeriesInputs newInput;
      newInput.StudyOID = studyOID;
      newInput.SeriesOID = seriesOID;
      if( seriesNumber == -1 )
        newInput.SeriesNumber = "";
      else
      {
        std::ostringstream stringHelper;
        stringHelper << seriesNumber;
        newInput.SeriesNumber = stringHelper.str();
      }
      newInput.SeriesDescription = seriesDescription;
      m_SeriesInputs.push_back( newInput );
    }

    AddStudy( studyOID, patientName, patientID, studyDescription );
  }
}

/** This function add a study, series and node to the dialog. A node which was loaded from chili have a seriesOID as property. With this property you can get the series and study where the node load from. You can set this attributes to the dialog. Then the user can save to this series and study. */
void QmitkChiliPluginSaveDialog::AddStudySeriesAndNode( std::string studyOID, std::string patientName, std::string patientID, std::string studyDescription, std::string seriesOID, int seriesNumber, std::string seriesDescription, mitk::DataTreeNode::Pointer node )
{
  if( studyOID != "" && seriesOID != "" )
  {
    //search if the study and series always exist
    bool entryExist = false;
    for( std::list< SeriesInputs >::iterator iter = m_SeriesInputs.begin(); iter != m_SeriesInputs.end(); iter ++ )
    {
      if( iter->StudyOID == studyOID && iter->SeriesOID == seriesOID )
      {
        entryExist = true;
        break;
      }
    }
    if( !entryExist )
    {
      SeriesInputs newInput;
      newInput.StudyOID = studyOID;
      newInput.SeriesOID = seriesOID;
      if( seriesNumber == -1 )
        newInput.SeriesNumber = "";
      else
      {
        std::ostringstream stringHelper;
        stringHelper << seriesNumber;
        newInput.SeriesNumber = stringHelper.str();
      }
      newInput.SeriesDescription = seriesDescription;
      m_SeriesInputs.push_back( newInput );
    }

    AddStudy( studyOID, patientName, patientID, studyDescription );

    NodeInputs newInput;
    newInput.SeriesOID = seriesOID;
    mitk::BaseProperty::Pointer manufacturerProperty = node->GetProperty( "CHILI: MANUFACTURER" );
    mitk::BaseProperty::Pointer institutionNameProperty = node->GetProperty( "CHILI: INSTITUTION NAME" );
    if( manufacturerProperty && institutionNameProperty && manufacturerProperty->GetValueAsString() == "MITK" && institutionNameProperty->GetValueAsString() == "DKFZ.MBI" )
      newInput.canBeOverride = true;
    else newInput.canBeOverride = false;

    QLabel* newOne;
    mitk::BaseProperty::Pointer descriptionProperty = node->GetProperty( "name" );
    if( descriptionProperty )
      newOne = new QLabel( descriptionProperty->GetValueAsString().c_str(), this );
    else
      newOne = new QLabel( "no Description", this );

    m_NodeLayout->addWidget( newOne );
    newInput.usedLabel = newOne;
    m_NodeInputs.push_back( newInput );
  }
}

/** This function add a node to the dialog. There are nodes which was not loaded from chili. This one have no seriesOID property. But we want to save them too. So we add only the node. If you add only nodes without a series or study, its not possible to save them. Because the user have no study and series to select. Therefore add the current selected study or series! */
void QmitkChiliPluginSaveDialog::AddNode( mitk::DataTreeNode::Pointer node )
{
  NodeInputs newInput;
  newInput.canBeOverride = false;
  newInput.SeriesOID = "";
  QLabel* newOne;
  mitk::BaseProperty::Pointer descriptionProperty = node->GetProperty( "name" );
  if( descriptionProperty )
    newOne = new QLabel( descriptionProperty->GetValueAsString().c_str(), this );
  else
    newOne = new QLabel( "no Description", this );

  m_NodeLayout->addWidget( newOne );
  newInput.usedLabel = newOne;
  m_NodeInputs.push_back( newInput );
}

/** This function refresh the dialog. This slot ensure that a study is selected, show all series to the selected study and call SetSeries(). IMPORTANT: This slot have to call one time bevor exec the dialog.*/
void QmitkChiliPluginSaveDialog::UpdateView()
{
  //ensure that a study is selected
  if( m_StudyListView->selectedItem() == 0 )
    m_StudyListView->setSelected( m_StudyListView->firstChild(), true );

  //get the studyOID from the selected study
  QmitkPluginListViewItem* entry = dynamic_cast<QmitkPluginListViewItem*>( m_StudyListView->selectedItem() );
  m_SeriesListView->clear();

  //search and show all series with the selected studyOID
  for( std::list< SeriesInputs >::iterator iter = m_SeriesInputs.begin(); iter != m_SeriesInputs.end(); iter ++ )
  {
    if( entry )
    {
      std::string savedOID = entry->GetChiliOID().ascii();
      if( savedOID == iter->StudyOID )
      {
        //search if the series always shown
        QListViewItem* walkThroughSeries = m_SeriesListView->firstChild();
        QmitkPluginListViewItem* entry = dynamic_cast<QmitkPluginListViewItem*>( walkThroughSeries );

        while( walkThroughSeries != 0 )
        {
          if( entry )
          {
            std::string secondOID = entry->GetChiliOID().ascii();
            if( secondOID != iter->SeriesOID )
            {
              walkThroughSeries = walkThroughSeries->itemBelow();
              entry = dynamic_cast<QmitkPluginListViewItem*>( walkThroughSeries );
            }
          }
        }

        if( walkThroughSeries == 0 )
          new QmitkPluginListViewItem( iter->SeriesOID.c_str(), "", m_SeriesListView, iter->SeriesNumber.c_str(), iter->SeriesDescription.c_str() );
      }
    }
  }

  SetSeries();
}

/** This slot ensure that a series is selected. If no series exist, the option to create a new series is set to the one and only. Call SetNodesByButtonGroup(). */
void QmitkChiliPluginSaveDialog::SetSeries()
{
  //ensure that a series is selected
  if( m_SeriesListView->selectedItem() == 0 )
    m_SeriesListView->setSelected( m_SeriesListView->firstChild(), true );

  //if no series exist
  if( m_SeriesListView->childCount() == 0 )
  {
    m_Override->setEnabled( false );
    m_Add->setEnabled( false );
    m_New->setChecked( true );
  }
  else
  {
    m_Override->setEnabled( true );
    m_Override->setChecked( true );
    m_Add->setEnabled( true );
  }

  SetNodesByButtonGroup();
}

/** This slot enable and strikeout the nodes in subject to the selected radiobutton. The lineedit for the SeriesDescription and -Number get hide or show. */
void QmitkChiliPluginSaveDialog::SetNodesByButtonGroup()
{
  if( m_SeriesListView->childCount() == 0 )
  {
    for( std::list< NodeInputs >::iterator iter = m_NodeInputs.begin(); iter != m_NodeInputs.end(); iter++ )
    {
      iter->usedLabel->setEnabled( true );
      QFont temp = iter->usedLabel->font();
      temp.setStrikeOut( false );
      iter->usedLabel->setFont( temp );
    }
    m_SeriesNumber->show();
    m_SeriesDescription->show();
    return;
  }

  QmitkPluginListViewItem* entry = dynamic_cast<QmitkPluginListViewItem*>( m_SeriesListView->selectedItem() );
  std::string selectedSeriesOID = entry->GetChiliOID().ascii();

  for( std::list< NodeInputs >::iterator iter = m_NodeInputs.begin(); iter != m_NodeInputs.end(); iter++ )
  {
    //set enabled
    if( ( m_Add->isChecked() ||  m_Override->isChecked() ) && iter->SeriesOID == selectedSeriesOID )
      iter->usedLabel->setEnabled( false );
    else
      iter->usedLabel->setEnabled( true );
    //set strikeout
    QFont temp = iter->usedLabel->font();
    if( ( m_Override->isChecked() && iter->SeriesOID == selectedSeriesOID && !iter->canBeOverride ) || ( m_Add->isChecked() && iter->SeriesOID == selectedSeriesOID ) )
      temp.setStrikeOut( true );
    else
      temp.setStrikeOut( false );
    iter->usedLabel->setFont( temp );
    //create new series
    if( m_New->isChecked() )
    {
      m_SeriesNumber->show();
      m_SeriesDescription->show();
    }
    else
    {
      m_SeriesNumber->hide();
      m_SeriesDescription->hide();
    }
  }
}

/** If the user want to create a new series, this slot check if the seriesDescription is not empty. */
void QmitkChiliPluginSaveDialog::CheckOutputs()
{
  if( m_New->isChecked() && m_SeriesDescription->text().ascii() == "" )
  {
    QMessageBox::information( 0, "MITK", "You want to create a series without Description?\nPlease fill the Description." );
    m_SeriesDescription->setFocus();
    return;
  }

  //quit the dialog
  accept();
}

/** This slot show a messagebox with helpinformation. */
void QmitkChiliPluginSaveDialog::ShowHelp()
{
  QMessageBox::information( 0, "MITK", "The node-name can be enabled or disabled\nA enabled node means, that the node dont exist in the current selected series and you can change the name.\nA disable node means, that the node always exist in the current selected series.\nDisabled ones can be stikeout or not.\nNo strikeout means, that the existing entry get overridden and therefore the original node-name get used.\nA strikeout node-name means, that you have no rights to override the existing entry.\nIn this case the node dont get saved. Only MBI-saved-Data can be overridden." );
}
