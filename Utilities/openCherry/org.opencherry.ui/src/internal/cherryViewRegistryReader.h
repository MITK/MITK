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

#ifndef CHERRYVIEWREGISTRYREADER_H_
#define CHERRYVIEWREGISTRYREADER_H_

#include "cherryRegistryReader.h"

namespace cherry
{

class ViewRegistry;

/**
 * \ingroup org_opencherry_ui_internal
 * 
 * A strategy to read view extensions from the registry.
 */
class ViewRegistryReader : public RegistryReader
{

public:

  /**
   * General view category id 
   */
  static std::string GENERAL_VIEW_ID ;

  /**
   * RegistryViewReader constructor comment.
   */
  ViewRegistryReader();

  /**
   * Read the view extensions within a registry.
   * @param in the extension registry
   * @param out the view registry
   */
  void ReadViews(ViewRegistry* out);

protected:
  /**
   * Reads the category element.
   */
  void ReadCategory(IConfigurationElement::Pointer element);

  /**
   * readElement method comment.
   */
  bool ReadElement(IConfigurationElement::Pointer element);

  /**
   * Reads the view element.
   */
  void ReadView(IConfigurationElement::Pointer element);

private:

  ViewRegistry* viewRegistry;

};

} // namespace cherry

#endif /*CHERRYVIEWREGISTRYREADER_H_*/
