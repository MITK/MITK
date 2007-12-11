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

/**
 * \brief Base class for components using ITK- and MITK-Filters. 
 * \ingroup QmitkFunctionalityComponent
 * 
 * 
 * 

 * 
 * \section BaseBaseAlgorithmComponent Overview
 * 
 * The BaseFunctionalityComponent ist the root-class for all components, based on ITK or MITK-
 * Filters. For that Filters, the BaseFunctionalityComponent contains the graphical user inter-
 * face.
 
 * Inherit class is:
 * 
 * - \ref QmitkAlgorithmFunctionalityComponent
 * 
 */


#ifndef QMITK_BASEALGORITHMCOMPONENT_H
#define QMITK_BASEALGORITHMCOMPONENT_H

class QmitkDataTreeComboBox;
class QmitkBaseAlgorithmComponent 
{

  public:

    
     /** \brief Constructor. */
    QmitkBaseAlgorithmComponent( );

     /** \brief Destructor. */
    ~QmitkBaseAlgorithmComponent();


  protected:



  private:



};

#endif

