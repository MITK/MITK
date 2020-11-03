/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
  ~QmitkDataManagerLightView() override;

  ///
  /// add the node to the list ...
  ///
  void NodeAdded(const mitk::DataNode* node) override;

  ///
  /// remove the node
  ///
  void NodeRemoved(const mitk::DataNode* node) override;

  /**
   * Called when a DataStorage Changed event was thrown. May be reimplemented
   * by deriving classes.
   */
  void NodeChanged(const mitk::DataNode* node) override;

protected slots:
  void on_DataItemList_currentRowChanged ( int currentRow );
  void on_Remove_pressed();
  void on_Load_pressed();

protected:

  ///
  /// \brief Create the view here.
  ///
  void CreateQtPartControl(QWidget* parent) override;

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
