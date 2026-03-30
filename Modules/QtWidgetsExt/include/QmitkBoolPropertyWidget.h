/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef QmitkBoolPropertyWidget_h
#define QmitkBoolPropertyWidget_h

#include <MitkQtWidgetsExtExports.h>
#include <QCheckBox>
#include <mitkProperties.h>

class _BoolPropertyWidgetImpl;

/**
 * \brief A QCheckBox that edits a mitk::BoolProperty.
 * \ingroup Widgets
 *
 * Bidirectionally synchronizes a checkbox with a mitk::BoolProperty. When
 * the property changes externally, the checkbox updates. When the user
 * toggles the checkbox, the property value is updated. If no property is
 * set or the property is removed, the checkbox enters a tristate
 * (indeterminate) disabled state.
 *
 * \sa QmitkPropertyViewFactory
 */
class MITKQTWIDGETSEXT_EXPORT QmitkBoolPropertyWidget : public QCheckBox
{
  Q_OBJECT

public:
  /**
   * \brief Construct the widget without an associated property.
   * \param[in] parent The parent widget.
   */
  QmitkBoolPropertyWidget(QWidget *parent = nullptr);

  /**
   * \brief Construct the widget with a label text.
   * \param[in] text The checkbox label text.
   * \param[in] parent The parent widget.
   */
  QmitkBoolPropertyWidget(const QString &text, QWidget *parent = nullptr);

  /** \brief Destructor. */
  ~QmitkBoolPropertyWidget() override;

  /**
   * \brief Bind this widget to a mitk::BoolProperty.
   * \param[in] property The property to edit. Pass nullptr to clear the binding.
   *
   * When set to nullptr the widget enters a tristate indeterminate disabled state.
   */
  void SetProperty(mitk::BoolProperty *property);

protected slots:

  void onToggle(bool on);

protected:
  _BoolPropertyWidgetImpl *m_PropEditorImpl;
};

#endif
