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

#include <mitkRenderingManager.h>

void QmitkPointBasedRegistrationControls::transfomationClassChanged(int i)
{
  m_Transform = i;
  m_Params->setTransformationType(i);
  checkCalculateEnabled(m_FixedPoints, m_MovingPoints);
  emit transformationChanged(m_Transform);
}

void QmitkPointBasedRegistrationControls::init()
{
  m_Transform = 0;
  m_MovingPoints = 0;
  m_FixedPoints = 0;
  m_MeanErrorLCD->hide();
  m_MeanError->hide();
  m_Params=new PointBasedRegistrationControlParameters();
  m_FixedPointListWidget->ShowPointSetActionButtons(true);
  mitk::Color fixedColor;
  fixedColor[0] = (0.0f);
  fixedColor[1] = (1.0f);
  fixedColor[2] = (1.0f);
  m_FixedPointListWidget->SetPointColor(fixedColor);
  m_FixedPointListWidget->SetName("PointBasedRegistrationNode");
  m_MovingPointListWidget->ShowPointSetActionButtons(true);
  mitk::Color movingColor;
  movingColor[0] = (1.0f);
  movingColor[1] = (1.0f);
  movingColor[2] = (0.0f);
  m_MovingPointListWidget->SetPointColor(movingColor);
  m_MovingPointListWidget->SetName("PointBasedRegistrationNode");
}

void QmitkPointBasedRegistrationControls::calculate()
{
  if (m_Transform == 0 || m_Transform == 1 || m_Transform == 2)
  {
    if (m_MovingPoints == 1 && m_FixedPoints == 1)
    {
      emit calculateIt(m_Params);
    }
    else
    {
      emit calculateWithICP(m_Params);
    }
  }
  else if (m_Transform == 3 || m_Transform == 4 || m_Transform == 5)
  {
    emit calculateIt(m_Params);  
  }
  else
  {
    emit calculateLandmarkWarping(m_Params);
  }
}

void QmitkPointBasedRegistrationControls::saveModelClicked()
{
  emit SaveModel();
}

void QmitkPointBasedRegistrationControls::UndoTransformationClicked()
{
  emit undoTransformation();
}

void QmitkPointBasedRegistrationControls::showRedGreen( bool show )
{
  emit showRedGreenValues(show);
}

void QmitkPointBasedRegistrationControls::opacityChanged( int value )
{
  float fValue = ((float)value)/100.0f;
  emit opacityValueChanged(fValue);
}

void QmitkPointBasedRegistrationControls::setUndoTransformationEnabled( bool enable )
{
  m_UndoTransformation->setEnabled(enable);
  m_SaveModel->setEnabled(enable);
}

bool QmitkPointBasedRegistrationControls::checkCalculateEnabled(int fixedLandmarks, int movingLandmarks)
{

  m_FixedPoints = fixedLandmarks;
  m_MovingPoints = movingLandmarks;
  if (m_Transform == 0 || m_Transform == 1 || m_Transform == 2)
  {
    if((movingLandmarks > 0 && fixedLandmarks > 0))
    {
      m_Calculate->setEnabled(true);
      return true;
    }
    else
    {
      m_Calculate->setEnabled(false);
      return false;
    }
  }
  else if (m_Transform == 3 || m_Transform == 4 || m_Transform == 5)
  {
    if ((movingLandmarks == fixedLandmarks) && movingLandmarks > 0)
    {
      m_Calculate->setEnabled(true);
      return true;
    }
    else
    {
      m_Calculate->setEnabled(false);
      return false;
    }
  }
  else
  {
    m_Calculate->setEnabled(true);
    return true;
  }
}

void QmitkPointBasedRegistrationControls::RedoTransformationClicked()
{
  emit redoTransformation();
}

void QmitkPointBasedRegistrationControls::setRedoTransformationEnabled( bool enable )
{
  m_RedoTransformation->setEnabled(enable);
}


void QmitkPointBasedRegistrationControls::reinitFixedClicked()
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

void QmitkPointBasedRegistrationControls::reinitMovingClicked()
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

void QmitkPointBasedRegistrationControls::globalReinitClicked()
{
  if(m_FixedSelector->GetSelectedNode() != NULL)
  {
    emit globalReinit( (dynamic_cast<mitk::DataTreeIteratorBase*> (m_FixedSelector->m_DataTreeIteratorClone.GetPointer())) );
  }
}

void QmitkPointBasedRegistrationControls::setInvisible(bool invisible)
{
  emit setImagesInvisible(invisible);
}
