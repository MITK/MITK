/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkNodePredicateBase_h
#define mitkNodePredicateBase_h

#include "itkObject.h"
#include <MitkCoreExports.h>
#include <mitkCommon.h>

namespace mitk
{
  class DataNode;
  //##Documentation
  //## @brief Interface for evaluation conditions used in the DataStorage class GetSubset() method
  //##
  //## Classes that inherit this interface can be used as predicates in the GetSubset() method
  //## of mitk::DataStorage. By combining different predicate objects, the user can form complex
  //## queries like "give me all nodes that either contain a surface object or a binary segmentation
  //## and that are tagged as Organtype == 'Liver'".
  //## @warning NodePredicates are now derived from itk::Object and make thus use of the smart pointer concept.
  //##          As a result predicates should only store raw pointers because for one thing they are not owners
  //##          of these objects and should not keep them alive.
  //##
  //## @ingroup DataStorage
  class MITKCORE_EXPORT NodePredicateBase : public itk::Object
  {
  public:
    mitkClassMacroItkParent(NodePredicateBase, itk::Object);

    //##Documentation
    //## @brief Standard Destructor
    ~NodePredicateBase() override;

    //##Documentation
    //## @brief This method will be used to evaluate the node. Has to be overwritten in subclasses
    virtual bool CheckNode(const mitk::DataNode *node) const = 0;
  };

} // namespace mitk

#endif
