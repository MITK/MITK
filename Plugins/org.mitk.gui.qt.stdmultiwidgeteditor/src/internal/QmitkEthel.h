/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkEthel_h
#define QmitkEthel_h

#include <mitkDataNode.h>

#include <QObject>

class QmitkAbstractMultiWidget;
class QMenu;

/**
 * \brief Rewards persistent lighting menu users with Ethel, a textured cat.
 *
 * Counts how often the lighting menu of the 3D render window is opened. On
 * the tenth opening, once per session, the menu gets an extra entry for
 * that one opening. Choosing it adds the cat from the plugin resources to
 * the data storage, visible in the 3D windows only, and fits the views to
 * the scene. After a few calm seconds she starts to spin, bounce and cycle
 * her colors.
 */
class QmitkEthel : public QObject
{
  Q_OBJECT

public:
  explicit QmitkEthel(QmitkAbstractMultiWidget* multiWidget);
  ~QmitkEthel() override;

private:
  void OnLightingMenuAboutToShow(QMenu* menu);
  void OnEthelTriggered();
  void Show();

  QmitkAbstractMultiWidget* m_MultiWidget;
  mitk::DataNode::Pointer m_EthelNode;
};

#endif
