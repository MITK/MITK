#ifndef QmitkNDIToolDelegate_h
#define QmitkNDIToolDelegate_h

/// Own includes.
#include "MitkIGTUIExports.h"
#include "mitkNodePredicateBase.h"
#include "mitkBaseProperty.h"
/// Toolkit includes.
#include <QStyledItemDelegate>

/// Forward declarations.
namespace mitk
{
  class DataStorage;
  class NodePredicateBase;
  class BaseProperty;
};

/// 
/// \class QmitkNDIToolDelegate
/// \brief An item delegate for rendering and editing mitk::Properties in a QTableView.
///
/// \see QmitkPropertiesTableModel
class MitkIGTUI_EXPORT QmitkNDIToolDelegate : public QStyledItemDelegate
{  
  Q_OBJECT

  public:
    ///
    /// Creates a new PropertyDelegate.
    ///
    QmitkNDIToolDelegate(QObject *parent = 0);

    enum ToolTableColumns
    {
      IndexCol = 0,
      NameCol,
      SROMCol,
      TypeCol,
      StatusCol,
      NodeCol
    };  ///< columns in tool table
    enum NDIItemDataRole
    {
      NDIToolRole  = 70,
      ToolIndexRole,
      FileNameRole,
      OrganNodeRole,
      TypeRole
    }; ///< item data roles for qt model


    void SetTypes(const QStringList& types);               ///< set types list for type editor combobox
    void SetDataStorage(mitk::DataStorage* ds);            ///< set datastorage for organ node editor
    void SetPredicate(mitk::NodePredicateBase::Pointer p); ///< set predicate for organ node editor
    void SetTagPropertyName(const std::string& name);      ///< set name of the property that is used to tag selected nodes
    void SetTagProperty(mitk::BaseProperty::Pointer prop);   ///< set the property that is used to tag selected nodes

    ///
    /// Renders a specific property  (overwritten from QItemDelegate)
    ///
    //void paint(QPainter *painter, const QStyleOptionViewItem &option
    //  , const QModelIndex &index) const;

    ///
    /// Create an editor for a specific property  (overwritten from QItemDelegate)
    ///
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option
      , const QModelIndex &index) const;

    ///
    /// Create an editor for a specific property  (overwritten from QItemDelegate)
    ///
    void setEditorData(QWidget *editor, const QModelIndex &index) const;

    ///
    /// When the user accepts input this func commits the data to the model  (overwritten from QItemDelegate)
    ///
    void setModelData(QWidget *editor, QAbstractItemModel* model
      , const QModelIndex &index) const;

    ///
    /// \brief Fit an editor to some geometry (overwritten from QItemDelegate)
    ///
    //void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;

  private slots:
    ///
    /// Invoked when the user accepts editor input, that is when he does not pushes ESC.
    ///
    void commitAndCloseEditor();
    void ComboBoxCurrentIndexChanged ( int index ) ;


private: 
    QStringList m_Types;
    mitk::DataStorage* m_DataStorage;
    mitk::NodePredicateBase::Pointer m_Predicate;
    mitk::BaseProperty::Pointer m_TagProperty;
    std::string m_TagPropertyName;
};
#endif /* QmitkNDIToolDelegate_h */
