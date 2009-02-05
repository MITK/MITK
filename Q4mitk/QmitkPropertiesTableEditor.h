#ifndef QmitkPropertiesTableEditor_h
#define QmitkPropertiesTableEditor_h

/// Own includes.
#include "mitkDataTreeNode.h"

/// Toolkit includes.
#include <QWidget>

/// Forward declarations.
class QmitkPropertiesTableModel;
class QTableView;

/// 
/// \class QmitkPropertiesTableEditor
/// \brief Combines a QTableView along with a QmitkPropertiesTableModel to a reusable
/// Property Editor component.
///
/// \see QmitkPropertyDelegate
class QMITK_EXPORT QmitkPropertiesTableEditor : public QWidget
{
public:

  ///
  /// Constructs a new QmitkDataStorageTableModel 
  /// and sets the DataTreeNode for this TableModel.
  QmitkPropertiesTableEditor(mitk::DataTreeNode::Pointer _Node = 0
    , QWidget* parent = 0, Qt::WindowFlags f = 0);

  ///
  /// Standard dtor. Nothing to do here.
  virtual ~QmitkPropertiesTableEditor();

  ///
  /// Convenience method. Sets the node in the model.
  ///
  void setNode(mitk::DataTreeNode::Pointer _Node);

  ///
  /// Convenience method. Get the node from the model.
  /// 
  mitk::DataTreeNode::Pointer getNode() const;

  ///
  /// Get the model.
  /// 
  QmitkPropertiesTableModel* getModel() const;

protected:  
  ///
  /// Initialise empty GUI.
  ///
  virtual void init();

  QTableView* m_NodePropertiesTableView;
  QmitkPropertiesTableModel* m_Model;
};

#endif /* QMITKPROPERTIESTABLEMODEL_H_ */
