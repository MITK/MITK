/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef MITKNODEPREDICATESOURCE_H_HEADER_INCLUDED_
#define MITKNODEPREDICATESOURCE_H_HEADER_INCLUDED_

#include "mitkNodePredicateBase.h"
#include "mitkDataStorage.h"
#include "mitkDataNode.h"
#include "mitkWeakPointer.h"

namespace mitk {

    //##Documentation
    //## @brief Predicate that evaluates if the given node is a direct or indirect source node of a specific node
    //## 
    //## @warning Calling CheckNode() can be computationally quite expensive for a large DataStorage.
    //##          Alternatively mitk::StandaloneDataStorage::GetSources() can be used 
    //## 
    //## @ingroup DataStorage
    class MITK_CORE_EXPORT NodePredicateSource : public NodePredicateBase
    {
    public:

      mitkClassMacro(NodePredicateSource, NodePredicateBase);
      mitkNewMacro3Param(NodePredicateSource, mitk::DataNode*, bool, mitk::DataStorage*);

      //##Documentation
      //## @brief Standard Destructor
      virtual ~NodePredicateSource();

      //##Documentation
      //## @brief Checks, if m_BaseNode is a source node of childNode  (e.g. if childNode "was created from" m_BaseNode)
      virtual bool CheckNode(const mitk::DataNode* childNode) const;

    protected:
      //##Documentation
      //## @brief Constructor - This class can either search only for direct source objects or for all source objects
      NodePredicateSource(mitk::DataNode* n, bool allsources, mitk::DataStorage* ds);

      mitk::WeakPointer<mitk::DataNode> m_BaseNode;
      bool m_SearchAllSources;
      mitk::WeakPointer<mitk::DataStorage> m_DataStorage;
    };

} // namespace mitk

#endif /* MITKNODEPREDICATESOURCE_H_HEADER_INCLUDED_ */
