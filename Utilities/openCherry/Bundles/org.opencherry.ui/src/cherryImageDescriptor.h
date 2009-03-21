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


#ifndef CHERRYIIMAGEDESCRIPTOR_H_
#define CHERRYIIMAGEDESCRIPTOR_H_

#include <cherryObject.h>
#include <cherryMacros.h>

#include "cherryUiDll.h"

namespace cherry {

/**
 * An image descriptor is an object that knows how to create
 * an image. Caching of images and resource management must be done
 * in GUI toolkit specific classes. An image descriptor
 * is intended to be a lightweight representation of an image.
 * <p>
 * To get an Image from an ImageDescriptor, the method
 * CreateImage() must be called. When the caller is done with an image obtained from CreateImage,
 * they must call DestroyImage() to give the GUI toolkit a chance to cleanup resources.
 * </p>
 *
 * @see org.eclipse.swt.graphics.Image
 */
struct CHERRY_UI ImageDescriptor : public Object {

  cherryObjectMacro(ImageDescriptor);

  /**
   * Returns a possibly cached image for this image descriptor. The
   * returned image must be explicitly disposed by calling DestroyImage().
   * The image will not be automatically garbage collected. In the event
   * of an error, a default image is returned if
   * <code>returnMissingImageOnError</code> is true, otherwise
   * <code>0</code> is returned.
   * <p>
   * Note: Even if <code>returnMissingImageOnError</code> is true, it is
   * still possible for this method to return <code>null</code> in extreme
   * cases, for example if the underlying OS/GUI toolkit runs out of image handles.
   * </p>
   *
   * @param returnMissingImageOnError
   *            flag that determines if a default image is returned on error
   * @return the image or <code>0</code> if the image could not be
   *         created
   */
  virtual void* CreateImage(bool returnMissingImageOnError = true) = 0;

  virtual void DestroyImage(void* img) = 0;

  virtual bool operator==(const Object* o) const = 0;

  /**
   * Creates and returns a new image descriptor from a file. If you specify <code>pluginid</code>, the
   * given <code>filename</code> is interpreted relative to the plugins base directory. If no <code>pluginid</code>
   * is given and <code>filename</code> is a relative path, the result is undefined.
   *
   * @param filename the file name.
   * @param pluginid the plugin id of the plugin which contains the file
   * @return a new image descriptor
   */
  static Pointer CreateFromFile(const std::string& filename, const std::string& pluginid = "");

  /**
   * Creates and returns a new image descriptor for the given image. Note
   * that disposing the original Image will cause the descriptor to become invalid.
   *
   * @since 3.1
   *
   * @param img image to create
   * @return a newly created image descriptor
   */
  static Pointer CreateFromImage(void* img);

  /**
   * Returns the shared image descriptor for a missing image.
   *
   * @return the missing image descriptor
   */
  static Pointer GetMissingImageDescriptor();


protected:
  ImageDescriptor();
};

}


#endif /* CHERRYIIMAGEDESCRIPTOR_H_ */
