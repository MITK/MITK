#ifndef QMITK_INTERACTIONFUNCTIONALITYCOMPONENT_H
#define QMITK_INTERACTIONFUNCTIONALITYCOMPONENT_H

#include "QmitkBaseFunctionalityComponent.h"


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
 * \brief Class for components that interact with the image 
 * \ingroup QmitkFunctionalityComponent
 * 
 * 
 * - \ref QmitkBaseFunctionalityComponent
 * - \ref QmitkBaseAlgorithmComponent

 * 
 * \section QmitkInteractionFunctionalityComponent Overview
 * 
 * The InteractionFunctionalityComponent is a class for components that interact with the image. It  
 * inherits the DataTreeIterator an the name from QmitkBaseFunctionalityComponent and has an own state
 * machine and an interactor (switch on/of)

 */


class QmitkInteractionFunctionalityComponent : public QmitkBaseFunctionalityComponent
{

  public:

     /** \brief Constructor. */
    QmitkInteractionFunctionalityComponent();

     /** \brief Destructor. */
    ~QmitkInteractionFunctionalityComponent();



  protected:

  private:



};

#endif

