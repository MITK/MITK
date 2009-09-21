#ifndef QmitkPropertiesTableEditor_h
#define QmitkPropertiesTableEditor_h

/// Own includes.
#include "mitkDataTreeNode.h"

/// Toolkit includes.
#include <QWidget>

/// Forward declarations.
class QmitkPropertiesTableModel;
class QTableView;
class QLineEdit;

/// 
/// \class QmitkPropertiesTableEditor
/// \brief Combines a QTableView along with a QmitkPropertiesTableModel to a reusable
/// Property Editor component.
///
/// \see QmitkPropertyDelegate
class QMITK_EXPORT QmitkPropertiesTableEditor : public QWidget
{
  Q_OBJECT

public:

  ///
  /// Constructs a new QmitkDataStorageTableModel 
  /// and sets the DataTreeNode for this TableModel.
  QmitkPropertiesTableEditor(QWidget* parent = 0, Qt::WindowFlags f = 0,mitk::DataTreeNode::Pointer _Node = 0);

  ///
  /// Standard dtor. Nothing to do here.
  virtual ~QmitkPropertiesTableEditor();

  ///
  /// Convenience method. Sets the property list in the model.
  ///
  void SetPropertyList(mitk::PropertyList::Pointer _List);

  ///
  /// Get the model.
  /// 
  QmitkPropertiesTableModel* getModel() const;

  protected slots:
    void PropertyFilterKeyWordTextChanged(const QString & text);
protected:  
  ///
  /// Initialise empty GUI.
  ///
  virtual void init();
  ///
  /// The table view that renders the property list.
  /// 
  QTableView* m_NodePropertiesTableView;
  ///
  /// A text field in which the user can enter a filter keyword for the properties. Only properties containing with this keyword
  /// will be selected.
  /// 
  QLineEdit* m_TxtPropertyFilterKeyWord;
  ///
  /// The property list table model.
  /// 
  QmitkPropertiesTableModel* m_Model;
};

#endif /* QMITKPROPERTIESTABLEMODEL_H_ */
