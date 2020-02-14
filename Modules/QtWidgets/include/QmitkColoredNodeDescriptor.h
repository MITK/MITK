/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkColoredNodeDescriptor_h
#define QmitkColoredNodeDescriptor_h

#include <QmitkNodeDescriptor.h>
#include <MitkQtWidgetsExports.h>

/**
 * \ingroup QmitkModule
 * \brief Synchronizes the magic color #00ff00 of the given SVG icon to the mitk::DataNode color.
 *
 * \sa QmitkNodeDescriptorManager
 */
class MITKQTWIDGETS_EXPORT QmitkColoredNodeDescriptor final : public QmitkNodeDescriptor
{
  Q_OBJECT

public:
  explicit QmitkColoredNodeDescriptor(const QString &className, const QString &pathToIcon, mitk::NodePredicateBase *predicate, QObject *parent = nullptr);
  ~QmitkColoredNodeDescriptor() override;

  QIcon GetIcon(const mitk::DataNode *node) const override;

private:
  struct Impl;
  Impl *m_Impl;
};

#endif
