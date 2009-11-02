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


#ifndef CHERRYINTROCONSTANTS_H_
#define CHERRYINTROCONSTANTS_H_

#include <string>

namespace cherry {

/**
 * Contains constants used by the intro implementation
 *
 * @since 3.0
 */
struct IntroConstants {

    /**
     * The id of the view that is used as the intro host.
     */
    static const std::string INTRO_VIEW_ID; // = "org.opencherry.ui.internal.introview";

    /**
     * The id of the editor that is used as the intro host.
     */
    static const std::string INTRO_EDITOR_ID; // = "org.opencherry.ui.internal.introeditor";

};


}

#endif /* CHERRYINTROCONSTANTS_H_ */
