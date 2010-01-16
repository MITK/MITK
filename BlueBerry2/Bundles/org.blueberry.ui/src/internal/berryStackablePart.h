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


#ifndef BERRYSTACKABLEPART_H_
#define BERRYSTACKABLEPART_H_

#include <osgi/framework/Macros.h>

#include "../berryShell.h"

#include "../berryPoint.h"
#include "../berryRectangle.h"

#include <string>

namespace berry {

struct IStackableContainer;
struct IWorkbenchWindow;

class StackablePart : public virtual Object
{

public:

  osgiObjectMacro(StackablePart);

  StackablePart(std::string id);

  virtual void CreateControl(void* parent) = 0;
  virtual void* GetControl() = 0;

  virtual SmartPointer<IStackableContainer> GetContainer() const;
  virtual void SetContainer(SmartPointer<IStackableContainer> container);

  virtual void Reparent(void* newParent);

  virtual void DescribeLayout(std::string& description) const;

  virtual std::string GetPlaceHolderId() const;

  virtual std::string GetId() const;

  virtual void SetId(const std::string& id);

  /**
   * Sets focus to this part.
   */
  virtual void SetFocus();

  virtual void SetBounds(const Rectangle& bounds);
  virtual Rectangle GetBounds();

  virtual Point GetSize();

  virtual bool IsDocked();

  virtual Shell::Pointer GetShell();

  SmartPointer<IWorkbenchWindow> GetWorkbenchWindow();

  /**
   * Returns the compound ID for this part.
   * The compound ID is of the form: primaryId [':' + secondaryId]
   *
   * @return the compound ID for this part.
   */
  virtual std::string GetCompoundId();

  virtual bool IsPlaceHolder() const;

  virtual void TestInvariants();

private:

  std::string id;
  SmartPointer<IStackableContainer> container;

};

}

#endif /* BERRYSTACKABLEPART_H_ */
