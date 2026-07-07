/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef QmitkColorPropertyView_h
#define QmitkColorPropertyView_h

#include <MitkQtWidgetsExtExports.h>
#include <QLabel>
#include <mitkColorProperty.h>
#include <mitkPropertyObserver.h>

/**
 * \brief Read-only QLabel that displays a mitk::ColorProperty as a colored swatch.
 *
 * Observes a mitk::ColorProperty and updates its background color whenever
 * the property changes. The widget auto-fills its background to show the color.
 *
 * \sa QmitkColorPropertyEditor, QmitkPropertyViewFactory
 */
class MITKQTWIDGETSEXT_EXPORT QmitkColorPropertyView : public QLabel, public mitk::PropertyView
{
  Q_OBJECT

public:
  /**
   * \brief Construct a color view for the given property.
   * \param[in] property The mitk::ColorProperty to observe and display.
   * \param[in] parent The parent widget.
   */
  QmitkColorPropertyView(const mitk::ColorProperty *property, QWidget *parent);

  /** \brief Destructor. */
  ~QmitkColorPropertyView() override;

protected:
  void PropertyChanged() override;
  void PropertyRemoved() override;

  void DisplayColor();

  const mitk::ColorProperty *m_ColorProperty;

  QPalette m_WidgetPalette;
};

#endif
