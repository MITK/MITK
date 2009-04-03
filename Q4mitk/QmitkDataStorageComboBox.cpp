#include "QmitkDataStorageComboBox.h"
#include "QmitkDataStorageListModel.h"

QmitkDataStorageComboBox::QmitkDataStorageComboBox(mitk::DataStorage::Pointer dataStorage, mitk::NodePredicateBase* pred, QWidget* parent
												   , bool _AutoSelectFirstItem)
: QComboBox(parent)
, m_DataStorageListModel(0)
, m_AutoSelectFirstItem(_AutoSelectFirstItem)
{
  init(dataStorage, pred);
}

QmitkDataStorageComboBox::QmitkDataStorageComboBox( QWidget* parent, bool _AutoSelectFirstItem )
: QComboBox(parent)
, m_DataStorageListModel(0)
, m_AutoSelectFirstItem(_AutoSelectFirstItem)
{
  init(0, 0);
}

QmitkDataStorageComboBox::~QmitkDataStorageComboBox()
{
  // maybe the model will also get deleted by QComboBox, just to be sure
  delete m_DataStorageListModel;
  m_DataStorageListModel = 0;
}

void QmitkDataStorageComboBox::SetDataStorage(mitk::DataStorage::Pointer dataStorage)
{
  m_DataStorageListModel->SetDataStorage(dataStorage.GetPointer());
}

mitk::DataStorage::Pointer QmitkDataStorageComboBox::GetDataStorage() const
{
  return m_DataStorageListModel->GetDataStorage();
}

void QmitkDataStorageComboBox::OnCurrentIndexChanged(int index)
{
  mitk::DataTreeNode* node = 0;
  if(index >= 0)
  {
    node = m_DataStorageListModel->GetDataNodes().at(index);
  }
  mitk::DataTreeNode::Pointer smartPtrToNode;
  smartPtrToNode = node;
  emit OnSelectionChanged(smartPtrToNode);
}
   
void QmitkDataStorageComboBox::SetPredicate(mitk::NodePredicateBase* pred)
{
  m_DataStorageListModel->SetPredicate(pred);
  if(m_AutoSelectFirstItem)
	  if(this->count() > 0)
		  this->setCurrentIndex(0);

}

mitk::NodePredicateBase* QmitkDataStorageComboBox::GetPredicate() const
{
  return m_DataStorageListModel->GetPredicate();
}

const mitk::DataTreeNode::Pointer QmitkDataStorageComboBox::GetSelectedNode() const
{
  mitk::DataTreeNode* node = 0;
  int _CurrentIndex = this->currentIndex();

  if(_CurrentIndex >= 0)
    node = m_DataStorageListModel->GetDataNodes().at(_CurrentIndex);

  mitk::DataTreeNode::Pointer smartPtrToNode;
  smartPtrToNode = node;
  return smartPtrToNode;
}

void QmitkDataStorageComboBox::init(mitk::DataStorage::Pointer dataStorage, mitk::NodePredicateBase* pred)
{
  m_DataStorageListModel = new QmitkDataStorageListModel(dataStorage, pred, this);
  this->setModel(m_DataStorageListModel);

  connect(this, SIGNAL(currentIndexChanged(int)), this, SLOT(OnCurrentIndexChanged(int)));
}
