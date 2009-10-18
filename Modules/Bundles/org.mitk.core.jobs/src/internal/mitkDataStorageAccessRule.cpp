/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 18503 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkDataStorageAccessRule.h"
#include <Poco/Exception.h>






mitk::DataStorageAccessRule
::DataStorageAccessRule (mitk::DataStorage::Pointer myDataStorage, mitk::DataTreeNode::Pointer myDataTreeNode, 
                         mitk::DataStorageAccessRule::RuleType myRule) 
 :m_Rule(myRule),
  m_sptrMyDataStorage(myDataStorage),
  m_sptrMyDataTreeNode(myDataTreeNode)

 {
   mitk::DataStorage::SetOfObjects::ConstPointer 
   sptr_parentsNodesFirstRule = m_sptrMyDataStorage->GetSources(m_sptrMyDataTreeNode);
   // checks if the DataTreeNode does exists within the specified DataTree, if not an 
   // Poco NotFoundException is thrown since the rule is useless 
   bool exsists = false ;
   for(mitk::DataStorage::SetOfObjects::const_iterator it = 
        sptr_parentsNodesFirstRule->begin(); it != sptr_parentsNodesFirstRule->end(); ++ it)
      { 
        if (*it == m_sptrMyDataTreeNode ) exsists = true ;
      }
   if (exsists == false) throw  Poco::NotFoundException()  ; 
     
 }

  

bool  
  mitk::DataStorageAccessRule
   ::Contains(cherry::ISchedulingRule::Pointer rule) 
  {
  DataStorageAccessRule::Pointer sptr_temp = rule.Cast<DataStorageAccessRule>();
  // test if the ISchedulingRule is a DataStorageAccessRule 
  if( sptr_temp == 0 ) return false ; 
  // test if the DataStorageAccessRule object is exactly the instance 
  else {
    if ( this == sptr_temp.GetPointer() ) return true ;

    return false ;
    }
  }



bool
  mitk::DataStorageAccessRule
    ::IsConflicting(cherry::ISchedulingRule::Pointer sptr_otherISchedulingRule) 
  {
     // test if the stored dataTreeNode 
     // cast to check if the ISchedulingRule is a DataStorageAccessRule 
    DataStorageAccessRule::Pointer sptr_DataStorageAccessRule = sptr_otherISchedulingRule.Cast<DataStorageAccessRule>();
     // checks if the Rule of the type ISchedulingRule is a DataStorageAccessRule
     if(sptr_DataStorageAccessRule == 0) return false ; 

     // the rule to be compared with is a DataStorageAccessRule
     else      
   {
     // testing if both jobs holding each rule do operate on the same DataStorage if not false is returned 
     if( !CompareDataStorages(sptr_DataStorageAccessRule->GetDataStorage()) ) return false ; 

     // checks if to rules to be compared are two add rules 
     if (m_Rule == ADD_RULE && m_Rule == sptr_DataStorageAccessRule->GetRuleType()) 
       return CompareTwoAddorRemoveRules() ; 

     // checks if to the two rules to be compared are two remove rules 
     if(m_Rule == REMOVE_RULE && m_Rule == sptr_DataStorageAccessRule->GetRuleType()) 
       return CompareTwoAddorRemoveRules() ; 

     // an add and remove rule needs to be compared
     else 
       { 
        return CompareAddandRemoveRules(sptr_DataStorageAccessRule) ;
       }
     
   }

  }

 bool
   mitk::DataStorageAccessRule
   ::CompareAddandRemoveRules(mitk::DataStorageAccessRule::Pointer sptr_otherDataStorageAccessRule) const
 {
      
      mitk::DataStorage::SetOfObjects::ConstPointer 
        sptr_parentsNodesFirstRule = m_sptrMyDataStorage->GetSources(m_sptrMyDataTreeNode);
     // checks if the DataStorageNode of to be compared DataStorageAccessRule is a parent node 
     // if so the job holding this DataStorageAccessRule need to wait until the operation on the parent node is performed 
      for(mitk::DataStorage::SetOfObjects::const_iterator it = 
        sptr_parentsNodesFirstRule->begin(); it != sptr_parentsNodesFirstRule->end(); ++ it)
      {
      if (*it == sptr_otherDataStorageAccessRule->GetDataTreeNode()) return true ;
      }
      mitk::DataStorage::SetOfObjects::ConstPointer
        sptr_derivedNodesRule = m_sptrMyDataStorage->GetDerivations(m_sptrMyDataTreeNode); 
      // checks if the DataStorage node of to be compared DataStorageAccessRule is a child node 
      // if so the job holding this DataStorageAccessRule needs to wait until the operation on the parent node is performed 
      for(mitk::DataStorage::SetOfObjects::const_iterator it = 
        sptr_derivedNodesRule->begin(); it != sptr_derivedNodesRule->end(); ++it)
      {
      if(*it == sptr_otherDataStorageAccessRule->GetDataTreeNode()) return true ;
      }
      
     // jobs operating on nodes on different branches do not cause conflicts thus they can be performed in different  
     // threads concurrently 
        return false ;  
 }



 bool 
   mitk::DataStorageAccessRule
   ::CompareDataStorages(mitk::DataStorage::Pointer otherDataStorage) const
   {
    if(m_sptrMyDataStorage == otherDataStorage) return true ; 
    else return false; 
   }



 bool
   mitk::DataStorageAccessRule
   ::CompareTwoAddorRemoveRules() const
   { 
       return false ; 
   }



 bool 
   mitk::DataStorageAccessRule
   ::TestDataTreeNode(mitk::DataTreeNode::Pointer  /*dataTreeToBeStored*/) const 
  {
   mitk::DataStorage::SetOfObjects::ConstPointer tempAllNodes = m_sptrMyDataStorage->GetAll(); 
   for(mitk::DataStorage::SetOfObjects::const_iterator it = tempAllNodes->begin(); it !=tempAllNodes->end(); ++ it){
     if (m_sptrMyDataTreeNode == *it ) return true ;

     }
   return false ; 
   
  }


  
 mitk::DataTreeNode::Pointer 
   mitk::DataStorageAccessRule
    ::GetDataTreeNode() const
   {
    return mitk::DataStorageAccessRule::m_sptrMyDataTreeNode ;  
   }



 mitk::DataStorage::Pointer
   mitk::DataStorageAccessRule
   ::GetDataStorage() const
  {
    return mitk::DataStorageAccessRule::m_sptrMyDataStorage ;     
  }


 mitk::DataStorageAccessRule::RuleType
   mitk::DataStorageAccessRule
   ::GetRuleType() const
   {
   if (m_Rule == ADD_RULE)
     return ( mitk::DataStorageAccessRule::ADD_RULE ) ;
   else {
     return ( mitk::DataStorageAccessRule::REMOVE_RULE ) ;

     }
   }


  
 
