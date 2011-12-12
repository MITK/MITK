/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-03-21 19:27:37 +0100 (Sa, 21 Mrz 2009) $
Version:   $Revision: 16719 $ 

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "QmitkTrackingSourcesCheckBoxPanelWidget.h"

#include <QMessageBox>


QmitkTrackingSourcesCheckBoxPanelWidget::QmitkTrackingSourcesCheckBoxPanelWidget(QWidget* parent)  
: QWidget(parent), m_Controls(NULL), m_SourceCheckboxes(NULL), m_NavigationDatas(NULL), m_SelectedIds(NULL)
{
  this->CreateQtPartControl( this );
  m_SourceCheckboxes = new TrackingSourcesCheckboxes();
}

QmitkTrackingSourcesCheckBoxPanelWidget::~QmitkTrackingSourcesCheckBoxPanelWidget()
{
  delete m_SelectedIds;
  delete m_SourceCheckboxes;
  delete m_NavigationDatas;
}

void QmitkTrackingSourcesCheckBoxPanelWidget::CreateQtPartControl(QWidget *parent)
{
  if (!m_Controls)
  {
    // create GUI widgets
    m_Controls = new Ui::QmitkTrackingSourcesCheckBoxPanelWidgetControls;
    m_Controls->setupUi(parent);

    this->CreateConnections();
  }
}

void QmitkTrackingSourcesCheckBoxPanelWidget::CreateConnections()
{
  connect( (QPushButton*) m_Controls->m_ActionButton, SIGNAL(toggled(bool)), this, SLOT(OnPerformActionClicked(bool)) ) ;
  connect( (QPushButton*) m_Controls->m_ActionButton, SIGNAL(clicked()), this, SLOT(OnPerformActionClicked()) ) ;
}


void QmitkTrackingSourcesCheckBoxPanelWidget::SetNavigationDatas(std::vector<mitk::NavigationData::Pointer>* navDatas)
{ 
  if( navDatas != NULL )
    m_NavigationDatas = navDatas;
}


void QmitkTrackingSourcesCheckBoxPanelWidget::AddNavigationData(mitk::NavigationData::Pointer nd)
{
  if(m_NavigationDatas == NULL)
    m_NavigationDatas = new std::vector<mitk::NavigationData::Pointer>();

  if( nd.IsNotNull() )
    m_NavigationDatas->push_back(nd);
}

bool QmitkTrackingSourcesCheckBoxPanelWidget::IsActionButtonChecked(){
  return m_Controls->m_ActionButton->isChecked();
}


const std::vector<int>* QmitkTrackingSourcesCheckBoxPanelWidget::GetSelectedTrackingSourcesIDs()
{
  if(m_SelectedIds == NULL)
    m_SelectedIds = new std::vector<int>();
  else
    m_SelectedIds->clear();

  for (unsigned int i=0; i < m_SourceCheckboxes->size(); i++)
  {
    if(m_SourceCheckboxes->at(i)->isChecked())
      m_SelectedIds->push_back(i);
  }

  return m_SelectedIds;
}

void QmitkTrackingSourcesCheckBoxPanelWidget::ClearPanel()
{
  while(m_Controls->m_GridLayout->count() > 0)
  {
    QWidget* actWidget = m_Controls->m_GridLayout->itemAt(0)->widget();
    m_Controls->m_GridLayout->removeWidget(actWidget);
    delete actWidget;
  }

  if(m_SourceCheckboxes != NULL)
    m_SourceCheckboxes->clear();

  if(m_NavigationDatas != NULL)
    m_NavigationDatas->clear();


}

void QmitkTrackingSourcesCheckBoxPanelWidget::ClearSelectedIDs()
{
  if(m_SelectedIds != NULL && !m_SelectedIds->empty())
    m_SelectedIds->clear();
}

void QmitkTrackingSourcesCheckBoxPanelWidget::ShowSourceCheckboxes()
{
  if( m_SourceCheckboxes != NULL )
    m_SourceCheckboxes->clear();

  if( m_NavigationDatas == NULL )
    return;

  QCheckBox* checkBox;

  int row = 0;
  int column;

  for(unsigned int i=0; i < m_NavigationDatas->size(); i++)  // puts a radiobutton for every tracking source output in a 2 columns QGridLayout
  {
    column = i % 4;
    if( i>0 && i%4==0 )
      row++;

    QString name(m_NavigationDatas->at(i).GetPointer()->GetName());

    checkBox = new QCheckBox(name, this);

    connect( checkBox, SIGNAL(toggled(bool)), this , SLOT(OnCheckboxClicked(bool)) );

    m_SourceCheckboxes->push_back(checkBox);
    m_Controls->m_GridLayout->addWidget(checkBox,row,column);  

  }

}

