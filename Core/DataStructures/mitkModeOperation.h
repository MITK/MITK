/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
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


#ifndef MODEOPERATION_H_HEADER_INCLUDED
#define MODEOPERATION_H_HEADER_INCLUDED

#include "mitkCommon.h"
#include "mitkOperation.h"
#include <mitkInteractor.h>

namespace mitk {
//  class Operation;

//##Documentation
//## @brief class that holds the information for a change of the modus of an interactor object
//##
//## @ingroup Undo
class ModeOperation : public Operation
{
public:
  typedef Interactor::ModeType ModeType;

  mitkClassMacro(ModeOperation, Operation);
  //##Documentation
	//## Constructor
  ModeOperation(OperationType operationType, ModeType mode);

  virtual ~ModeOperation();

  ModeType GetMode();

protected:
  
  ModeType m_Mode;
};
}//namespace mitk
#endif /* MODEOPERATION_H_HEADER_INCLUDED */
