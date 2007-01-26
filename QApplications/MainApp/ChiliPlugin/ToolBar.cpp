#include "ToolBar.h"
#include <qpushbutton.h>
#include <qcheckbox.h>
#include "mitkLightBoxImageReader.h"
#include <qlayout.h>
#include <qbuttongroup.h>
#include <chili/qclightboxmanager.h>
#include <qptrlist.h>


ToolBar::ToolBar(QWidget* parent,QcPlugin* qcplugin)
{
    task=parent;
    plugin=qcplugin;
    idLightbox=0;
    m_KeepDataTreeNodes=false;

    toolbar=new QButtonGroup(parent);
    toolbar->setMaximumHeight(40);
    layout=new QGridLayout(parent,2,0,0,0);
    toolbar->setExclusive(!m_KeepDataTreeNodes);

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
    button->setToggleButton(true);
    layout2->addWidget(button,1,i-1);
    toolbar->insert(button,i);

    for (int i=1;i<5;++i)
      connect(toolbar->find(i),SIGNAL(toggled(bool)),this,SLOT(ButtonToggled(bool)));
    //connect(toolbar,SIGNAL(clicked(int)),this,SLOT(ButtonToggled(int))); // all buttons
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
  /////itkGenericOutputMacro(<<"connect buttons");
}

void ToolBar::Reinitialize(bool on)
{
  if (!toolbar->find(6)->isOn())
  {
    if (on)
    {
      //force new instantiation of App
      emit ChangeWidget(true);
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
  if (m_KeepDataTreeNodes)
  {
     for(int i=1;i<5;++i)
      if (toolbar->find(i)->isOn())
          SelectLightbox(i-1);
  }
  else
  {
    if(on)
    {
      int id=toolbar->id(toolbar->selected());
      SelectLightbox(id-1);
    }

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
  m_KeepDataTreeNodes = on;

  if (m_KeepDataTreeNodes)
  {
    //TODO: when toggeled - button should be showed as pressed
    toolbar->setExclusive(false);
  }
  else
  {
    emit ChangeWidget();
    for(int i=1;i<6;++i)
        ((QPushButton*)(toolbar->find(i)))->setOn ( false );
    toolbar->setExclusive(true);
  }
}

bool ToolBar::KeepDataTreeNodes()
{
  return m_KeepDataTreeNodes;
}
