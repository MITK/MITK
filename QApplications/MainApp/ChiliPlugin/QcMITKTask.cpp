#include <QcMITKTask.h>

QcMITKTask::QcMITKTask(const char** xpm, QWidget* parent, const char* name) :
            QcTask(xpm, parent, name), m_Visible(true)
{
  isImageTask( true );
  // this is only available in the CHILI SDK 3.14.1 or later. Unfortunately there is
  // a problem with the VERSION_CODE_MACRO (see bug 1418). If you get compilation errors here,
  // please comment this line.
  this->setLocalizable(false);
}


QcMITKTask::~QcMITKTask()
{
}


void QcMITKTask::showEvent(QShowEvent * /*se*/)
{
  setVisible( true );
}


void QcMITKTask::hideEvent(QHideEvent * /*he*/)
{ 
  setVisible( false );
}


bool QcMITKTask::IsVisible()
{
  return m_Visible;
}


void QcMITKTask::setVisible( bool isVisible )
{
  m_Visible = isVisible;
}

