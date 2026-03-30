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
 * \brief Node descriptor that recolors an SVG icon to match the DataNode color.
 *
 * This class extends QmitkNodeDescriptor by replacing the magic color \c \#00ff00
 * in the provided SVG icon template with the actual color of the mitk::DataNode.
 * Recolored icons are cached internally so that each unique color is only rendered once.
 *
 * \sa QmitkNodeDescriptor
 * \sa QmitkNodeDescriptorManager
 */
class MITKQTWIDGETS_EXPORT QmitkColoredNodeDescriptor final : public QmitkNodeDescriptor
{
  Q_OBJECT

public:
  /**
   * \brief Constructs a colored node descriptor.
   *
   * \param[in] className   The class name this descriptor is associated with.
   * \param[in] pathToIcon  Path to an SVG file containing the magic color \c \#00ff00.
   * \param[in] predicate   Node predicate that determines which nodes this descriptor matches.
   * \param[in] parent      Optional parent QObject.
   */
  explicit QmitkColoredNodeDescriptor(const QString &className, const QString &pathToIcon, mitk::NodePredicateBase *predicate, QObject *parent = nullptr);
  ~QmitkColoredNodeDescriptor() override;

  /**
   * \brief Returns the icon with the magic color replaced by the node's color.
   *
   * The node's RGB color property is read and converted to a hex color code.
   * The resulting icon is cached for future requests with the same color.
   *
   * \param[in] node The data node whose color is used for the icon.
   * \return The recolored QIcon, or an empty icon if \p node is nullptr.
   */
  QIcon GetIcon(const mitk::DataNode *node) const override;

private:
  struct Impl;
  Impl *m_Impl;
};

#endif
