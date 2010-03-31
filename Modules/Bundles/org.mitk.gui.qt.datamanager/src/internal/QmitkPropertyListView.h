/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-07-14 19:11:20 +0200 (Tue, 14 Jul 2009) $
Version:   $Revision: 18127 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

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
