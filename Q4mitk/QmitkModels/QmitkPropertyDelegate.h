#ifndef QmitkPropertyDelegate_h
#define QmitkPropertyDelegate_h

/// Own includes.
#include "mitkBaseProperty.h"

/// Toolkit includes.
#include <QItemDelegate>

/// Forward declarations.

/// 
/// \class QmitkPropertyDelegate
/// \brief An item delegate for rendering and editing mitk::Properties in a QTableView.
///
/// \see QmitkPropertiesTableModel
class QMITK_EXPORT QmitkPropertyDelegate : public QItemDelegate
{  
  Q_OBJECT

  public:
    ///
    /// Creates a new PropertyDelegate.
    ///
    QmitkPropertyDelegate(QObject *parent = 0);

    ///
    /// Renders a specific property.
    ///
    void paint(QPainter *painter, const QStyleOptionViewItem &option
      , const QModelIndex &index) const;

    ///
    /// Create an editor for a specific property.
    ///
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option
      , const QModelIndex &index) const;

    ///
    /// Create an editor for a specific property.
    ///
    void setEditorData(QWidget *editor, const QModelIndex &index) const;

    ///
    /// When the user accepts input this func commits the data to the model.
    ///
    void setModelData(QWidget *editor, QAbstractItemModel* model
      , const QModelIndex &index) const;

    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;

  private slots:
    ///
    /// Invoked when the user accepts editor input, that is when he does not pushes ESC.
    ///
    void commitAndCloseEditor();
    void ComboBoxCurrentIndexChanged ( int index ) ;
    void SpinBoxValueChanged ( const QString& value ) ;

};

#endif /* QMITKPROPERTIESTABLEMODEL_H_ */
