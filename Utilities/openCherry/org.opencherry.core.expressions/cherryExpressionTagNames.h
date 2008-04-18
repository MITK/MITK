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

#ifndef CHERRYEXPRESSIONTAGNAMES_H_
#define CHERRYEXPRESSIONTAGNAMES_H_

#include <string>

#include <org.opencherry.osgi/cherryDll.h>

namespace cherry {

/**
 * Class defining the tag names of the XML elements of the common
 * expression language.
 * 
 * @since 3.0
 */
class CHERRY_API ExpressionTagNames {

public:
  
  /** The tag name of the enablement expression (value: <code>enablement</code>) */
  static const std::string ENABLEMENT;
  
  /** The tag name of the and expression (value: <code>and</code>) */
  static const std::string AND;
  
  /** The tag name of the or expression (value: <code>or</code>) */
  static const std::string OR;
  
  /** The tag name of the not expression (value: <code>not</code>) */
  static const std::string NOT;
  
  /** The tag name of the instanceof expression (value: <code>instanceof</code>) */
  static const std::string INSTANCEOF;
  
  /** The tag name of the test expression (value: <code>test</code>) */
  static const std::string TEST;
  
  /** The tag name of the with expression (value: <code>with</code>) */
  static const std::string WITH;
  
  /** The tag name of the adapt expression (value: <code>adapt</code>) */
  static const std::string ADAPT;
  
  /** The tag name of the count expression (value: <code>count</code>) */
  static const std::string COUNT;
  
  /** The tag name of the adapt expression (value: <code>iterate</code>) */
  static const std::string ITERATE;
  
  /** The tag name of the resolve expression (value: <code>resolve</code>) */
  static const std::string RESOLVE;

  /** The tag name of the systemTest expression (value: <code>systemTest</code>) */
  static const std::string SYSTEM_TEST;
  
  /** The tag name of the equals expression (value: <code>equals</code>) */
  static const std::string EQUALS;

  /**
   * The tag name of the reference expression (value: <code>reference</code>)
   * 
   * @since 3.3
   */
  static const std::string REFERENCE;
};

}  // namespace cherry

#endif /*CHERRYEXPRESSIONTAGNAMES_H_*/
