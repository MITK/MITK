/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you want to add, delete, or rename functions or slots, use
** Qt Designer to update this file, preserving your code.
**
** You should not define a constructor or destructor in this file.
** Instead, write your code in functions called init() and destroy().
** These will automatically be called by the form's constructor and
** destructor.
*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <qcombobox.h>

void QmitkRigidRegistrationControls::init()
{
  m_FixedDimension = 0;
  m_MovingDimension = 0;
  translationParams = new int[3];
  connect(this,SIGNAL(calculateRigidRegistration()),qmitkRigidRegistrationSelector1,SLOT(CalculateTransformation()));
  connect(this,SIGNAL(loadRigidRegistrationParameter()),qmitkRigidRegistrationSelector1,SLOT(LoadRigidRegistrationParameter()));
  connect(this,SIGNAL(saveRigidRegistrationParameter()),qmitkRigidRegistrationSelector1,SLOT(SaveRigidRegistrationParameter()));
  connect(this,SIGNAL(stopOptimization(bool)),qmitkRigidRegistrationSelector1,SLOT(StopOptimization(bool)));
  connect(qmitkRigidRegistrationSelector1,SIGNAL(OptimizerChanged(double)),this,SLOT(SetOptimizerValue( double )));
  connect(qmitkRigidRegistrationSelector1,SIGNAL(TransformChanged()),this,SLOT(CheckCalculateEnabled()));
  connect(qmitkRigidRegistrationSelector1,SIGNAL(AddNewTransformationToUndoList()),this,SLOT(AddNewTransformationToUndoList()));
  this->CheckCalculateEnabled();
  //this->registrationTabChanged(m_RigidTransform->currentPage());
}
void QmitkRigidRegistrationControls::SaveModelClicked()
{
  emit saveModel();
}

void QmitkRigidRegistrationControls::UndoTransformationClicked()
{
  emit undoTransformation();
  this->CheckCalculateEnabled();
}

void QmitkRigidRegistrationControls::ReinitFixedClicked()
{
  if (m_FixedSelector->GetSelectedNode() != NULL)
  {
    mitk::DataTreeNode* node = m_FixedSelector->GetSelectedNode();
    if (node != NULL )
    {
      mitk::BaseData::Pointer basedata = node->GetData();
      emit reinitFixed( basedata->GetTimeSlicedGeometry() );
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }
  }
}

void QmitkRigidRegistrationControls::ReinitMovingClicked()
{
    if (m_MovingSelector->GetSelectedNode() != NULL)
  {
    mitk::DataTreeNode* node = m_MovingSelector->GetSelectedNode();
    if (node != NULL )
    {
      mitk::BaseData::Pointer basedata = node->GetData();
      emit reinitMoving( basedata->GetTimeSlicedGeometry() );
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }
  }
}

void QmitkRigidRegistrationControls::GlobalReinitClicked()
{
  emit globalReinit( (dynamic_cast<mitk::DataTreeIteratorBase*> (m_FixedSelector->m_DataTreeIteratorClone.GetPointer())) );
}

void QmitkRigidRegistrationControls::MovingImageOpacityChanged( int value )
{
  float fValue = ((float)value)/100.0f;
  emit opacityChanged(fValue);
}

void QmitkRigidRegistrationControls::ShowImagesRedGreenSelected( bool show )
{
  emit showImagesRedGreen(show);
}

void QmitkRigidRegistrationControls::RedoTransformationClicked()
{
  emit redoTransformation();
}

void QmitkRigidRegistrationControls::SetUndoEnabled( bool enable )
{
  m_UndoTransformation->setEnabled(enable);
}

void QmitkRigidRegistrationControls::SetRedoEnabled( bool enable )
{
  m_RedoTransformation->setEnabled(enable);
}

void QmitkRigidRegistrationControls::CheckCalculateEnabled()
{
  if (m_MovingSelector->GetSelectedNode() != NULL && m_FixedSelector->GetSelectedNode() != NULL && m_RigidTransform->label(m_RigidTransform->currentPageIndex()) != "Manual")
  {
    m_CalculateTransformation->setEnabled(true);
    if (m_FixedDimension != m_MovingDimension || m_FixedDimension < 2 || m_FixedDimension > 3)
    {
      m_CalculateTransformation->setEnabled(false);
    }
    else if (qmitkRigidRegistrationSelector1->m_TransformBox->currentItem() < 5 &&  (m_FixedDimension < 2 || m_FixedDimension > 3))
    {
      m_CalculateTransformation->setEnabled(false);
    }
    else if ((qmitkRigidRegistrationSelector1->m_TransformBox->currentItem() > 4 && qmitkRigidRegistrationSelector1->m_TransformBox->currentItem() < 13) && m_FixedDimension != 3)
    {
      m_CalculateTransformation->setEnabled(false);
    }
    else if (qmitkRigidRegistrationSelector1->m_TransformBox->currentItem() > 12 &&  m_FixedDimension != 2)
    {
      m_CalculateTransformation->setEnabled(false);
    }
  }
  else
  {
    m_CalculateTransformation->setEnabled(false);
  }
}

void QmitkRigidRegistrationControls::ShowBothImagesSelected()
{
  emit showBothImages();
}

void QmitkRigidRegistrationControls::ShowFixedImageSelected()
{
  emit showFixedImage();
}

