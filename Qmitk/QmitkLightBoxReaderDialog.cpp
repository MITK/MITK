#include "QmitkLightBoxReaderDialog.h"

#include <qlistbox.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qcursor.h>
#include <qapplication.h>

// the constructor create the widget
QmitkLightBoxReaderDialog::QmitkLightBoxReaderDialog( QWidget* parent, const char* name )
:QDialog( parent, name, true ),
 m_MaxCount(0)
{
  QDialog::setCaption("Image import");

  QDialog::setMinimumSize(250, 300);

  QBoxLayout * verticalLayout = new QVBoxLayout( this );
  verticalLayout->setMargin(5);
  verticalLayout->setSpacing(5);

  verticalLayout->addWidget( new QLabel( "Slices contain different spacings. \n Please select the spacing you want to use.", this ) );
  m_Spacings = new QListBox( this );

  verticalLayout->addWidget( m_Spacings );

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

  QApplication::restoreOverrideCursor();
}

// destructor
QmitkLightBoxReaderDialog::~QmitkLightBoxReaderDialog()
{
}

// return the selected spacing
mitk::Vector3D QmitkLightBoxReaderDialog::GetSpacing()
{
  QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
  return m_SpacingVector[ m_Spacings->currentItem() ];
}

// add a spacing to the listbox
void QmitkLightBoxReaderDialog::addSpacings( mitk::Vector3D spacing, int count )
{
  // convert a Vector3D and int to string
  std::ostringstream stringHelper;
  stringHelper << "Spacing: ";
  stringHelper << spacing[2];
  stringHelper << ", Count: ";
  stringHelper << count;
  std::string resultString = stringHelper.str();

  // put the string into the listbox
  m_Spacings->insertItem( resultString.c_str() );
  // save the spacing
  m_SpacingVector.push_back( spacing );

  // count the maximum
  if( count > m_MaxCount )
  {
    m_MaxCount = count;
    // select the item with the current highest count
    m_Spacings->setCurrentItem( m_Spacings->count()-1 );
  }
}
