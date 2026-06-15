/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkEnumerationPropertyWidget_h
#define QmitkEnumerationPropertyWidget_h

#include <MitkQtWidgetsExtExports.h>

#include <QComboBox>
#include <QHash>

namespace mitk
{
  class EnumerationProperty;
}

class _EnumPropEditorImpl;

/**
 * \brief A QComboBox that edits a mitk::EnumerationProperty.
 *
 * Populates the combo box with the enumeration strings from the property
 * and bidirectionally synchronizes selection changes. When the property
 * value changes externally, the combo box selection updates. When the
 * user selects a different item, the property value is updated.
 *
 * \sa QmitkPropertyViewFactory, mitk::EnumerationProperty
 */
class MITKQTWIDGETSEXT_EXPORT QmitkEnumerationPropertyWidget : public QComboBox
{
  Q_OBJECT

public:
  /**
   * \brief Construct the widget.
   * \param[in] parent The parent widget.
   */
  QmitkEnumerationPropertyWidget(QWidget *parent = nullptr);

  /** \brief Destructor. */
  ~QmitkEnumerationPropertyWidget() override;

  /**
   * \brief Bind this widget to a mitk::EnumerationProperty.
   *
   * Populates the combo box with all enum strings from the property.
   * Pass nullptr to clear the binding and the combo box.
   *
   * \param[in] property The enumeration property to edit.
   */
  void SetProperty(mitk::EnumerationProperty *property);

protected slots:

  void OnIndexChanged(int index);

protected:
  _EnumPropEditorImpl *propView;
};

#endif
