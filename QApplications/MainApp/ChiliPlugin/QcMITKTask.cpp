#include <QcMITKTask.h>

QcMITKTask::QcMITKTask(const char** xpm, QWidget* parent, const char* name) :
            QcTask(xpm, parent, name), m_Visible(true)
{
  isImageTask( true );
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

