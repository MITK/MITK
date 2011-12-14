#ifndef mitkIContextMenuAction_H_
#define mitkIContextMenuAction_H_

#include <berryMacros.h>
#include <berryQtViewPart.h>

#include <vector>
#include <mitkDataNode.h>
#include "mitkDataStorage.h"

class QmitkStdMultiWidget;

namespace mitk
{
  /**
  * A context menu action, which is linked to the context menu <br>
  * through an extension point. For an example check the <br>
  * <code> plugin.xml </code> and the connected classes of <br>
  * the the segmentation bundle and also the <code> QmitkDataManagerView.cpp </code> <br>
  * in this bundle.
  */
  struct IContextMenuAction
  {   
    berryInterfaceMacro(IContextMenuAction, mitk)

      /**
      * @brief Executes the action, that linked to the context menu entry.
      */
      virtual void Run( const std::vector<mitk::DataNode*>& selectedNodes ) = 0;

    // Setters
    virtual void SetDataStorage(mitk::DataStorage* dataStorage) = 0;
    virtual void SetStdMultiWidget(QmitkStdMultiWidget *stdMultiWidget) = 0;
    virtual void SetSmoothed(bool smoothed) = 0;
    virtual void SetDecimated(bool decimated) = 0;
    virtual void SetFunctionality(berry::QtViewPart* functionality) = 0;
  };
}

Q_DECLARE_INTERFACE(mitk::IContextMenuAction, "org.mitk.datamanager.IContextMenuAction")

#endif // mitkIContextMenuAction_H_ 
