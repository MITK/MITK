$(license)

#ifndef $(view-file-name)_h
#define $(view-file-name)_h

#include <berryISelectionListener.h>

#include <QmitkAbstractView.h>

#include "ui_$(view-file-name)Controls.h"

/**
  \brief $(view-class-name)

  \warning  This class is not yet documented. Use "git blame" and ask the author to provide basic documentation.

  \sa QmitkAbstractView
  \ingroup ${plugin_target}_internal
*/
class $(view-class-name) : public QmitkAbstractView
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:
  static const std::string VIEW_ID;

protected:
  virtual void CreateQtPartControl(QWidget *parent) override;

  virtual void SetFocus() override;

  /// \brief called by QmitkFunctionality when DataManager's selection has changed
  virtual void OnSelectionChanged(berry::IWorkbenchPart::Pointer source,
                                  const QList<mitk::DataNode::Pointer> &nodes) override;

  /// \brief Called when the user clicks the GUI button
  void DoImageProcessing();

  Ui::$(view-file-name)Controls m_Controls;
};

#endif // $(view-file-name)_h
