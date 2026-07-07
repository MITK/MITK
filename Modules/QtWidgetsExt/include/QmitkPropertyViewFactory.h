/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkPropertyViewFactory_h
#define QmitkPropertyViewFactory_h

#include <MitkQtWidgetsExtExports.h>
#include <mitkProperties.h>

class QWidget;

/**
 * \brief Singleton factory for creating property view and editor widgets.
 *
 * Given a mitk::BaseProperty, this factory creates the appropriate Qt widget
 * to either display (view) or edit the property value. Supports string, color,
 * bool, int, float, double, and enumeration properties.
 *
 * \note Created widgets are owned by the caller and must be deleted properly.
 *
 * \sa QmitkBasePropertyView, QmitkStringPropertyView, QmitkColorPropertyView,
 *     QmitkNumberPropertyView, QmitkBoolPropertyWidget, QmitkStringPropertyEditor,
 *     QmitkColorPropertyEditor, QmitkNumberPropertyEditor
 */
class MITKQTWIDGETSEXT_EXPORT QmitkPropertyViewFactory
{
public:
  /** \brief View type identifiers. */
  enum ViewTypes
  {
    vtDEFAULT = 0 ///< Default view type.
  };

  /** \brief Editor type identifiers. */
  enum EditorTypes
  {
    etDEFAULT = 0,       ///< Default (always editable) editor.
    etALWAYS_EDIT = 1,   ///< Always-edit mode editor.
    etON_DEMAND_EDIT = 2  ///< On-demand (click to edit) editor.
  };

  /**
   * \brief Get the singleton instance.
   * \return Pointer to the factory instance.
   */
  static QmitkPropertyViewFactory *GetInstance();

  /**
   * \brief Create a read-only view widget for the given property.
   *
   * The caller owns the returned widget and must delete it.
   *
   * \param[in] property The property to display.
   * \param[in] type The view type (default: vtDEFAULT).
   * \param[in] parent The parent widget.
   * \return A new QWidget displaying the property, or nullptr if unsupported.
   */
  QWidget *CreateView(const mitk::BaseProperty *property, unsigned int type = 0, QWidget *parent = nullptr);

  /**
   * \brief Create an editor widget for the given property.
   *
   * The caller owns the returned widget and must delete it.
   *
   * \param[in] property The property to edit.
   * \param[in] type The editor type (default: etDEFAULT).
   * \param[in] parent The parent widget.
   * \return A new QWidget for editing the property, or nullptr if unsupported.
   */
  QWidget *CreateEditor(mitk::BaseProperty *property, unsigned int type = 0, QWidget *parent = nullptr);

protected:
  /** \brief Hidden constructor; use GetInstance(). */
  QmitkPropertyViewFactory();
  /** \brief Destructor. */
  ~QmitkPropertyViewFactory();
};

#endif
