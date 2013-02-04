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

#ifndef QmitkDataManagerLightView_H_
#define QmitkDataManagerLightView_H_

/// Qmitk
#include <QmitkAbstractView.h>

struct QmitkDataManagerLightViewData;

///
/// \brief Data management view with reduced functions ("light")
///
class QmitkDataManagerLightView : public QmitkAbstractView
{
  Q_OBJECT

public:

  static const std::string VIEW_ID; // = "org.mitk.extapp.defaultperspective"
  ///
  /// \brief Standard ctor.
  ///
  QmitkDataManagerLightView();

  ///
  /// \brief Standard dtor.
  ///
  virtual ~QmitkDataManagerLightView();

  ///
  /// add the node to the list ...
  ///
  virtual void NodeAdded(const mitk::DataNode* node);

  ///
  /// remove the node
  ///
  virtual void NodeRemoved(const mitk::DataNode* node);

protected slots:
  void on_DataItemList_currentRowChanged ( int currentRow );

protected:

  ///
  /// \brief Create the view here.
  ///
  virtual void CreateQtPartControl(QWidget* parent);

  ///
  /// focus on load image
  ///
  void SetFocus();

private:
  QmitkDataManagerLightViewData* d;
};

#endif /*QmitkDataManagerLightView_H_*/
