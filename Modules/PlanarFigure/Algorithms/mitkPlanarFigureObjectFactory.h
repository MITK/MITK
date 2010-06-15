/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2010-04-16 16:53:19 +0200 (Fr, 16 Apr 2010) $
Version:   $Revision: 13129 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#ifndef PLANARFIGUREOBJECTFACTORY_H_INCLUDED
#define PLANARFIGUREOBJECTFACTORY_H_INCLUDED

#include "mitkCoreObjectFactoryBase.h"
#include "PlanarFigureExports.h"

namespace mitk {

class PlanarFigure_EXPORT PlanarFigureObjectFactory : public CoreObjectFactoryBase
{
public:

  mitkClassMacro(PlanarFigureObjectFactory,CoreObjectFactoryBase);
  itkNewMacro(PlanarFigureObjectFactory);

  virtual Mapper::Pointer CreateMapper(mitk::DataNode* node, MapperSlotId slotId);

  virtual void SetDefaultProperties(mitk::DataNode* node);

  virtual const char* GetFileExtensions();

  virtual const char* GetSaveFileExtensions();

  void RegisterIOFactories();

protected:
  PlanarFigureObjectFactory(); 
};

}
// global declaration for simple call by
// applications
void PlanarFigure_EXPORT RegisterPlanarFigureObjectFactory();

#endif // PLANARFIGUREOBJECTFACTORY_H_INCLUDED
