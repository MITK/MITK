#include "ToolBar.h"
#include <qpushbutton.h>
#include "mitkLightBoxImageReader.h"
#include <qlayout.h>
#include <qbuttongroup.h>
#include <qptrlist.h>


ToolBar::ToolBar(QWidget* parent,QcPlugin* qcplugin)
{
    task=parent;
    plugin=qcplugin;
    idLightbox=0;

    toolbar=new QButtonGroup(parent);
    toolbar->setMaximumHeight(40);
    layout=new QGridLayout(parent,2,0,0,0);
    toolbar->setExclusive(true);
    
    layout->addWidget(toolbar,2,0); 
    
    QGridLayout* layout2=new QGridLayout(toolbar,1,6,5);
   
    for(int i=1;i<7;++i)
    {
      QPushButton* button= new QPushButton(QString("%1").arg(i), toolbar);
      button->setToggleButton(true);
      layout2->addWidget(button,1,i-1);
      toolbar->insert(button,i);
    }
    
    toolbar->find(5)->setText(QString("re-initialize"));
    toolbar->find(6)->setText(QString("mode multi"));
    for (int i=1;i<5;++i)
      connect(toolbar->find(i),SIGNAL(toggled(bool)),this,SLOT(ButtonToggled(bool)));
    connect(toolbar->find(5),SIGNAL(toggled(bool)),this,SLOT(Reinitialize(bool)));
    connect(toolbar->find(6),SIGNAL(toggled(bool)),this,SLOT(ToolbarMode(bool)));

}

ToolBar::~ToolBar()
{
  
}       

QButtonGroup* ToolBar::GetToolBar()
{
  return toolbar;
}


void ToolBar::SetWidget(QWidget* app)
{
  widget=app;
  widget->hide();
  layout->addWidget(widget,1,0);
  connect(toolbar,SIGNAL(clicked(int)),widget,SLOT(show()));
  /////itkGenericOutputMacro(<<"set widget");
}

void ToolBar::ConnectButton(int number)
{
  for (int i=1;i<=number;++i)
    toolbar->find(i)->show();
    
  for (int i=number+1;i<5;++i)
    toolbar->find(i)->hide();
  /////itkGenericOutputMacro(<<"connect buttons");
} 

void ToolBar::Reinitialize(bool on)
{
  if (!toolbar->find(6)->isOn())
  {
    if (on)
    {
      toolbar->setButton(idLightbox+1);
    } 
    /////itkGenericOutputMacro(<<"reinitialize");
  }
  else
  {
    this->ButtonToggled(true);
    ((QPushButton*)(toolbar->find(5)))->setOn ( false );
  }
}

void ToolBar::ButtonToggled(bool on)
{   
  if (!toolbar->find(6)->isOn())
  {
    if (on)
    {
      emit ChangeWidget();
      int id=toolbar->id(toolbar->selected());
      SelectLightbox(id-1);
    }
  }
  else
  {
    emit ChangeWidget();
    for(int i=1;i<5;++i)
      if (toolbar->find(i)->isOn())
          SelectLightbox(i-1);    
  }
}

void ToolBar::SelectLightbox(int id)
{
    idLightbox=id;
    QcLightboxManager *lbm=plugin->lightboxManager();
    QPtrList<QcLightbox> list;
    QcLightbox* lb;
    list=lbm->getLightboxes();  
    lb=list.take(idLightbox);
    emit LightboxSelected(lb);
    /////itkGenericOutputMacro(<<"select lightbox");
}

void ToolBar::ToolbarMode(bool on)
{
  if (on)
  {
    toolbar->setExclusive(false);
    emit ChangeWidget();    
  }
  else
  {
    for(int i=1;i<6;++i)
        ((QPushButton*)(toolbar->find(i)))->setOn ( false );
    toolbar->setExclusive(true);
  }
}