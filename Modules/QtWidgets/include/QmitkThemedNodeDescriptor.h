/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkThemedNodeDescriptor_h
#define QmitkThemedNodeDescriptor_h

#include <QmitkNodeDescriptor.h>

/**
 * \ingroup QmitkModule
 * \brief Node descriptor whose icon follows the application style sheet.
 *
 * The icon is themed on demand rather than on construction. The workbench
 * applies its style sheet only after the plug-ins have been started, so a
 * descriptor that themes its icon in the constructor and is created before
 * that freezes the placeholder colors of the untouched SVG.
 *
 * \sa QmitkStyleManager
 * \sa QmitkNodeDescriptor
 * \sa QmitkNodeDescriptorManager
 */
class MITKQTWIDGETS_EXPORT QmitkThemedNodeDescriptor final : public QmitkNodeDescriptor
{
  Q_OBJECT

public:
  /**
   * \brief Constructs a themed node descriptor.
   *
   * \param[in] className   The class name this descriptor is associated with.
   * \param[in] pathToIcon  Path to an SVG file containing the magic colors understood by QmitkStyleManager.
   * \param[in] predicate   Node predicate that determines which nodes this descriptor matches.
   * \param[in] parent      Optional parent QObject.
   */
  explicit QmitkThemedNodeDescriptor(const QString &className, const QString &pathToIcon, mitk::NodePredicateBase *predicate, QObject *parent = nullptr);
  ~QmitkThemedNodeDescriptor() override;

  /**
   * \brief Returns the icon themed for the current style sheet.
   *
   * The icon is cached and re-themed only when the style sheet changed, so it
   * follows a theme switch at runtime.
   */
  QIcon GetIcon(const mitk::DataNode *node) const override;

private:
  struct Impl;
  Impl *m_Impl;
};

#endif
