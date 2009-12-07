#ifndef QmitkNDIToolDelegate_h
#define QmitkNDIToolDelegate_h

/// Own includes.
#include "MitkIGTUIExports.h"
#include "mitkNodePredicateBase.h"

/// Toolkit includes.
#include <QStyledItemDelegate>

/// Forward declarations.
namespace mitk
{
  class DataStorage;
  class NodePredicateBase;
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

    void SetTypes(const QStringList& types);       ///< set types list for type editor combobox
    void SetDataStorage(mitk::DataStorage* ds);    ///< set datastorage for organ node editor
    void SetPredicate(mitk::NodePredicateBase::Pointer p); ///< set predicate for organ node editor

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
};
#endif /* QmitkNDIToolDelegate_h */
