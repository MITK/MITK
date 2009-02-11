#include "QmitkDataStorageComboBox.h"
#include "QmitkDataStorageListModel.h"

QmitkDataStorageComboBox::QmitkDataStorageComboBox(mitk::DataStorage::Pointer dataStorage, mitk::NodePredicateBase* pred, QWidget* parent)
: QComboBox(parent)
, m_DataStorageListModel(0)
{
  m_DataStorageListModel = new QmitkDataStorageListModel(dataStorage, pred, this);
  this->setModel(m_DataStorageListModel);

  connect(this, SIGNAL(currentIndexChanged(int)), this, SLOT(onCurrentIndexChanged(int)));
}

QmitkDataStorageComboBox::~QmitkDataStorageComboBox()
{
  // maybe the model will also get deleted by QComboBox, just to be sure
  delete m_DataStorageListModel;
  m_DataStorageListModel = 0;
}

void QmitkDataStorageComboBox::SetDataStorage(mitk::DataStorage::Pointer dataStorage)
{
  m_DataStorageListModel->SetDataStorage(dataStorage);
}

mitk::DataStorage::Pointer QmitkDataStorageComboBox::GetDataStorage() const
{
  return m_DataStorageListModel->GetDataStorage();
}

void QmitkDataStorageComboBox::OnCurrentIndexChanged(int index)
{
  if(index != -1)
  {
    const mitk::DataTreeNode::Pointer node = m_DataStorageListModel->GetDataNodes()->ElementAt(index);
    emit OnSelectionChanged(node);
  }
}
   
void QmitkDataStorageComboBox::SetPredicate(mitk::NodePredicateBase* pred)
{
  m_DataStorageListModel->SetPredicate(pred);
}

mitk::NodePredicateBase* QmitkDataStorageComboBox::GetPredicate() const
{
  return m_DataStorageListModel->GetPredicate();
}
   
