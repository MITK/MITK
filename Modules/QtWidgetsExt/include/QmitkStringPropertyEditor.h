/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef QmitkStringPropertyEditor_h
#define QmitkStringPropertyEditor_h

#include <MitkQtWidgetsExtExports.h>
#include <QLineEdit>
#include <mitkPropertyObserver.h>
#include <mitkStringProperty.h>

/**
 * \brief A QLineEdit that edits a mitk::StringProperty.
 * \ingroup Widgets
 *
 * Bidirectionally synchronizes the line edit text with a mitk::StringProperty.
 * When the user types, the property is updated. When the property changes
 * externally, the text is updated. If the property is removed, "n/a" is displayed.
 *
 * \sa QmitkStringPropertyView, QmitkStringPropertyOnDemandEdit, QmitkPropertyViewFactory
 */
class MITKQTWIDGETSEXT_EXPORT QmitkStringPropertyEditor : public QLineEdit, public mitk::PropertyEditor
{
  Q_OBJECT

public:
  /**
   * \brief Construct an editor for the given string property.
   * \param[in] property The mitk::StringProperty to edit.
   * \param[in] parent The parent widget.
   */
  QmitkStringPropertyEditor(mitk::StringProperty *property, QWidget *parent);

  /** \brief Destructor. */
  ~QmitkStringPropertyEditor() override;

protected:
  void PropertyChanged() override;
  void PropertyRemoved() override;

  mitk::StringProperty *m_StringProperty;

protected slots:

  void onTextChanged(const QString &);

private:
};

#endif
