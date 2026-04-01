/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkPropertyItemDelegate_h
#define QmitkPropertyItemDelegate_h

#include <MitkQtWidgetsExports.h>
#include <QListView>
#include <QStyledItemDelegate>
#include <mitkPropertyList.h>
#include <mitkWeakPointer.h>

class QComboBox;
class QLineEdit;
class QToolButton;
class QResizeEvent;

/**
 * \brief A small widget combining a QLineEdit and a QToolButton for color editing.
 *
 * Displays a hex color string in the line edit and a colored button. Clicking the
 * button opens a QColorDialog. Used internally by QmitkPropertyItemDelegate.
 *
 * \sa QmitkPropertyItemDelegate
 */
class QmitkColorWidget : public QWidget
{
  Q_OBJECT

public:
  /**
   * \brief Constructs the color widget.
   * \param[in] parent Optional parent widget.
   */
  explicit QmitkColorWidget(QWidget *parent = nullptr);
  ~QmitkColorWidget() override;

  /**
   * \brief Returns the currently selected color.
   * \return The current QColor.
   */
  QColor GetColor() const;

  /**
   * \brief Sets the displayed color.
   * \param[in] color The color to set.
   */
  void SetColor(QColor color);

signals:
  /** \brief Emitted when a new color has been picked via the dialog or line edit. */
  void ColorPicked();

private slots:
  void OnButtonClicked();
  void OnLineEditEditingFinished();

private:
  QColor m_Color;
  QLineEdit *m_LineEdit;
  QToolButton *m_Button;
};

/**
 * \brief Custom QListView used as the popup view for combo boxes in property editing.
 *
 * Handles painting and resizing to work correctly within a QComboBox popup.
 *
 * \sa QmitkPropertyItemDelegate
 */
class QmitkComboBoxListView : public QListView
{
  Q_OBJECT

public:
  /**
   * \brief Constructs the list view for the given combo box.
   * \param[in] comboBox The parent combo box. May be nullptr.
   */
  explicit QmitkComboBoxListView(QComboBox *comboBox = nullptr);
  ~QmitkComboBoxListView() override;

protected:
  void paintEvent(QPaintEvent *event) override;
  void resizeEvent(QResizeEvent *event) override;
  void initViewItemOption(QStyleOptionViewItem *option) const override;

private:
  QComboBox *m_ComboBox;
};

/**
 * \brief Item delegate for editing MITK properties in a QTreeView or QTableView.
 *
 * Creates type-appropriate editor widgets (spin boxes, check boxes, combo boxes,
 * color pickers, line edits) based on the property type. Works with
 * QmitkPropertyItemModel.
 *
 * \sa QmitkPropertyItemModel
 * \sa QmitkPropertyItem
 * \sa QmitkPropertiesTableEditor
 */
class MITKQTWIDGETS_EXPORT QmitkPropertyItemDelegate : public QStyledItemDelegate
{
  Q_OBJECT

public:
  /**
   * \brief Constructs the delegate.
   * \param[in] parent Optional parent QObject.
   */
  explicit QmitkPropertyItemDelegate(QObject *parent = nullptr);
  ~QmitkPropertyItemDelegate() override;

  /**
   * \brief Creates a type-appropriate editor widget for the given model index.
   * \param[in] parent The parent widget for the editor.
   * \param[in] option Style options for the item.
   * \param[in] index  The model index to create an editor for.
   * \return The created editor widget.
   */
  QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

  /**
   * \brief Paints the item, with special handling for color properties.
   * \param[in] painter The painter to use.
   * \param[in] option  Style options for the item.
   * \param[in] index   The model index to paint.
   */
  void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

  /**
   * \brief Sets the editor widget data from the model.
   * \param[in] editor The editor widget.
   * \param[in] index  The model index providing the data.
   */
  void setEditorData(QWidget *editor, const QModelIndex &index) const override;

  /**
   * \brief Writes the editor widget data back to the model.
   * \param[in] editor The editor widget.
   * \param[in] model  The model to write to.
   * \param[in] index  The model index to update.
   */
  void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;

  /**
   * \brief Sets the property list whose properties are being edited.
   * \param[in] propertyList The property list to associate with this delegate.
   */
  void SetPropertyList(mitk::PropertyList *propertyList);

private slots:
  void OnComboBoxCurrentIndexChanged(int index);
  void OnSpinBoxEditingFinished();
  void OnColorPicked();

private:
  std::string GetPropertyName(const QModelIndex &index) const;

  mitk::WeakPointer<mitk::PropertyList> m_PropertyList;
};

#endif
