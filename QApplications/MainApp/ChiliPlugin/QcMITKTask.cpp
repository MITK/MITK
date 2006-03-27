#include <QcMITKTask.h>
#include <iostream>

QcMITKTask::QcMITKTask(const char** xpm, QWidget* parent, const char* name) :
            QcTask(xpm, parent, name){};
QcMITKTask::~QcMITKTask()
{
}

void
QcMITKTask::showEvent(QShowEvent *se)
{
  std::cout<<"SHOW"<<std::endl;
}

void
QcMITKTask::hideEvent(QHideEvent *he)
{
  std::cout<<"HIDE"<<std::endl;
}


