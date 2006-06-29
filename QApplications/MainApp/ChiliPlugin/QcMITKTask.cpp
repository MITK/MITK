#include <QcMITKTask.h>
#include <iostream>

QcMITKTask::QcMITKTask(const char** xpm, QWidget* parent, const char* name) :
            QcTask(xpm, parent, name), m_Visible(true){};
QcMITKTask::~QcMITKTask()
{
}


void QcMITKTask::showEvent(QShowEvent *se)
{
  VisibleOn();
  std::cout<<"SHOW"<<std::endl;
}


void QcMITKTask::hideEvent(QHideEvent *he)
{ 
  VisibleOff();
  std::cout<<"HIDE"<<std::endl;
}


bool QcMITKTask::IsVisible()
{
  return m_Visible;
}


void QcMITKTask::VisibleOn()
{
  m_Visible = true;
}


void QcMITKTask::VisibleOff()
{
  m_Visible = false;
}

