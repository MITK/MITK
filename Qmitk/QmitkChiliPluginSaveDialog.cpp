#include "QmitkChiliPluginSaveDialog.h"

#include <qlistview.h>
#include <qlineedit.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qmessagebox.h>

#include <sstream>

//#include <mitkBaseProperty.h>

// the constructor create the widget
QmitkChiliPluginSaveDialog::QmitkChiliPluginSaveDialog( QWidget* parent, const char* name )
:QDialog( parent, name, true )
{
  QDialog::setCaption("Save To Chili");

  //QDialog::setMinimumSize( 250, 300 );

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
  for( unsigned int x = 0; x < m_StudyListView->columns(); x++ )
  {
    m_StudyListView->removeColumn( x );
  }
  m_StudyListView->addColumn( "Patient Name" );
  m_StudyListView->addColumn( "Patient ID" );
  m_StudyListView->addColumn( "Study Description" );
  m_StudyListView->addColumn( "Study OID" );
  m_StudyListView->setAllColumnsShowFocus ( true );
  connect( ( QObject* )( m_StudyListView ), SIGNAL( selectionChanged() ), ( QObject* ) this, SLOT( UpdateView() ) );
  gridLayout->addWidget( m_StudyListView, 1, 0 );

  //Series-Listview
  m_SeriesListView = new QListView( this );
  m_SeriesListView->setMargin( 3 );
  for( unsigned int x = 0; x < m_SeriesListView->columns(); x++ )
  {
    m_SeriesListView->removeColumn( x );
  }
  m_SeriesListView->addColumn( "Series Number" );
  m_SeriesListView->addColumn( "Series Description" );
  m_SeriesListView->addColumn( "Series OID" );
  m_SeriesListView->setAllColumnsShowFocus ( true );
  connect( ( QObject* )( m_SeriesListView ), SIGNAL( selectionChanged() ), ( QObject* ) this, SLOT( SetSeries() ) );
  gridLayout->addWidget( m_SeriesListView, 1, 1 );

  //Nodes to save
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
  QBoxLayout * horizontalLayout = new QHBoxLayout( 0 );
  horizontalLayout->setSpacing(5);
  horizontalLayout->addStretch();
  horizontalLayout->addWidget( okButton );
  horizontalLayout->addWidget( abortButton );
  gridLayout->addWidget( new QLabel( "If the node enabled, they get saved to the selected study and series and you can change the node-name.\nIf the node diabled, they get saved to the selected study and series but the node-name can not be changed, because the data get override.\nIf the node is strikeout, they dont get saved to the selected study and series.", this ), 4, 0 );
  gridLayout->addLayout( horizontalLayout, 4, 1 );
}

// destructor
QmitkChiliPluginSaveDialog::~QmitkChiliPluginSaveDialog()
{
}

