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
