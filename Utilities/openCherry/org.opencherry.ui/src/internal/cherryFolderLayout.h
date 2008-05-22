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

#ifndef CHERRYFOLDERLAYOUT_H_
#define CHERRYFOLDERLAYOUT_H_

#include "../cherryIFolderLayout.h"

#include "cherryPartStack.h"
#include "cherryViewFactory.h"

namespace cherry {

class PageLayout;


/**
 * This layout is used to define the initial set of views and placeholders
 * in a folder.
 * <p>
 * Views are added to the folder by ID. This id is used to identify 
 * a view descriptor in the view registry, and this descriptor is used to 
 * instantiate the <code>IViewPart</code>.
 * </p>
 */
class FolderLayout : public IFolderLayout {
  
public: cherryClassMacro(FolderLayout)
  
private:
  
  PartStack::Pointer folder;

  SmartPointer<PageLayout> pageLayout;

  ViewFactory* viewFactory;

    
public:

    /**
     * Create an instance of a <code>FolderLayout</code> belonging to a 
     * <code>PageLayout</code>.
     */
     FolderLayout(SmartPointer<PageLayout> pageLayout, PartStack::Pointer folder,
            ViewFactory* viewFactory);

    /* (non-Javadoc)
     * @see org.eclipse.ui.IPlaceholderFolderLayout#addPlaceholder(java.lang.String)
     */
    void AddPlaceholder(const std::string& viewId);

    /* (non-Javadoc)
     * @see org.eclipse.ui.IFolderLayout#addView(java.lang.String)
     */
    void AddView(const std::string& viewId);

    /* (non-Javadoc)
     * @see org.eclipse.ui.IPlaceholderFolderLayout#getProperty(java.lang.String)
     */
    std::string GetProperty(const std::string& id);

    /* (non-Javadoc)
     * @see org.eclipse.ui.IPlaceholderFolderLayout#setProperty(java.lang.String, java.lang.String)
     */
    void SetProperty(const std::string& id, const std::string& value);
 
    
    private:
      
    /**
        * Inform the page layout of the new part created
        * and the folder the part belongs to.
        */
  void LinkPartToPageLayout(const std::string& viewId, LayoutPart::Pointer newPart);
};

}

#endif /*CHERRYFOLDERLAYOUT_H_*/
