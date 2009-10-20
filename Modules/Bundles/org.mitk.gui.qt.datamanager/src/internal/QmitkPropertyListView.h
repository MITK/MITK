#ifndef QmitkPropertyListView_h_
#define QmitkPropertyListView_h_

// Own includes
#include <cherryISelectionListener.h>
#include "../mitkQtDataManagerDll.h"
#include <mitkDataTreeNodeSelection.h>
#include <QmitkFunctionality.h>
#include <QmitkDataTreeNodeSelectionProvider.h>

class QmitkPropertiesTableEditor;

///
/// A view to show 
///
class MITK_QT_DATAMANAGER QmitkPropertyListView : public QmitkFunctionality, virtual public cherry::ISelectionListener
{  
public: 
  cherryObjectMacro(QmitkPropertyListView)

  ///
  /// The unique ID of this view
  ///
  static const std::string VIEW_ID;
  ///
  /// \brief Standard ctor.
  ///
  QmitkPropertyListView();
  ///
  /// \brief Standard dtor.
  ///
  virtual ~QmitkPropertyListView();
  ///
  /// \brief Create the view here.
  ///
  void CreateQtPartControl(QWidget* parent);

  ///
  /// Invoked when the DataManager selection changed
  ///
  virtual void SelectionChanged(cherry::IWorkbenchPart::Pointer part
    , cherry::ISelection::ConstPointer selection);

private:
  ///
  /// \brief The properties table editor.
  ///
  QmitkPropertiesTableEditor* m_NodePropertiesTableEditor;
  ///
  /// A selection listener for datatreenode events
  ///
  cherry::ISelectionListener::Pointer m_SelectionListener;
  ///
  /// 
  ///
  friend struct cherry::SelectionChangedAdapter<QmitkPropertyListView>;
};

#endif /*QmitkPropertyListView_H_*/
