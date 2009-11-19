#include "QmitkAboutDialog.h"


QmitkAboutDialog::QmitkAboutDialog(QWidget* parent, Qt::WindowFlags f)
:QDialog(parent, f)
{
  m_GUI = new Ui::QmitkAboutDialog;
  m_GUI->setupUi(this);

  //general
  //connect( m_GUI->btnQuitApplication,SIGNAL(clicked()), qApp, SLOT(closeAllWindows()) );

}

QmitkAboutDialog::~QmitkAboutDialog()
{

}









