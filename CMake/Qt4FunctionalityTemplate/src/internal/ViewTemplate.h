@PLUGIN_COPYRIGHT@

#ifndef @UBUNDLE_NAMESPACE@_@UVIEW_CLASS@_H_INCLUDED
#define @UBUNDLE_NAMESPACE@_@UVIEW_CLASS@_H_INCLUDED

#include <@VIEW_BASE_CLASS_H@>

#include <string>

#include "@VIEW_CONTROLS_FILE@"

@BEGIN_NAMESPACE@

/*!

  \brief QmitkTemplate 

  Functionality for demonstration of MITK basics.

  This functionality allows the user to set some seed points that are used for a simple region growing algorithm from ITK.

  \sa QmitkFunctionality
  \ingroup Functionalities
*/
class @VIEW_CLASS@ : public QObject, public @VIEW_BASE_CLASS@
{  

  // this is needed for all Qt objects that should have a MOC object (everything that derives from QObject)
  Q_OBJECT
  
  public: 

  static const std::string VIEW_ID;

  @VIEW_CLASS@();
  virtual ~@VIEW_CLASS@();

  virtual void CreateQtPartControl(QWidget *parent);

  /// \brief Creation of the connections of main and control widget  
  virtual void CreateConnections();

  /// \brief Called when the functionality is activated
  virtual void Activated();
  
  virtual void Deactivated();

  virtual void StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget);
  virtual void StdMultiWidgetNotAvailable();

protected slots:  
  
  /// \brief Called when the user clicks the GUI button
  void DoSomething();

protected:  
  
  Ui::@VIEW_CONTROLS_CLASS@* m_Controls;

  QmitkStdMultiWidget* m_MultiWidget;
};


@END_NAMESPACE@

#endif // @UBUNDLE_NAMESPACE@_@UVIEW_CLASS@_H_INCLUDED

