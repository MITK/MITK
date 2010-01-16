/*=========================================================================
 
Program:   BlueBerry Platform
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

#ifndef BERRYIADAPTERMANAGERPROVIDER_H_
#define BERRYIADAPTERMANAGERPROVIDER_H_

namespace berry {

class AdapterManager;

/**
 * The callback interface for the elements desiring to lazily supply
 * information to the adapter manager.
 * 
 * @since org.blueberry.core.runtime 3.2
 */
struct IAdapterManagerProvider {

  /**
   * Add factories. The method called before the AdapterManager starts
   * using factories.
   *  
   * @param adapterManager the adapter manager that is about to be used
   * @return <code>true</code> if factories were added; <code>false</code> 
   * if no factories were added in this method call.
   */
  bool AddFactories(AdapterManager* adapterManager);
};

}  // namespace berry

#endif /*BERRYIADAPTERMANAGERPROVIDER_H_*/
