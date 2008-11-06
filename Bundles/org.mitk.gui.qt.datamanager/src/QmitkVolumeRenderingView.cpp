#include "QmitkVolumeRenderingView.h"

#include <QComboBox>

#include <mitkProperties.h>
#include <mitkNodePredicateDataType.h>

void QmitkVolumeRenderingView::CreateQtPartControl(QWidget* parent)
{
  QVBoxLayout* layout = new QVBoxLayout(parent);

  QComboBox* comboBox = new QComboBox(parent);
  m_ComboModel =
    new QmitkDataStorageListModel(new mitk::NodePredicateDataType("Image"),
                                  comboBox);
  m_ComboModel->SetDataStorage(this->GetDataStorage());
  comboBox->setModel(m_ComboModel);

  layout->addWidget(comboBox);

  m_TransferFunctionWidget = new QmitkTransferFunctionWidget(parent);
  layout->addWidget(m_TransferFunctionWidget, 1);

  if (m_ComboModel->rowCount() > 0)
    this->comboIndexChanged(0);

  connect(comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(comboIndexChanged(int)));
}

void QmitkVolumeRenderingView::comboIndexChanged(int index)
{
  QModelIndex modelIndex = m_ComboModel->index(index);
  QVariant nodeVariant = m_ComboModel->data(modelIndex, QmitkDataTreeNodeRole);
  mitk::DataTreeNode::Pointer node = nodeVariant.value<mitk::DataTreeNode::Pointer>();

  node->SetProperty("volumerendering", mitk::BoolProperty::New(true));

  m_TransferFunctionWidget->SetDataTreeNode(node);
}

void QmitkVolumeRenderingView::SetFocus()
{

}

QmitkVolumeRenderingView::~QmitkVolumeRenderingView()
{

}

