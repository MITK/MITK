/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef QmitkStringPropertyOnDemandEdit_h
#define QmitkStringPropertyOnDemandEdit_h

#include <MitkQtWidgetsExtExports.h>
#include <QLabel>
#include <QLayout>
#include <mitkPropertyObserver.h>
#include <mitkStringProperty.h>

/**
 * \brief A simple clickable QLabel helper used by QmitkStringPropertyOnDemandEdit.
 *
 * Emits a clicked() signal on mouse release.
 */
class MITKQTWIDGETSEXT_EXPORT QClickableLabel2 : public QLabel
{
  Q_OBJECT

signals:
  /** \brief Emitted when the label is clicked. */
  void clicked();

public:
  /**
   * \brief Construct the clickable label.
   * \param[in] parent The parent widget.
   * \param[in] f Window flags.
   */
  QClickableLabel2(QWidget *parent, Qt::WindowFlags f = {}) : QLabel(parent, f) {}

  /** \brief Emit clicked() on mouse release. */
  void mouseReleaseEvent(QMouseEvent *) override { emit clicked(); }
};

/**
 * \brief An on-demand string property editor that shows the value as a label
 *        with a "..." button to open an input dialog.
 * \ingroup Widgets
 *
 * Displays the current string property value as a QLabel. Clicking the "..."
 * button opens a QInputDialog to edit the text. The property is updated only
 * when the dialog is accepted. If the property is removed, "n/a" is shown.
 *
 * \sa QmitkStringPropertyEditor, QmitkStringPropertyView, QmitkPropertyViewFactory
 */
class MITKQTWIDGETSEXT_EXPORT QmitkStringPropertyOnDemandEdit : public QFrame, public mitk::PropertyEditor
{
  Q_OBJECT

public:
  /**
   * \brief Construct an on-demand editor for the given string property.
   * \param[in] property The mitk::StringProperty to edit.
   * \param[in] parent The parent widget.
   */
  QmitkStringPropertyOnDemandEdit(mitk::StringProperty *property, QWidget *parent);

  /** \brief Destructor. */
  ~QmitkStringPropertyOnDemandEdit() override;

protected:
  void PropertyChanged() override;
  void PropertyRemoved() override;

  mitk::StringProperty *m_StringProperty;

  QHBoxLayout *m_layout;
  QLabel *m_label;
  QClickableLabel2 *m_toolbutton;

protected slots:

  void onToolButtonClicked();
};

#endif
