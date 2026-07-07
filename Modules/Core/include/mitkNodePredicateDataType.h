/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkNodePredicateDataType_h
#define mitkNodePredicateDataType_h

#include <mitkDataNode.h>
#include <mitkNodePredicateBase.h>
#include <string>

namespace mitk
{
  /** \brief Predicate that evaluates if the given DataNode's data object is of a specific data type.
   *
   * The data type must be specified in the constructor as a string. The string must equal the
   * result value of the requested data type's GetNameOfClass() method.
   * This predicate performs exact type matching. For type-compatible matching
   * (including derived types), use TNodePredicateDataType.
   *
   * \ingroup DataStorage
   */
  class MITKCORE_EXPORT NodePredicateDataType : public NodePredicateBase
  {
  public:
    mitkClassMacro(NodePredicateDataType, NodePredicateBase);
    mitkNewMacro1Param(NodePredicateDataType, const char *);

    /** \brief Standard Destructor. */
    ~NodePredicateDataType() override;

    /** \brief Checks if the node's data object is of the specified data type.
     *
     * Compares GetNameOfClass() of the data object against the stored type string.
     * Returns false if the node has no data object.
     *
     * \param node The DataNode to check.
     * \return True if the data object type matches the stored type string.
     */
    bool CheckNode(const mitk::DataNode *node) const override;

  protected:
    /** \brief Protected constructor, use static instantiation functions instead.
     *
     * \param datatype The class name string to match (must not be nullptr).
     */
    NodePredicateDataType(const char *datatype);

    std::string m_ValidDataType;
  };

  /**
   * \brief Tests for type compatibility (dynamic_cast).
   *
   * In contrast to NodePredicateDataType this class also accepts derived types.
   * E.g. if you query for type BaseData, you will also get Image and Surface objects.
   *
   * The desired type is given as a template parameter, the constructor takes no other parameters.
   */
  template <class T>
  class TNodePredicateDataType : public NodePredicateBase
  {
  public:
    mitkClassMacro(TNodePredicateDataType, NodePredicateBase);
    itkFactorylessNewMacro(TNodePredicateDataType);

    ~TNodePredicateDataType() override {}

    /** \brief Checks if the node's data object is of the specified type (using dynamic_cast).
     *
     * Also accepts derived types.
     *
     * \param node The DataNode to check.
     * \return True if the data object can be cast to type T.
     */
    bool CheckNode(const mitk::DataNode *node) const override
    {
      return node && node->GetData() && dynamic_cast<T *>(node->GetData());
    }

  protected:
    /** \brief Protected constructor, use static instantiation functions instead. */
    TNodePredicateDataType() {}
  };

} // namespace mitk

#endif
