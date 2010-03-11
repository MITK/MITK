@PLUGIN_COPYRIGHT@

#ifndef @VIEW_CLASS@_h
#define @VIEW_CLASS@_h

#include <berryISelectionListener.h>

#include <@VIEW_BASE_CLASS_H@>

#include "@VIEW_CONTROLS_FILE@"

@BEGIN_NAMESPACE@

/*!
  \brief @VIEW_CLASS@ 

  \warning  This application module is not yet documented. Use "svn blame/praise/annotate" and ask the author to provide basic documentation.

  \sa QmitkFunctionality
  \ingroup Functionalities
*/
class @VIEW_CLASS@ : public QObject, public QmitkFunctionality
{  
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT
  
  public:  

    static const std::string VIEW_ID;

    @VIEW_CLASS@();
    ~@VIEW_CLASS@();

    virtual void CreateQtPartControl(QWidget *parent);

    virtual void StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget);
    virtual void StdMultiWidgetNotAvailable();

  protected slots:
  
    /// \brief Called when the user clicks the GUI button
    void DoImageProcessing();

  protected:

    /// \brief called by QmitkFunctionality when DataManager's selection has changed
    virtual void OnSelectionChanged( std::vector<mitk::DataTreeNode*> nodes );

    Ui::@VIEW_CONTROLS_CLASS@* m_Controls;

    QmitkStdMultiWidget* m_MultiWidget;
};

@END_NAMESPACE@

#endif // @UBUNDLE_NAMESPACE@_@UVIEW_CLASS@_H_INCLUDED

