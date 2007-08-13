#include "QmitkChiliPluginDifferentStudiesDialog.h"

#include <qbuttongroup.h>
#include <qradiobutton.h>

#include <qpushbutton.h>
#include <qlayout.h>
#include <qlabel.h>

#include <sstream>

// the constructor create the widget
QmitkChiliPluginDifferentStudiesDialog::QmitkChiliPluginDifferentStudiesDialog( QWidget* parent, const char* name )
:QDialog( parent, name, true ), m_Done( false )
{
  QDialog::setCaption("Study selection");

  QDialog::setMinimumSize(250, 300);

  QBoxLayout * verticalLayout = new QVBoxLayout( this );
  verticalLayout->setMargin(5);
  verticalLayout->setSpacing(5);
  verticalLayout->addWidget( new QLabel( "MITK found different source-studies.\nIt's possible that the data to save have different source-studies or the current selected study is different to them.\nPlease select on study where you want do save all data.\n", this ) );

  m_ButtonGroup = new QButtonGroup( 0 );

  m_Series = new QGridLayout( verticalLayout );

  m_Series->setMargin(10);
  m_Series->setSpacing(10);

  m_Series->addWidget( new QLabel( "Patient Name", this ), 0, 1 );
  m_Series->addWidget( new QLabel( "Patient ID", this ), 0, 2 );
  m_Series->addWidget( new QLabel( "Study Description", this ), 0, 3 );
  m_Series->addWidget( new QLabel( "Study Date", this ), 0, 4 );
  m_Series->addWidget( new QLabel( "Modality", this ), 0, 5 );
  m_Series->addWidget( new QLabel( "Contain Node", this ), 0, 6 );

  m_Ok = new QPushButton( tr("OK"), this );
  m_Ok->setDefault(true);
  m_Ok->setFocus();
  connect( m_Ok, SIGNAL( clicked() ), this, SLOT( accept() ) );

  m_Abort = new QPushButton( tr("Cancel"), this );
  connect( m_Abort, SIGNAL( clicked() ), this, SLOT( reject() ) );
  QBoxLayout * horizontalLayout = new QHBoxLayout( verticalLayout );
  horizontalLayout->setSpacing(5);
  horizontalLayout->addStretch();
  horizontalLayout->addWidget( m_Ok );
  horizontalLayout->addWidget( m_Abort );
}

// destructor
QmitkChiliPluginDifferentStudiesDialog::~QmitkChiliPluginDifferentStudiesDialog()
{
}

std::string QmitkChiliPluginDifferentStudiesDialog::GetSelectedStudy()
{
  if( m_ButtonGroup->selected() == 0 )
    return "";
  else return m_ButtonGroup->selected()->name();
}

void QmitkChiliPluginDifferentStudiesDialog::addStudy( mitk::ChiliPlugin::StudyInformation newStudyInput, mitk::ChiliPlugin::PatientInformation newPatientInput, std::string nodeName )
{
  //check if the studyOID already shown
  bool found = false;
  int count = 0;

  while( count < m_StudyOIDs.size() && found == false )
  {
    if( m_StudyOIDs[ count ] == newStudyInput.OID )
    {
      found = true;
    }
    else
    {
      count++;
    }
  }

  if( found )
  //if study already exist, add current nodeName
  {
    m_Series->addWidget( new QLabel( nodeName.c_str(), this ), count, 6 );
  }
  else
  //study is new
  {
    int row = m_Series->numRows();

    QRadioButton *tempButton = new QRadioButton( this, newStudyInput.OID.c_str() );
    if( !m_Done)
    {
      tempButton->setChecked ( true );
      m_Done = true;
    }
    else tempButton->setChecked ( false );

    m_ButtonGroup->insert( tempButton );
    m_Series->addWidget( tempButton, row, 0 );

    m_Series->addWidget( new QLabel( newPatientInput.Name.c_str(), this ), row, 1 );
    m_Series->addWidget( new QLabel( newPatientInput.ID.c_str(), this ), row, 2 );
    m_Series->addWidget( new QLabel( newStudyInput.Description.c_str(), this ), row, 3 );
    m_Series->addWidget( new QLabel( newStudyInput.Date.c_str(), this ), row, 4 );
    m_Series->addWidget( new QLabel( newStudyInput.Modality.c_str(), this ), row, 5 );
    m_Series->addWidget( new QLabel( nodeName.c_str(), this ), row, 6 );
  }
}
