#ifndef QMITK_ALGORITHMFUNCTIONALITYCOMPONENT_H
#define QMITK_ALGORITHMENFUNCTIONALITYCOMPONENT_H

#include "QmitkBaseFunctionalityComponent.h"
#include "QmitkBaseAlgorithmComponent.h"
/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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

/**
 * \brief Class for components based on ITK- and MITK-Filters 
 * \ingroup QmitkFunctionalityComponent
 * 
 * 
 * - \ref QmitkBaseFunctionalityComponent
 * - \ref QmitkBaseAlgorithmComponent

 * 
 * \section QmitkAlgorithmFunctionalityComponent Overview
 * 
 * The AlgorithmFunctionalityComponent is a class for components based on ITK- and MTIK- Filters. It  
 * inherits the DataTreeIterator an the name from QmitkBaseFunctionalityComponent, and the Algorithm 
 * with UserInterface from QmitkBaseAlgorithmComponent

 */


class QmitkAlgorithmFunctionalityComponent : public QmitkBaseFunctionalityComponent, public QmitkBaseAlgorithmComponent
{

  public:

  protected:

  private:



};

#endif

