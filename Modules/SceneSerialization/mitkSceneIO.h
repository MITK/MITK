/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile: mitkPropertyManager.cpp,v $
Language:  C++
Date:      $Date$
Version:   $Revision: 1.12 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#ifndef mitkSceneIO_h_included
#define mitkSceneIO_h_included

#include "SceneSerializationExports.h"

#include "mitkDataStorage.h"
#include "mitkNodePredicateBase.h"

namespace mitk
{

class SceneSerialization_EXPORT SceneIO : public itk::Object
{
  public:

    mitkClassMacro( SceneIO, itk::Object );
    itkNewMacro(Self);

    /**
     * \brief Load a scene of objects from file
     * \return DataStorage with all scene objects and their relations
     *
     * Attempts to read the provided file and create objects with 
     * parent/child relations into a DataStorage.
     * 
     * \param filename full filename of the scene file
     * \param storage If given, this DataStorage is used instead of a newly created one
     * \param clearStorageFirst If set, the provided DataStorage will be cleared before populating it with the loaded objects
     */
    virtual DataStorage::Pointer LoadScene( const std::string& filename, 
                                            DataStorage* storage = NULL, 
                                            bool clearStorageFirst = false );

    /**
     * \brief Save a scene of objects to file
     * \return success or not
     *
     * Attempts to write a scene file, which contains the nodes of the
     * provided DataStorage, their parent/child relations, and properties.
     * 
     * \param storage a DataStorage containing all nodes that should be saved
     * \param filename full filename of the scene file
     * \param predicate defining which items of the datastorage to use and which not
     */
    virtual bool SaveScene( DataStorage* storage,
                            const std::string& filename,
                            NodePredicateBase* predicate = NULL );

  protected:

};

}

#endif

