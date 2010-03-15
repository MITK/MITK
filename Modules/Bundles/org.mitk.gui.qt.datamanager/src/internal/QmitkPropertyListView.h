#ifndef QmitkPropertyListView_h_
#define QmitkPropertyListView_h_

// Own includes

#include "../mitkQtDataManagerDll.h"
#include <mitkDataNodeSelection.h>
#include <QmitkFunctionality.h>
#include <QmitkDataNodeSelectionProvider.h>

class QmitkPropertiesTableEditor;

///
/// A view to show 
///
class MITK_QT_DATAMANAGER QmitkPropertyListView : public QmitkFunctionality
{  
public: 
  berryObjectMacro(QmitkPropertyListView)

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
  /// This is not a standalone functionality (can be easily used with other functionalities open) -> return false
  ///
  virtual bool IsExclusiveFunctionality() const;

  ///
  /// Invoked when the DataManager selection changed
  ///
  virtual void OnSelectionChanged(std::vector<mitk::DataNode*> nodes);

private:
  ///
  /// \brief The properties table editor.
  ///
  QmitkPropertiesTableEditor* m_NodePropertiesTableEditor;

  friend struct berry::SelectionChangedAdapter<QmitkPropertyListView>;
};

#endif /*QmitkPropertyListView_H_*/
