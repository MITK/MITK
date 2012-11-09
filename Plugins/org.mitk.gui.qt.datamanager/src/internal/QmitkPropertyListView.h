/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

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
