@PLUGIN_COPYRIGHT@

#ifndef @UBUNDLE_NAMESPACE@_@UVIEW_CLASS@_H_INCLUDED
#define @UBUNDLE_NAMESPACE@_@UVIEW_CLASS@_H_INCLUDED

@INCLUDE_DLL_H@
#include <@VIEW_BASE_CLASS_H@>

class QWidget;
class QPushButton;

@BEGIN_NAMESPACE@

class @DLL_DEFINE@ @VIEW_CLASS@ : public @VIEW_BASE_CLASS@
{
public:
  void SetFocus();

protected:
  void CreateQtPartControl(QWidget* parent);

  QPushButton* m_ButtonStart;
};

@END_NAMESPACE@

#endif /*@UBUNDLE_NAMESPACE@_@UVIEW_CLASS@_H_INCLUDED*/
