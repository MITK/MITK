/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkNodePredicateDimension_h
#define mitkNodePredicateDimension_h

#include <mitkNodePredicateBase.h>

namespace mitk
{
  class BaseData;

  /** \brief Predicate that evaluates if the given DataNode's data object
   * has the specified dimension, for datasets where dimension is applicable.
   *
   * For Image data, both the image dimension and pixel component count are checked.
   * For non-Image data, the dimension is inferred from the TimeGeometry if the
   * pixel component count is the default value (1).
   * Evaluates to "false" for unsupported datasets.
   *
   * \ingroup DataStorage
   */
  class MITKCORE_EXPORT NodePredicateDimension : public NodePredicateBase
  {
  public:
    mitkClassMacro(NodePredicateDimension, NodePredicateBase);
    mitkNewMacro1Param(NodePredicateDimension, unsigned int);
    mitkNewMacro2Param(NodePredicateDimension, unsigned int, int);

    /** \brief Standard Destructor. */
    ~NodePredicateDimension() override;

    /** \brief Checks if the node's data object has the specified dimension.
     *
     * \param node The DataNode to check.
     * \return True if the data object dimension and pixel components match.
     */
    bool CheckNode(const mitk::DataNode *node) const override;

  protected:
    /** \brief Constructor with dimension only (pixel components defaults to 1).
     *
     * \param dimension The required image dimension.
     */
    NodePredicateDimension(unsigned int dimension);

    /** \brief Constructor with dimension and pixel component count.
     *
     * \param dimension The required image dimension.
     * \param pixelComponents The required number of pixel components.
     */
    NodePredicateDimension(unsigned int dimension, int pixelComponents);

    unsigned int m_Dimension;
    std::size_t m_PixelComponents;
  };
} // namespace mitk

#endif
