#ifndef QMITK_BASEFUNCTIONALITYCOMPONENT_H
#define QMITK_BASEFUNCTIONALITYCOMPONENT_H

/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: -1 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


/**
* \brief Base class for hierarchical functionality components. This class contains the DataTreeIterator, access by the usual set-and get-methods, and the name-attribut of a component
* \ingroup QmitkFunctionalityComponent
* 
* 
* 

* 
* \section BaseFunctionalityComponent Overview
* 
* The BaseFunctionalityComponent is the root-class for all component-based functionalities.
* As common ground for all inherit classes it contains the DataTreeIterator, i.e an iterator
* on the current node of the data tree, and the name-attribute of the component. 
* Inherit classes are:
* 
* - \ref QmitkAlgorithmFunctionalityComponent
* - \ref QmitkInteractionFunctionalityComponent
* - \ref QmitkFunctionalityComponentContainer
* 
*/

#include <qobject.h>
#include "QmitkBaseComponent.h"
#include "mitkDataTreeNode.h"
#include <mitkDataStorage.h>


class QMITKEXT_EXPORT QmitkBaseFunctionalityComponent : public QmitkBaseComponent
{
  Q_OBJECT

public:
  /***************       CONSTRUCTOR      ***************/
  /** 
  * \brief Constructor. 
  */
  QmitkBaseFunctionalityComponent(QObject *parent=0, const char *name=0);

  /***************        DESTRUCTOR      ***************/
  /** 
  * @brief Destructor. 
  */
  virtual  ~QmitkBaseFunctionalityComponent();

  /***************        DESTRUCTOR      ***************/
  /** 
  * @brief Return an object of mitkDataStorage
  */
  mitk::DataStorage::Pointer GetDefaultDataStorage();


  /***************      OHTER METHODS     ***************/
  
  /** \brief Slot method that will be called if TreeNodeSelector widget was activated to select the current image. */
  void ImageSelected(const mitk::DataTreeNode::Pointer item);

  /*!
  * @brief Image selected in MainSelector
  */
  mitk::Image* m_ParentMitkImage;

  /*!
  * @brief Image selected in MainSelector
  */
  mitk::Image* m_MitkImage;

  mitk::DataStorage::Pointer m_DataStorage;


};
#endif

