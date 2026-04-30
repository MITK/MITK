/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef QmitkStringPropertyView_h
#define QmitkStringPropertyView_h

#include <MitkQtWidgetsExtExports.h>
#include <QLabel>
#include <mitkPropertyObserver.h>
#include <mitkStringProperty.h>

/**
 * \brief Read-only QLabel that displays a mitk::StringProperty value as text.
 * \ingroup Widgets
 *
 * Observes a mitk::StringProperty and updates the label text whenever the
 * property value changes. If the property is removed, "n/a" is displayed.
 *
 * \sa QmitkStringPropertyEditor, QmitkStringPropertyOnDemandEdit, QmitkPropertyViewFactory
 */
class MITKQTWIDGETSEXT_EXPORT QmitkStringPropertyView : public QLabel, public mitk::PropertyView
{
  Q_OBJECT

public:
  /**
   * \brief Construct a view for the given string property.
   * \param[in] property The mitk::StringProperty to observe.
   * \param[in] parent The parent widget.
   */
  QmitkStringPropertyView(const mitk::StringProperty *property, QWidget *parent);

  /** \brief Destructor. */
  ~QmitkStringPropertyView() override;

protected:
  void PropertyChanged() override;
  void PropertyRemoved() override;

  const mitk::StringProperty *m_StringProperty;
};

#endif