void QmitkTrackingSourcesCheckBoxPanelWidget::EnableCheckboxes(bool enable)
{
  for(unsigned int i=0; i< m_SourceCheckboxes->size(); i++)
  {
    m_SourceCheckboxes->at(i)->setEnabled(enable);
  }
}

void QmitkTrackingSourcesCheckBoxPanelWidget::SelectAll()
{
  for(unsigned int i=0; i< m_SourceCheckboxes->size(); i++)
  {
    m_SourceCheckboxes->at(i)->setChecked(true);
  }
}

void QmitkTrackingSourcesCheckBoxPanelWidget::DeselectAll()
{
  for(unsigned int i=0; i< m_SourceCheckboxes->size(); i++)
  {
    m_SourceCheckboxes->at(i)->setChecked(false);
  }
}

void QmitkTrackingSourcesCheckBoxPanelWidget::SelectCheckbox(unsigned int idx)
{
  m_SourceCheckboxes->at(idx)->setChecked(true);
}

void QmitkTrackingSourcesCheckBoxPanelWidget::DeselectCheckbox(unsigned int idx)
{
  m_SourceCheckboxes->at(idx)->setChecked(false);
}


void QmitkTrackingSourcesCheckBoxPanelWidget::OnCheckboxClicked(bool checked)
{
  QCheckBox* sender = qobject_cast< QCheckBox* > (QObject::sender());

  if( sender == NULL )
    throw std::invalid_argument("No sender found!");

  int idx = -1;

  for(unsigned int i=0 ;i < m_SourceCheckboxes->size(); i++)
  {
    if(sender == m_SourceCheckboxes->at(i))
    {   idx=i;
    break;
    }    
  }

  if(idx>-1)
  {
    if(checked)
      emit Selected(idx);
    else
      emit Deselected(idx);
  }

}

void QmitkTrackingSourcesCheckBoxPanelWidget::SetInfoText(QString text)
{
  m_Controls->m_InfoLabel->setText(text);
}

void QmitkTrackingSourcesCheckBoxPanelWidget::SetActionPerformButtonText(QString text)
{
  m_Controls->m_ActionButton->setText(text);
}


void QmitkTrackingSourcesCheckBoxPanelWidget::HideActionPerformButton(bool hide)
{
  if(hide)
    m_Controls->m_ActionButton->hide();
  else
    m_Controls->m_ActionButton->show();
}


void QmitkTrackingSourcesCheckBoxPanelWidget::SetActionPerformButtonCheckable(bool checkable)
{
  if(checkable)
    m_Controls->m_ActionButton->setCheckable(true);
  else
    m_Controls->m_ActionButton->setCheckable(false);
}

void QmitkTrackingSourcesCheckBoxPanelWidget::OnPerformActionClicked(bool toggled)
{
  if(this->GetSelectedTrackingSourcesIDs()->empty())
  {
    m_Controls->m_ActionButton->setChecked(false);   
    return;
  }

  if(toggled)
  {
    bool invalidND = false;

    for(int i=0; i < this->GetSelectedTrackingSourcesIDs()->size(); ++i)
    {
      if(!(m_NavigationDatas->at(this->GetSelectedTrackingSourcesIDs()->at(i))->IsDataValid()))
        invalidND = true;
    }

    if(invalidND)
    {
      QMessageBox::warning(NULL, "Invalid Tracking Data", "One or more instruments are in invalid tracking state! Requested action can not be performed!");
      m_Controls->m_ActionButton->setChecked(false);
      return;
    }
    emit PerformAction();
  }
  else
    emit StopAction();
}

void QmitkTrackingSourcesCheckBoxPanelWidget::OnPerformActionClicked()
{
  if(this->GetSelectedTrackingSourcesIDs()->empty()){
    m_Controls->m_ActionButton->setChecked(false);
    return;   
  }

  emit Action();
}