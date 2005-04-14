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
#include "mitkTransferFunctionProperty.h"

void QmitkTransferFunctionWidget::SetDataTreeNode( mitk::DataTreeNode* node)
{
  if (mitk::TransferFunctionProperty::Pointer tfp = dynamic_cast<mitk::TransferFunctionProperty*>(node->GetProperty("TransferFunction").GetPointer()))
  {
    if (tfp)
    {
      mitk::TransferFunction* tf = dynamic_cast<mitk::TransferFunction*>(tfp->GetValue().GetPointer());
      if (tf)
      {
        std::cout << "TF access" << std::endl;
        m_TransferFunctionCanvas->SetTransferFunction(tf);
        UpdateMinMaxLabels();
      }
    }
  }
}

void QmitkTransferFunctionWidget::PropertyChange( QListViewItem * PropertyItem )
{
    return;
  /* if (NodeViewPropertyItem* nvpi = dynamic_cast<NodeViewPropertyItem*>(PropertyItem))
  {
    mitk::TransferFunctionProperty* tfp = dynamic_cast<mitk::TransferFunctionProperty*>(nvpi->GetProperty().GetPointer());
    if (tfp)
    {
      mitk::TransferFunction* tf = dynamic_cast<mitk::TransferFunction*>(tfp->GetValue().GetPointer());
      if (tf)
      {
        std::cout << "TF access" << std::endl;
        m_TransferFunctionCanvas->SetTransferFunction(tf);
        UpdateMinMaxLabels();
      }
    }
  } */
}

void QmitkTransferFunctionWidget::UpdateMinMaxLabels()
{
  if (m_TransferFunctionCanvas->GetTransferFunction().IsNotNull())
  {
    m_MinLabel->setText(QString::number(m_TransferFunctionCanvas->GetTransferFunction()->GetMin()));
    m_MaxLabel->setText(QString::number(m_TransferFunctionCanvas->GetTransferFunction()->GetMax()));
  }
}
