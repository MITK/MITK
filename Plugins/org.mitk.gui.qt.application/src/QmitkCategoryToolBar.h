/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkCategoryToolBar_h
#define QmitkCategoryToolBar_h

#include <org_mitk_gui_qt_application_Export.h>

#include <QPointer>
#include <QToolBar>

class QLabel;
class QToolButton;

/**
 * \brief Tool bar for the views of one category that can name its category.
 *
 * The category name is either painted above the buttons, shown in a floating
 * label while the mouse rests on any category tool bar of the same window,
 * or not shown at all. Vertical tool bars, e.g. docked to a side of the
 * window, never show it. Clicking the name above the buttons expands a tool
 * bar whose buttons do not all fit.
 *
 * Buttons are aligned to the bottom edge. Tool bars that share a row are
 * stretched to the height of the tallest one, so this keeps the buttons of all
 * tool bars in a row on one line, whether there is a category name above them
 * or not. Tool bars that do not belong to a category, like the main actions
 * tool bar, use this class with an empty category for that reason alone.
 */
class MITK_QT_APP QmitkCategoryToolBar : public QToolBar
{
  Q_OBJECT

public:
  enum class CategoryLabel
  {
    Hidden,
    AboveButtons,
    OnHover
  };

  explicit QmitkCategoryToolBar(const QString& category, QWidget* parent = nullptr);
  ~QmitkCategoryToolBar() override;

  QString GetCategory() const;

  CategoryLabel GetCategoryLabel() const;
  void SetCategoryLabel(CategoryLabel categoryLabel);

  QSize sizeHint() const override;
  QSize minimumSizeHint() const override;

protected:
  void actionEvent(QActionEvent* event) override;
  void changeEvent(QEvent* event) override;
  void enterEvent(QEnterEvent* event) override;
  void leaveEvent(QEvent* event) override;
  void hideEvent(QHideEvent* event) override;
  void moveEvent(QMoveEvent* event) override;
  void resizeEvent(QResizeEvent* event) override;
  void paintEvent(QPaintEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override;
  bool eventFilter(QObject* watched, QEvent* event) override;

private:
  bool ReservesCaptionSpace() const;
  bool IsExpanded() const;
  QFont GetCaptionFont() const;
  QRect GetCaptionRect() const;
  int GetButtonsLeft() const;
  int GetButtonsRight() const;
  void AlignExtensionButton();

  QList<QmitkCategoryToolBar*> GetToolBarsOfSameWindow() const;
  void ShowHoverLabels();
  void UpdateShownHoverLabels();
  void HideHoverLabel();
  void HideHoverLabelsUnlessHovered();

  QString m_Category;
  CategoryLabel m_CategoryLabel;
  QToolButton* m_ExtensionButton;
  QPointer<QLabel> m_HoverLabel;
};

#endif