QmitkChiliPluginSaveDialog::ReturnValue QmitkChiliPluginSaveDialog::GetSelection()
{
  ReturnValue newReturnValue;
  std::string studyOID = m_StudyListView->selectedItem()->text( 3 );
  newReturnValue.StudyOID = studyOID.c_str();
  std::string seriesOID = m_SeriesListView->selectedItem()->text( 2 );
  newReturnValue.SeriesOID = seriesOID.c_str();
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

QmitkChiliPluginSaveDialog::NewSeriesInformation QmitkChiliPluginSaveDialog::GetSeriesInformation()
{
  NewSeriesInformation newSeriesInformation;
  std::string seriesDescription = m_SeriesDescription->text();
  newSeriesInformation.SeriesDescription = seriesDescription.c_str();
  if( m_SeriesNumber->text() == "" )
    newSeriesInformation.SeriesNumber = 0;
  else
  {
    std::stringstream ssStream;
    ssStream << m_SeriesNumber->text();
    ssStream >> newSeriesInformation.SeriesNumber;
  }
  return newSeriesInformation;
}

void QmitkChiliPluginSaveDialog::AddStudy( std::string studyOID, std::string patientName, std::string patientID, std::string studyDescription )
{
  if( studyOID != "" )
  {
    //search if the study always exist
    bool exist = false;
    for( std::list< DifferentInputs >::iterator iter = m_Inputs.begin(); iter != m_Inputs.end(); iter ++ )
    {
      if( iter->StudyOID == studyOID )
      {
        exist = true;
        break;
      }
    }
    //if they dont exist, add
    if( exist == false )
    {
      DifferentInputs newInput;
      newInput.StudyOID = studyOID;
      newInput.PatientName = patientName;
      newInput.PatientID = patientID;
      newInput.StudyDescription = studyDescription;
      newInput.SeriesOID = "";
      newInput.SeriesNumber = "";
      newInput.SeriesDescription = "";
      newInput.Node = NULL;
      newInput.NodeDescriptionField = NULL;
      m_Inputs.push_back( newInput );
      //add study to dialog
      new QListViewItem( m_StudyListView, patientName.c_str(), patientID.c_str(), studyDescription.c_str(), studyOID.c_str() );
    }
  }
}

void QmitkChiliPluginSaveDialog::AddStudyAndSeries( std::string studyOID, std::string patientName, std::string patientID, std::string studyDescription, std::string seriesOID, int seriesNumber, std::string seriesDescription )
{
  if( studyOID != "" && seriesOID != "" )
  {
    //search if the study and series always exist
    bool studyAndSeriesExist = false;
    bool studyExist = false;
    for( std::list< DifferentInputs >::iterator iter = m_Inputs.begin(); iter != m_Inputs.end(); iter ++ )
    {
      if( iter->StudyOID == studyOID )
      {
        studyExist = true;
        if( iter->SeriesOID == seriesOID )
        {
          studyAndSeriesExist = true;
         break;
        }
      }
    }
    //if they dont exist, add
    if( studyAndSeriesExist == false )
    {
      DifferentInputs newInput;
      newInput.StudyOID = studyOID;
      newInput.PatientName = patientName;
      newInput.PatientID = patientID;
      newInput.StudyDescription = studyDescription;
      newInput.SeriesOID = seriesOID;
      if( seriesNumber == -1 )
        newInput.SeriesNumber = "";
      else
      {
        std::ostringstream stringHelper;
        stringHelper << seriesNumber;
        newInput.SeriesNumber = stringHelper.str();
      }
      newInput.SeriesNumber = seriesNumber;
      newInput.SeriesDescription = seriesDescription;
      newInput.Node = NULL;
      newInput.NodeDescriptionField = NULL;

      //add study only if they not exist
      if( studyExist == false )
        new QListViewItem( m_StudyListView, patientName.c_str(), patientID.c_str(), studyDescription.c_str(), studyOID.c_str() );

      m_Inputs.push_back( newInput );
    }
  }
}

void QmitkChiliPluginSaveDialog::AddStudySeriesAndNode( std::string studyOID, std::string patientName, std::string patientID, std::string studyDescription, std::string seriesOID, int seriesNumber, std::string seriesDescription, mitk::DataTreeNode::Pointer node )
{
  if( studyOID != "" && seriesOID != "" )
  //add new input
  {
    DifferentInputs newInput;
    newInput.StudyOID = studyOID;
    newInput.PatientName = patientName;
    newInput.PatientID = patientID;
    newInput.StudyDescription = studyDescription;
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
    if( node.IsNotNull() )
    {
      newInput.Node = node;
      mitk::BaseProperty::Pointer nameProperty = node.GetPointer()->GetProperty( "name" );
      QLineEdit* newEdit = new QLineEdit( this );
      newEdit->setText( nameProperty->GetValueAsString().c_str() );
      m_NodeLayout->addWidget( newEdit );
      newInput.NodeDescriptionField = newEdit;
    }

    //add study
    bool exist = false;
    for( std::list< DifferentInputs >::iterator iter = m_Inputs.begin(); iter != m_Inputs.end(); iter ++ )
    {
      if( iter->StudyOID == studyOID )
      {
        exist = true;
        break;
      }
    }
    //only if they not exist
    if( exist == false )
      new QListViewItem( m_StudyListView, patientName.c_str(), patientID.c_str(), studyDescription.c_str(), studyOID.c_str() );

    m_Inputs.push_back( newInput );
  }
}

void QmitkChiliPluginSaveDialog::AddNode( mitk::DataTreeNode::Pointer node )
{
  DifferentInputs newInput;
  newInput.StudyOID = "";
  newInput.PatientName = "";
  newInput.PatientID = "";
  newInput.StudyDescription = "";
  newInput.SeriesOID = "";
  newInput.SeriesNumber = "";
  newInput.SeriesDescription = "";
  if( node.IsNotNull() )
  {
    newInput.Node = node;
    mitk::BaseProperty::Pointer nameProperty = node.GetPointer()->GetProperty( "name" );
    QLineEdit* newEdit = new QLineEdit( this );
    newEdit->setText( nameProperty->GetValueAsString().c_str() );
    m_NodeLayout->addWidget( newEdit );
    newInput.NodeDescriptionField = newEdit;
  }
  m_Inputs.push_back( newInput );
}

void QmitkChiliPluginSaveDialog::UpdateView()
{
  //ensure that a study is selected
  if( m_StudyListView->selectedItem() == 0 )
  {
    //select the first one
    m_StudyListView->setSelected( m_StudyListView->firstChild(), true );
  }

  //get the studyOID from the selected study
  std::string showSeriesWithStudyOID = m_StudyListView->selectedItem()->text( 3 );
  m_SeriesListView->clear();
  //search and show all series with the selected studyOID
  for( std::list< DifferentInputs >::iterator iter = m_Inputs.begin(); iter != m_Inputs.end(); iter ++ )
  {
    if( iter->StudyOID == showSeriesWithStudyOID )
    {
      //insert them only, if the series didnt show yet
      if( m_SeriesListView->findItem( iter->SeriesOID.c_str(), 2, Qt::ExactMatch ) == 0 )
      {
        new QListViewItem( m_SeriesListView, iter->SeriesNumber.c_str(), iter->SeriesDescription.c_str(), iter->SeriesOID.c_str() );
      }
    }
  }

  SetSeries();
}

void QmitkChiliPluginSaveDialog::SetSeries()
{
  //ensure that a series is selected
  if( m_SeriesListView->selectedItem() == 0 )
    m_SeriesListView->setSelected( m_SeriesListView->firstChild(), true );

  //if no series exist
  if( m_SeriesListView->selectedItem()->text( 2 ) == "" )
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

void QmitkChiliPluginSaveDialog::SetNodesByButtonGroup()
{
  std::string selectedSeriesOID = m_SeriesListView->selectedItem()->text( 2 );
  for( std::list< DifferentInputs >::iterator iter = m_Inputs.begin(); iter != m_Inputs.end(); iter ++ )
  {
    if( iter->NodeDescriptionField != NULL )
    {
      mitk::BaseProperty::Pointer manufacturerProperty = iter->Node->GetProperty( "CHILI: MANUFACTURER" );
      mitk::BaseProperty::Pointer institutionNameProperty = iter->Node->GetProperty( "CHILI: INSTITUTION NAME" );
      //set enable
      if( ( m_Add->isChecked() ||  m_Override->isChecked() ) && iter->SeriesOID == selectedSeriesOID )
      {
        iter->NodeDescriptionField->setEnabled( false );
      }
      else
      {
        iter->NodeDescriptionField->setEnabled( true );
      }
      //set strikeout
      QFont temp = iter->NodeDescriptionField->font();
      if( iter->SeriesOID == selectedSeriesOID && ( ( m_Override->isChecked() && ( manufacturerProperty.IsNull() || manufacturerProperty->GetValueAsString() != "MITK" || institutionNameProperty.IsNull() || institutionNameProperty->GetValueAsString() != "DKFZ.MBI" ) ) || m_Add->isChecked() ) )
      {
        temp.setStrikeOut( true );
      }
      else
      {
        temp.setStrikeOut( false );
      }
      iter->NodeDescriptionField->setFont( temp );
      //set seriesDescription and seriesNumber
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
  m_SeriesListView->hideColumn( 2 );
  m_StudyListView->hideColumn( 3 );
}

void QmitkChiliPluginSaveDialog::CheckOutputs()
{
  if( m_New->isChecked() && m_SeriesDescription->text() == "" )
  {
    QMessageBox::information( 0, "MITK", "You want to create a series without Description?\nPlease fill the Description." );
    m_SeriesDescription->setFocus();
    return;
  }

  for( std::list< DifferentInputs >::iterator iter = m_Inputs.begin(); iter != m_Inputs.end(); iter ++ )
  {
    //its possible that only the current selected series and study added, this entry have no nodelineedit
    if( iter->NodeDescriptionField != NULL )
    {
      std::string nodeDescription = iter->NodeDescriptionField->text();
      if( nodeDescription != "" )
        iter->Node->SetProperty( "name", new mitk::StringProperty( nodeDescription ) );
      else
      {
        QMessageBox::information( 0, "MITK", "Found a node without Description.\nPlease fill all Node-Descriptions." );
        iter->NodeDescriptionField->setFocus();
       return;
      }
    }
  }
  //quit the dialog
  accept();
}
