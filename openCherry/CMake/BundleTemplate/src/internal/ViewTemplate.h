@PLUGIN_COPYRIGHT@

#ifndef @UBUNDLE_NAMESPACE@_@UVIEW_CLASS@_H_INCLUDED
#define @UBUNDLE_NAMESPACE@_@UVIEW_CLASS@_H_INCLUDED

#include <@VIEW_BASE_CLASS_H@>
#include <string>

class QWidget;
class QPushButton;

@BEGIN_NAMESPACE@

/*!
 * \ingroup @NormalizedPluginID@_internal
 *
 * \brief @PLUGIN_NAME@
 *
 * You need to reimplement the methods SetFocus() and CreateQtPartControl(QWidget*)
 * from @VIEW_BASE_CLASS@
 *
 * \sa @VIEW_BASE_CLASS@
 */
class @VIEW_CLASS@ : public @VIEW_BASE_CLASS@
{
public:

  static const std::string VIEW_ID;

  /*!
   * \brief Gives focus to a specific control in the view
   * This method is called from the framework when the view is activated.
   */
  void SetFocus();

protected:

  /*!
   * \brief Builds the user interface of the view
   * This method is called from the framework. The parent widget has no layout, so
   * you should set one adapted to your needs.
   */
  void CreateQtPartControl(QWidget* parent);

  QPushButton* m_ButtonStart;
};

@END_NAMESPACE@

#endif /*@UBUNDLE_NAMESPACE@_@UVIEW_CLASS@_H_INCLUDED*/
