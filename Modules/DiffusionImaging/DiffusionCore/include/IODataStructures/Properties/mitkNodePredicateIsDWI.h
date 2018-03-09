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


#ifndef MITKNODEPREDICATEISDWI_H_HEADER_INCLUDED_
#define MITKNODEPREDICATEISDWI_H_HEADER_INCLUDED_

#include <MitkDiffusionCoreExports.h>

#include "mitkNodePredicateBase.h"
#include "mitkBaseProperty.h"
#include "mitkBaseData.h"

namespace mitk {

    /**Documentation
     * @brief Predicate that evaluates whether a given node contains a diffusion weighted image
     * @ingroup DataStorage
     */

    class MITKDIFFUSIONCORE_EXPORT NodePredicateIsDWI : public NodePredicateBase
    {
    public:
      mitkClassMacro(NodePredicateIsDWI, NodePredicateBase);
      itkFactorylessNewMacro(NodePredicateIsDWI)

      //##Documentation
      //## @brief Standard Destructor
      ~NodePredicateIsDWI() override;

      //##Documentation
      //## @brief Checks, if the node's data contains a property that is equal to m_ValidProperty
      bool CheckNode(const mitk::DataNode* node) const override;

    protected:
      //##Documentation
      //## @brief Constructor to check for a named property
      NodePredicateIsDWI();
    };

} // namespace mitk

#endif /* MITKNODEPREDICATEISDWI_H_HEADER_INCLUDED_ */
