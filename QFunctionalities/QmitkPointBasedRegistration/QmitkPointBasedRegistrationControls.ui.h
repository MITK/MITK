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
  checkCalculateEnabled();
  emit transformationChanged(m_Transform);
}

void QmitkPointBasedRegistrationControls::fixedSelected()
{
  emit addFixedInteractor();
}

void QmitkPointBasedRegistrationControls::movingSelected()
{
  emit addMovingInteractor();
}

void QmitkPointBasedRegistrationControls::init()
{
  m_Transform = 0;
  m_MovingPoints = 0;
  m_FixedPoints = 0;
  m_MeanErrorLCD->hide();
  m_MeanError->hide();
  m_Params=new PointBasedRegistrationControlParameters();
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

void QmitkPointBasedRegistrationControls::bothSelected()
{
  emit changeBothSelected();
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

void QmitkPointBasedRegistrationControls::ResetPointsetsClicked()
{
  emit ResetPointsets();
}

void QmitkPointBasedRegistrationControls::addFixedPoints( mitk::PointSet::Pointer fixedPoints )
{
  m_FixedPointset = fixedPoints;
  refreshTable();
}

void QmitkPointBasedRegistrationControls::addMovingPoints( mitk::PointSet::Pointer movingPoints )
{
  m_MovingPointset = movingPoints;
  refreshTable();
}

void QmitkPointBasedRegistrationControls::SetNoModelSelected()
{
  m_NoModelSelected->setChecked(true);
}

void QmitkPointBasedRegistrationControls::setUndoTransformationEnabled( bool enable )
{
  m_UndoTransformation->setEnabled(enable);
}

void QmitkPointBasedRegistrationControls::checkCalculateEnabled()
{
  if (m_Transform == 0 || m_Transform == 1 || m_Transform == 2)
  {
    if((m_MovingPoints > 0 && m_FixedPoints > 0))
    {
      m_Calculate->setEnabled(true);
    }
    else
    {
      m_Calculate->setEnabled(false);
    }
  }
  else if (m_Transform == 3 || m_Transform == 4 || m_Transform == 5)
  {
    if ((m_MovingPoints == m_FixedPoints) && m_MovingPoints > 0)
    {
      m_Calculate->setEnabled(true);
    }
    else
    {
      m_Calculate->setEnabled(false);
    }
  }
  else
  {
    m_Calculate->setEnabled(true);
  }
}

void QmitkPointBasedRegistrationControls::LandmarkSelected( int row, int col, int /*button*/, const QPoint & /*mousePos*/ )
{
 if (col == 0)
  {
    if (m_PointsetTable->text(row, col) != "")
    {
      if (!(m_FixedSelected->isChecked()))
      {
        m_FixedSelected->setChecked(true);
        emit addFixedInteractor();
      }
      emit selectFixedPoint(row);
    }
  }
  else if (col == 1)
  {
    if (m_PointsetTable->text(row, col) != "")
    {
      if (!(m_MovingSelected->isChecked()))
      {
        m_MovingSelected->setChecked(true);
        emit addMovingInteractor();
      }
      emit selectMovingPoint(row);
    }
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

void QmitkPointBasedRegistrationControls::refreshTable()
{
  //clear the table
  int numberOfRows = m_PointsetTable->numRows();
  for (int row = numberOfRows; row > 0; row--)
  {
    m_PointsetTable->removeRow(row-1);
  }

  m_FixedPoints = 0;
  if (m_FixedPointset.IsNotNull())
  {
    mitk::PointSet::DataType::Pointer ItkData;
    ItkData = m_FixedPointset->GetPointSet();
    if (ItkData.IsNotNull())
    {
      mitk::PointSet::PointDataIterator it = ItkData->GetPointData()->Begin();
      mitk::PointSet::PointDataIterator end = ItkData->GetPointData()->End();
      QString label;
      itk::Point<mitk::ScalarType, 3> point;
      int i = 0;
      for ( ;it != end; it++)
      {
        ItkData->GetPoints()->GetElementIfIndexExists(it->Index(), &point);
        label = "F " + QString::number(i+1);
        m_PointsetTable->insertRows(i);
        m_PointsetTable->setText(i, 0, label);
        i++;
      }
      m_FixedPoints = i;
    }
  }

  m_MovingPoints = 0;
  if (m_MovingPointset.IsNotNull())
  {
    mitk::PointSet::DataType::Pointer ItkData;
    ItkData = m_MovingPointset->GetPointSet();
    if (ItkData.IsNotNull())
    {
      mitk::PointSet::PointDataIterator it = ItkData->GetPointData()->Begin();
      mitk::PointSet::PointDataIterator end = ItkData->GetPointData()->End();
      QString label;
      itk::Point<mitk::ScalarType, 3> point;
      int j = 0;
      for ( ;it != end; it++)
      {
        ItkData->GetPoints()->GetElementIfIndexExists(it->Index(), &point);
        label = "M " + QString::number(j+1);
        if ( j + 1 > m_FixedPoints) 
        {
          m_PointsetTable->insertRows(j);
        }
        m_PointsetTable->setText(j, 1, label);
        j++;
      }
      m_MovingPoints = j;
    }
  }
  m_PointsetTable->repaint();
  checkCalculateEnabled();
}

void QmitkPointBasedRegistrationControls::setInvisible(bool invisible)
{
  emit setImagesInvisible(invisible);
}

void QmitkPointBasedRegistrationControls::loadFixedPointSetClicked()
{
  emit loadFixedPointSet();
}

void QmitkPointBasedRegistrationControls::saveFixedPointSetClicked()
{
  emit saveFixedPointSet();
}

void QmitkPointBasedRegistrationControls::loadMovingPointSetClicked()
{
  emit loadMovingPointSet();
}

void QmitkPointBasedRegistrationControls::saveMovingPointSetClicked()
{
  emit saveMovingPointSet();
}