void QmitkRigidRegistrationControls::ShowMovingImageSelected()
{
  emit showMovingImage();
}

void QmitkRigidRegistrationControls::xTrans_valueChanged( int v)
{
  if (m_MovingSelector->GetSelectedNode() != NULL)
  {
    translationParams[0]=v;
	  translationParams[1]=m_YTransSlider->value();
	  translationParams[2]=m_ZTransSlider->value();
    QToolTip::remove(m_XTransSlider);
    char str[33];
    sprintf(str,"%d",v);    
    QToolTip::add( m_XTransSlider, tr( str ) );
    emit translateMovingImage(translationParams);
  }
  else
  {
    MovingImageChanged();
  }
}

void QmitkRigidRegistrationControls::yTrans_valueChanged( int v)
{
  if (m_MovingSelector->GetSelectedNode() != NULL)
  {
    translationParams[0]=m_XTransSlider->value();
	  translationParams[1]=v;
	  translationParams[2]=m_ZTransSlider->value();
    QToolTip::remove(m_YTransSlider);
    char str[33];
    sprintf(str,"%d",v);    
    QToolTip::add( m_YTransSlider, tr( str ) );
    emit translateMovingImage(translationParams);
  }
  else
  {
    MovingImageChanged();
  }
}

void QmitkRigidRegistrationControls::zTrans_valueChanged( int v)
{
  if (m_MovingSelector->GetSelectedNode() != NULL)
  {
    translationParams[0]=m_XTransSlider->value();
	  translationParams[1]=m_YTransSlider->value();
    translationParams[2]=v;
    QToolTip::remove(m_ZTransSlider);
    char str[33];
    sprintf(str,"%d",v);    
    QToolTip::add( m_ZTransSlider, tr( str ) );
    emit translateMovingImage(translationParams);
  }
  else
  {
    MovingImageChanged();
  }
}

void QmitkRigidRegistrationControls::MovingImageChanged()
{
  if (m_MovingSelector->GetSelectedNode() != NULL)
  {
    if (dynamic_cast<mitk::Image*>(m_MovingSelector->GetSelectedNode()->GetData()))
    {
      m_XTransSlider->setValue(0);
      m_YTransSlider->setValue(0);
      m_ZTransSlider->setValue(0);
      translationParams[0]=0;
      translationParams[1]=0;
      translationParams[2]=0;
      m_MovingDimension = dynamic_cast<mitk::Image*>(m_MovingSelector->GetSelectedNode()->GetData())->GetDimension();
      qmitkRigidRegistrationSelector1->SetMovingDimension(m_MovingDimension);
      qmitkRigidRegistrationSelector1->SetMovingNode(m_MovingSelector->GetSelectedNode());
      this->CheckCalculateEnabled();
    }
  }
}

void QmitkRigidRegistrationControls::FixedImageChanged()
{
  if (m_FixedSelector->GetSelectedNode() != NULL)
  {
    if (dynamic_cast<mitk::Image*>(m_FixedSelector->GetSelectedNode()->GetData()))
    {
      m_FixedDimension = dynamic_cast<mitk::Image*>(m_FixedSelector->GetSelectedNode()->GetData())->GetDimension();
      qmitkRigidRegistrationSelector1->SetFixedDimension(m_FixedDimension);
      qmitkRigidRegistrationSelector1->SetFixedNode(m_FixedSelector->GetSelectedNode());
      this->CheckCalculateEnabled();
    }
  }
}

void QmitkRigidRegistrationControls::Calculate()
{
  qmitkRigidRegistrationSelector1->SetFixedNode(m_FixedSelector->GetSelectedNode());
  qmitkRigidRegistrationSelector1->SetMovingNode(m_MovingSelector->GetSelectedNode());
  this->frame4->setEnabled(false);
  this->m_StopOptimization->setEnabled(true);
  if (m_RigidTransform->label(m_RigidTransform->currentPageIndex()) == "Automatic")
  {
    emit calculateRigidRegistration();
  }
  this->m_StopOptimization->setEnabled(false);
  emit (stopOptimization(false));
  this->frame4->setEnabled(true);
}

void QmitkRigidRegistrationControls::AutomaticAlignCenters()
{
  emit alignCenters();
}


void QmitkRigidRegistrationControls::SetOptimizerValue( double value )
{
  m_OptimizerValueLCD->display(value);
}

void QmitkRigidRegistrationControls::registrationTabChanged( QWidget * /*widget*/ )
{
  if (m_RigidTransform->currentPageIndex() == 0)
  {
    qmitkRigidRegistrationSelector1->show();
    m_ManualFrame->hide();
  }
  else if (m_RigidTransform->currentPageIndex() == 1)
  {
    qmitkRigidRegistrationSelector1->hide();
    m_ManualFrame->show();
  }
}

void QmitkRigidRegistrationControls::AddNewTransformationToUndoList()
{
  emit addNewTransformationToUndoList();
}

void QmitkRigidRegistrationControls::LoadRigidRegistrationParameterClicked()
{
  emit loadRigidRegistrationParameter();
}

void QmitkRigidRegistrationControls::SaveRigidRegistrationParameterClicked()
{
  emit saveRigidRegistrationParameter();
}

void QmitkRigidRegistrationControls::StopOptimizationClicked()
{
  emit stopOptimization(true);
}
