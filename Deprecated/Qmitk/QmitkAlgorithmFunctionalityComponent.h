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
#ifndef QMITK_ALGORITHMFUNCTIONALITYCOMPONENT_H
#define QMITK_ALGORITHMENFUNCTIONALITYCOMPONENT_H

#include "QmitkBaseFunctionalityComponent.h"
#include "QmitkBaseAlgorithmComponent.h"

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

// TODO : check inheritance! only one base class can be subtype of QObject (see man moc)

class QMITK_EXPORT QmitkAlgorithmFunctionalityComponent : public QmitkBaseFunctionalityComponent, public QmitkBaseAlgorithmComponent
{

  Q_OBJECT

  public:

    QmitkAlgorithmFunctionalityComponent();
    ~QmitkAlgorithmFunctionalityComponent();

  protected:

  private:



};

#endif

