/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef QmitkBasePropertyView_h
#define QmitkBasePropertyView_h

#include <MitkQtWidgetsExtExports.h>
#include <QLabel>
#include <mitkProperties.h>
#include <mitkPropertyObserver.h>

/**
 * \brief Read-only QLabel that displays any mitk::BaseProperty value as text.
 * \ingroup Widgets
 *
 * Observes a mitk::BaseProperty and updates the label text whenever the
 * property value changes, using BaseProperty::GetValueAsString(). If the
 * property is removed, the label displays "n/a".
 *
 * \sa QmitkPropertyViewFactory, QmitkStringPropertyView, QmitkNumberPropertyView
 */
class MITKQTWIDGETSEXT_EXPORT QmitkBasePropertyView : public QLabel, public mitk::PropertyView
{
  Q_OBJECT

public:
  /**
   * \brief Construct the view for the given property.
   * \param[in] property The mitk::BaseProperty to observe and display.
   * \param[in] parent The parent widget.
   */
  QmitkBasePropertyView(const mitk::BaseProperty *property, QWidget *parent);

  /** \brief Destructor. */
  ~QmitkBasePropertyView() override;

protected:
  /** \brief Called when the observed property value changes. Updates the label text. */
  void PropertyChanged() override;

  /** \brief Called when the observed property is removed. Sets text to "n/a". */
  void PropertyRemoved() override;

private:
};

#endif
