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
  virtual void NodeAdded(const mitk::DataNode* node) override;

  ///
  /// remove the node
  ///
  virtual void NodeRemoved(const mitk::DataNode* node) override;

  /**
   * Called when a DataStorage Changed event was thrown. May be reimplemented
   * by deriving classes.
   */
  virtual void NodeChanged(const mitk::DataNode* node) override;

protected slots:
  void on_DataItemList_currentRowChanged ( int currentRow );
  void on_Remove_pressed();
  void on_Load_pressed();

protected:

  ///
  /// \brief Create the view here.
  ///
  virtual void CreateQtPartControl(QWidget* parent) override;

  ///
  /// evaluate the new label string
  ///
  void ListSelectionChanged();
  void FileOpen(const char *fileName, mitk::DataNode *parentNode);
  void RemoveNode(mitk::DataNode *node);
  void GlobalReinit();
  void ToggleVisibility();

  ///
  /// focus on load image
  ///
  void SetFocus() override;

private:
  QmitkDataManagerLightViewData* d;
};

#endif /*QmitkDataManagerLightView_H_*/
