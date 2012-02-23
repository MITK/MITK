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

#include <QmitkAbstractView.h>

#include <mitkDataNodeSelection.h>
#include <QmitkDataNodeSelectionProvider.h>

class QmitkPropertiesTableEditor;

///
/// A view to show 
///
class QmitkPropertyListView : public QmitkAbstractView
{  
  Q_OBJECT

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
  /// Invoked when the DataManager selection changed
  ///
  virtual void OnSelectionChanged(berry::IWorkbenchPart::Pointer part, const QList<mitk::DataNode::Pointer>& nodes);

protected:

  void SetFocus();

private:
  ///
  /// \brief The properties table editor.
  ///
  QmitkPropertiesTableEditor* m_NodePropertiesTableEditor;

  friend struct berry::SelectionChangedAdapter<QmitkPropertyListView>;
};

#endif /*QmitkPropertyListView_H_*/
