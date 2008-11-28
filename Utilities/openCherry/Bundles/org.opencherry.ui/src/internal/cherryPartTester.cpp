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

#include "cherryPartTester.h"

#include <Poco/Exception.h>

#include <cherryObjectString.h>

#include "../cherryIEditorSite.h"
#include "../cherryIViewSite.h"

namespace cherry
{

void PartTester::TestEditor(IEditorPart* part)
{

  TestWorkbenchPart(part);

  if (!(part->GetSite() == part->GetEditorSite().GetPointer()))
    throw Poco::AssertionViolationException("The part's editor site must be the same as the part's site"); //$NON-NLS-1$
  IEditorInput::Pointer input = part->GetEditorInput();
  if (input.IsNull())
    throw Poco::AssertionViolationException("The editor input must be non-null"); //$NON-NLS-1$

  TestEditorInput(input);

  part->IsDirty();
  part->IsSaveAsAllowed();
  part->IsSaveOnCloseNeeded();
}

void PartTester::TestEditorInput(IEditorInput* /*input*/)
{

  //input.getAdapter(Object.class);

  // Don't test input.getImageDescriptor() -- the workbench never uses that
  // method and most editor inputs would fail the test. It should really be
  // deprecated.


  // Persistable element may be null
  //    IPersistableElement persistableElement = input.getPersistable();
  //    if (persistableElement != null) {
  //      Assert
  //          .isNotNull(persistableElement.getFactoryId(),
  //              "The persistable element for the editor input must have a non-null factory id"); //$NON-NLS-1$
  //        }
}

void PartTester::TestWorkbenchPart(IWorkbenchPart* part)
{
  //        IPropertyListener testListener = new IPropertyListener() {
  //            public void propertyChanged(Object source, int propId) {
  //                
  //            }
  //        };

  // Test addPropertyListener
  //part.addPropertyListener(testListener);

  // Test removePropertyListener
  //part.removePropertyListener(testListener);

  // Test equals
  if (!(part->operator==(part)))
    throw Poco::AssertionViolationException("A part must be equal to itself"); //$NON-NLS-1$
  Object::Pointer obj = new ObjectString("jo");
  if (part->operator==(obj))
    throw Poco::AssertionViolationException("A part must have a meaningful operator== method"); //$NON-NLS-1$

  // Test getAdapter   
  //Object partAdapter = part.getAdapter(part.getClass());
  //Assert.isTrue(partAdapter == null || partAdapter == part,
  //"A part must adapter to itself or return null"); //$NON-NLS-1$

  // Test getTitleImage
  //Assert.isNotNull(part.getTitleImage(),
  //    "A part's title image must be non-null"); //$NON-NLS-1$

  // Compute hashCode
  //part.hashCode();

}

void PartTester::TestView(IViewPart* part)
{
  if (!(part->GetSite() == part->GetViewSite().GetPointer()))
    throw Poco::AssertionViolationException(
        "A part's site must be the same as a part's view site"); //$NON-NLS-1$

  TestWorkbenchPart(part);
}

}
