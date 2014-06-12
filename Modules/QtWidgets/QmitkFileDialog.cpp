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

//QT
#include <QGroupBox>
#include <QCheckBox>
#include <QGridLayout>


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
    this->SetRanking(priority);
    //std::vector<std::string> options;
    OptionList options;
    options.push_back(std::make_pair("isANiceGuy", true));
    options.push_back(std::make_pair("canFly", false));
    options.push_back(std::make_pair("isAwesome", true));
    options.push_back(std::make_pair("hasOptions", true));
    options.push_back(std::make_pair("has more Options", true));
    options.push_back(std::make_pair("has maaaaaaaany Options", true));
    this->SetOptions(options);
    m_ServiceReg = this->RegisterService();
  }

  ~DummyReader()
  {
    if (m_ServiceReg) m_ServiceReg.Unregister();
  }

  using mitk::AbstractFileReader::Read;

  virtual std::vector< itk::SmartPointer<mitk::BaseData> >  Read(std::istream& /*stream*/)
  {
    std::vector<mitk::BaseData::Pointer> result;
    return result;
  }

private:

  DummyReader* Clone() const
  {
    return new DummyReader(*this);
  }

  us::ServiceRegistration<mitk::IFileReader> m_ServiceReg;
}; // End of internal dummy reader


//
//
//
//

//

//

//
//
//

class QmitkFileDialogPrivate
{
public:

  /** \brief This method is part of the widget and needs not to be called separately. */
  void CreateQtPartControl(QWidget *parent);

  /** \brief Remove all checkboxes from the options box.*/
  void ClearOptionsBox();

  /** \brief Contains the checkboxes for the options*/
  QGridLayout* m_BoxLayout;

  /** \brief The Options the user has set for the reader / writer*/
  mitk::IFileReader::OptionList m_Options;
};

QmitkFileDialog::QmitkFileDialog(QWidget* parent, Qt::WindowFlags f)
  : QFileDialog(parent, f)
  , d(new QmitkFileDialogPrivate)
{
  this->setOption(QFileDialog::DontUseNativeDialog);

  DummyReader* dr = new DummyReader(".xsfd", 1000);

  d->CreateQtPartControl(this);
}

QmitkFileDialog::~QmitkFileDialog()
{
}

//////////////////// INITIALIZATION /////////////////////

void QmitkFileDialogPrivate::CreateQtPartControl(QWidget *parent)
{
  // cast own layout to gridLayout
  QGridLayout *layout = qobject_cast<QGridLayout*>(parent->layout());

  // creat groupbox for options
  QGroupBox *box = new QGroupBox(parent);
  box->setTitle("Options:");
  box->setVisible(true);
  m_BoxLayout = new QGridLayout(box);
  box->setLayout(m_BoxLayout);
  layout->addWidget(box,4,0,1,3);

  parent->connect( parent, SIGNAL(currentChanged(QString)), parent, SLOT(DisplayOptions( QString)) );
  parent->connect( parent, SIGNAL(fileSelected(QString)), parent, SLOT(ProcessSelectedFile()) );
}

/////////////////////////// OPTIONS ///////////////////////////////

void QmitkFileDialog::DisplayOptions(const QString& path)
{
  d->ClearOptionsBox();

  mitk::IFileReader::OptionList options = QueryAvailableOptions(path);
  int i = 0;
  for (mitk::IFileReader::OptionList::const_iterator iter = options.begin(),
       end = options.end(); iter != end; ++iter)
  {
    QCheckBox *checker = new QCheckBox(this);
    checker->setText( QString::fromStdString(iter->first) );
    checker->setChecked( iter->second );
    d->m_BoxLayout->addWidget(checker, i / 4, i % 4);
    ++i;
  }
}

void QmitkFileDialogPrivate::ClearOptionsBox()
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

mitk::IFileReader::OptionList QmitkFileDialog::GetSelectedOptions()
{
  mitk::IFileReader::OptionList result;

  if ( d->m_BoxLayout != NULL )
  {
    QLayoutItem* item;
    while ( ( item = d->m_BoxLayout->takeAt( 0 ) ) != NULL )
    {
      QCheckBox* checker = dynamic_cast<QCheckBox*> (item->widget());
      if (checker)
      {
        mitk::IFileReader::Option option( checker->text().toStdString() , checker->isChecked() );
        result.push_back(option);
      }

      delete item->widget();
      delete item;
    }
  }
  return result;
}
