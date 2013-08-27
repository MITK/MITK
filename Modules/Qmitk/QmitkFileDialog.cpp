/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

//#define _USE_MATH_DEFINES
#include <QmitkFileDialog.h>

// MITK
#include <mitkIFileReader.h>

// STL Headers
#include <list>

//microservices
#include <usGetModuleContext.h>
#include <usModuleContext.h>
#include <usServiceProperties.h>

//QT
#include <qgroupbox.h>
#include <qcheckbox.h>
#include <mitkCommon.h>

// Test imports, delete later
#include <mitkAbstractFileReader.h>

class DummyReader : public mitk::AbstractFileReader
{
public:

  DummyReader(const DummyReader& other)
    : mitk::AbstractFileReader(other)
  {
  }

  DummyReader(const std::string& extension, int priority)
    : mitk::AbstractFileReader(extension, "This is a dummy description")
  {
    m_Priority = priority;
    //std::list<std::string> options;
    m_Options.push_front(std::make_pair("isANiceGuy", true));
    m_Options.push_front(std::make_pair("canFly", false));
    m_Options.push_front(std::make_pair("isAwesome", true));
    m_Options.push_front(std::make_pair("hasOptions", true));
    m_Options.push_front(std::make_pair("has more Options", true));
    m_Options.push_front(std::make_pair("has maaaaaaaany Options", true));
    m_ServiceReg = this->RegisterService();
  }

  ~DummyReader()
  {
    if (m_ServiceReg) m_ServiceReg.Unregister();
  }

  using mitk::AbstractFileReader::Read;

  virtual std::list< itk::SmartPointer<mitk::BaseData> >  Read(const std::istream& /*stream*/, mitk::DataStorage* /*ds*/ = 0)
  {
    std::list<mitk::BaseData::Pointer> result;
    return result;
  }

  virtual void SetOptions(const std::list< mitk::IFileReader::FileServiceOption >& options )
  {
    m_Options = options;
    //m_Registration.SetProperties(GetServiceProperties());
  }

private:

  DummyReader* Clone() const
  {
    return new DummyReader(*this);
  }

  us::ServiceRegistration<mitk::IFileReader> m_ServiceReg;
}; // End of internal dummy reader

const std::string QmitkFileDialog::VIEW_ID = "org.mitk.views.QmitkFileDialog";

//
//
//
//

//

//

//
//
//

QmitkFileDialog::QmitkFileDialog(QWidget* parent, Qt::WindowFlags f): QFileDialog(parent, f)
{
  this->setOption(QFileDialog::DontUseNativeDialog);

  DummyReader* dr = new DummyReader(".xsfd", 1000);

  CreateQtPartControl(this);
}

QmitkFileDialog::~QmitkFileDialog()
{
}

//////////////////// INITIALIZATION /////////////////////

void QmitkFileDialog::CreateQtPartControl(QWidget *parent)
{
  // cast own layout to gridLayout
  QGridLayout *layout = (QGridLayout*)this->layout();

  // creat groupbox for options
  QGroupBox *box = new QGroupBox(this);
  box->setTitle("Options:");
  box->setVisible(true);
  m_BoxLayout = new QGridLayout(box);
  box->setLayout(m_BoxLayout);
  layout->addWidget(box,4,0,1,3);

  this->CreateConnections();

  //  m_context = us::getmodulecontext();
}

void QmitkFileDialog::CreateConnections()
{
  connect( this, SIGNAL(currentChanged( const QString &)), this, SLOT(DisplayOptions( QString)) );
  connect( this, SIGNAL(fileSelected( const QString &)), this, SLOT(ProcessSelectedFile()) );
}

/////////////////////////// OPTIONS ///////////////////////////////

void QmitkFileDialog::DisplayOptions(QString path)
{
  ClearOptionsBox();

  std::list< mitk::IFileReader::FileServiceOption > options = QueryAvailableOptions(path.toStdString());
  int i = 0;
  while (options.size() > 0)
  {
    QCheckBox *checker = new QCheckBox(this);
    checker->setText( options.front().first.c_str() );
    checker->setChecked( options.front().second );
    options.pop_front();
    m_BoxLayout->addWidget(checker, i / 4, i % 4);
    i++;
  }
}

void QmitkFileDialog::ClearOptionsBox()
{
  if ( m_BoxLayout != NULL )
  {
    QLayoutItem* item;
    while ( ( item = m_BoxLayout->takeAt( 0 ) ) != NULL )
    {
      delete item->widget();
      delete item;
    }
  }
}

std::list< mitk::IFileReader::FileServiceOption > QmitkFileDialog::GetSelectedOptions()
{
  std::list<mitk::IFileReader::FileServiceOption> result;

  if ( m_BoxLayout != NULL )
  {
    QLayoutItem* item;
    while ( ( item = m_BoxLayout->takeAt( 0 ) ) != NULL )
    {
      QCheckBox* checker = dynamic_cast<QCheckBox*> (item->widget());
      if (checker)
      {
        mitk::IFileReader::FileServiceOption option = std::make_pair( checker->text().toStdString() , checker->isChecked() );
        result.push_back(option);
      }

      delete item->widget();
      delete item;
    }
  }
  return result;
}
