#include "ToolBar.h"
#include <qpushbutton.h>
#include "mitkLightBoxImageReader.h"
#include <qlayout.h>
#include <qbuttongroup.h>

ToolBar::ToolBar(QWidget* parent,QWidget* ap)
{
    
    toolbar=new QButtonGroup(parent);
    toolbar->setMaximumHeight(40);
    layout=new QGridLayout(parent,2,0,0,0);
    toolbar->setExclusive(true);
    //widget=ap;
    //layout->addWidget(widget,1,0); 
    layout->addWidget(toolbar,2,0); 
    
    //widget->hide();
    QGridLayout* layout2=new QGridLayout(toolbar,1,5,5);
   
    for(int i=1;i<5;++i)
    {
      QPushButton* button= new QPushButton(QString("%1").arg(i), toolbar);
      button->setToggleButton(true);
      layout2->addWidget(button,1,i-1);
      toolbar->insert(button,i);
    }
    QPushButton* button= new QPushButton(QString("re-initialize"), toolbar);
    button->setToggleButton(true);
    layout2->addWidget(button,1,4);
    toolbar->insert(button,5);
            
}

ToolBar::~ToolBar()
{
  
}       

QButtonGroup* ToolBar::GetToolBar()
{
  return toolbar;
}


void ToolBar::SetWidget(QWidget* ap)
{
  widget=ap;
  widget->hide();
  layout->addWidget(widget,1,0);
  for (int i=1;i<5;++i)
    connect(toolbar->find(i),SIGNAL(clicked()),widget,SLOT(show()));
  connect(toolbar->find(5),SIGNAL(clicked()),widget,SLOT(hide()));
}


void ToolBar::ConnectButton(int number)
{
  for (int i=1;i<=number;++i)
    toolbar->find(i)->show();
    
  for (int i=number+1;i<5;++i)
    toolbar->find(i)->hide();
} 

//void ToolBar::ClickButton(bool on)
//{
//    itkGenericOutputMacro(<<"connect widget");
//    int id=toolbar->id(toolbar->selected());
//    
//    if (on==true)
//    { 
//      
//      widget->show();
//    }
//    else
//      widget->hide();
//}
