#include "ToolBar.h"
#include <qpushbutton.h>
#include <qcheckbox.h>
#include "mitkLightBoxImageReader.h"
#include <qlayout.h>
#include <qbuttongroup.h>
#include <chili/qclightboxmanager.h>
#include <qptrlist.h>


ToolBar::ToolBar(QWidget* parent,QcPlugin* qcplugin):
task(parent),
plugin(qcplugin),
idLightbox(0),
m_KeepDataTreeNodes(false)
{
    /*task=parent;
    plugin=qcplugin;
    idLightbox=0;
    m_KeepDataTreeNodes=false;*/

    toolbar=new QButtonGroup(parent);
    toolbar->setMaximumHeight(40);
    layout=new QGridLayout(parent,2,0,0,0);

    layout->addWidget(toolbar,2,0);

    QGridLayout* layout2=new QGridLayout(toolbar,1,6,5);

    int i=1;
    for(;i<6;++i)
    {
      // for some strange reason the "text" is not accepted
      // when set in the constructor, therefore it is set
      // in ConnectButton below.
      QPushButton* button= new QPushButton(toolbar, QString("toolbarSelectLightBox%1").arg(i).latin1());
      button->setToggleButton(true);
      layout2->addWidget(button,1,i-1);
      toolbar->insert(button,i);
    }

    //KeepDataTree Checkbox
    QCheckBox* cb = new QCheckBox( toolbar, "ToolbarCB" );
    layout2->addWidget(cb,1,i-1);
    toolbar->insert(cb,i++);

    //ConferenceButton
    QPushButton* button= new QPushButton(toolbar, QString("toolbarSelectLightBox%1").arg(i).latin1());
    layout2->addWidget(button,1,i-1);
    toolbar->insert(button,i);

    connect(toolbar,SIGNAL(clicked(int)),this,SLOT(ButtonToggled(int)));
    //for (int i=1;i<5;++i)
    //  connect(toolbar->find(i),SIGNAL(clicked(int)),this,SLOT(ButtonToggled(int)));
    //connect(toolbar,SIGNAL(clicked(int)),this,SLOT(ButtonToggled(int))); // all buttons
    connect(toolbar->find(5),SIGNAL(toggled(bool)),this,SLOT(Reinitialize(bool)));
    connect(toolbar->find(6),SIGNAL(toggled(bool)),this,SLOT(KeepDataTreeNodesCBtoggled(bool)));

    //toolbar->setExclusive(false);
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
  layout->setRowStretch(1,0);
  connect(toolbar,SIGNAL(clicked(int)),widget,SLOT(show()));
  widget->show();
  /////itkGenericOutputMacro(<<"set widget");
}

void ToolBar::ConnectButton(int number)
{
  for (int i=1;i<=number;++i)
    toolbar->find(i)->show();

  for (int i=number+1;i<5;++i)
    toolbar->find(i)->hide();

  SetButtonText();
  /////itkGenericOutputMacro(<<"connect buttons");
}

void ToolBar::SetButtonText()
{  
  // for some strange reason the "text" is not accepted
  // when set in the constructor (if the plugin was already
  // enabled on chili-startup - if it was not enabled in the
  // beginning but during the chili session, then it works !??)
  // Therefore we put it here.
  for(int i=1;i<5;++i)
  {
    toolbar->find(i)->setText(QString("LightBox %1").arg(i));
  }
  toolbar->find(5)->setText(QString("Reinitialize"));
  toolbar->find(6)->setText(QString("Keep data tree"));
  toolbar->find(7)->setText(QString("Conference INFO"));
}

void ToolBar::Reinitialize(bool on)
{
  ((QPushButton*)(toolbar->find(5)))->setOn ( false );
  
  //force new instantiation of App
  emit ChangeWidget(true);

  ResetLightboxButtons();
}

void ToolBar::ButtonToggled(int lbId)
{
  if(lbId>4) return;

  // reset lightbox buttons
  if (m_KeepDataTreeReinitNeeded)
  {
    ResetLightboxButtons(lbId);
    m_KeepDataTreeReinitNeeded=false;
    //emit ChangeWidget();
  }

  // show buttons toggled the way needed
  if (!m_KeepDataTreeNodes)
  {
    this->ResetLightboxButtons(lbId);
  }
  else
  {
    ((QPushButton*)(toolbar->find(lbId)))->setOn(true);
  }

  // select the lightbox
  if(((QPushButton*)(toolbar->find(lbId)))->isOn())
  {
    SelectLightbox(lbId-1);
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

void ToolBar::KeepDataTreeNodesCBtoggled(bool on)
{
  m_KeepDataTreeNodes = on;
  int lb;
  int lbcount=0;
  
  for(int i=1;i<5;++i)
  {
    if(((QPushButton*)(toolbar->find(i)))->isOn())
    {
      lb = i;
      lbcount++;
    }
  }

  // change style to slecte only one button or more.
  m_KeepDataTreeReinitNeeded = (!m_KeepDataTreeNodes && lbcount>1)?true:false;

  //if(!m_KeepDataTreeNodes && lbcount>1) 
  //{
    //emit ChangeWidget(true);
    //this->ResetLightboxButtons(idLightbox+1);
    //SelectLightbox(idLightbox);
  //}

}

bool ToolBar::KeepDataTreeNodes()
{
  return m_KeepDataTreeNodes;
}

void ToolBar::ResetLightboxButtons(int select)
{
  for(int i=1;i<5;++i)
  {
    if(((QPushButton*)(toolbar->find(i)))->isOn() && i!=select)
    {
      ((QPushButton*)(toolbar->find(i)))->setOn(false);
    }
  }
}