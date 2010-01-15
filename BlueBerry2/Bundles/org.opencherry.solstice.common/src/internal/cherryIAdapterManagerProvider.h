/*=========================================================================
 
Program:   openCherry Platform
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

#ifndef CHERRYIADAPTERMANAGERPROVIDER_H_
#define CHERRYIADAPTERMANAGERPROVIDER_H_

namespace cherry {

class AdapterManager;

/**
 * The callback interface for the elements desiring to lazily supply
 * information to the adapter manager.
 * 
 * @since org.opencherry.core.runtime 3.2
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

}  // namespace cherry

#endif /*CHERRYIADAPTERMANAGERPROVIDER_H_*/
