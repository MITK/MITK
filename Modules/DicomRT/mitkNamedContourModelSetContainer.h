/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef mitkNamedContourModelSet_h
#define mitkNamedContourModelSet_h

#include <mitkDicomRTExports.h>
#include <itkObject.h>
#include <itkObjectFactory.h>
#include <mitkCommon.h>

#include <mitkContourModelSet.h>

namespace mitk
{

  class mitkDicomRT_EXPORT NamedContourModelSetContainer: public mitk::ContourModelSet
  {

  public:

    /**
     * @brief The NamedContourModelSet class
     */
    class NamedContourModelSet: public itk::Object
    {
    public:
      mitkClassMacro( NamedContourModelSet, itk::Object )
      itkNewMacro( Self )

      void SetRoiName(char*);
      char* roiName;
      void SetContourModelSet(mitk::ContourModelSet::Pointer);
      char* GetRoiName();
      mitk::ContourModelSet::Pointer GetContourModelSet();

    protected:
      NamedContourModelSet();
      virtual ~NamedContourModelSet();

    private:
      mitk::ContourModelSet::Pointer contourModelSet;
    };

    mitkClassMacro( NamedContourModelSetContainer, itk::Object )
    itkNewMacro( Self )

    virtual ~NamedContourModelSetContainer();

    /**
     * @brief Vector for storing the mitk::ContourModelSet and their names.
     *
     * The Vector stores a mitk::NamedContourModelSet::NamedContourModelSet object,
     * which is an inner-class.
     */
    std::vector<mitk::NamedContourModelSetContainer::NamedContourModelSet::Pointer> namedContourModelSetVector;

    /**
     * @brief Stores a mitk::ContourModelSet and a name together into a vector of mitk::NamedContourModelSet objects
     * @param modelSet The mitk::ContourModelSet which should be stored
     * @param name The name of the stored mitk::ContourModelSet
     *
     * The method creates a mitk::NamedContourModelSet::NamedContourModelSet object and stores this object
     * in the mitk::NamedContourModelSet::namedContourModelSetVector.
     */
    void SetNamedContourModelSet(mitk::ContourModelSet::Pointer modelSet, char* name);

  protected:

    NamedContourModelSetContainer();

  };

}

#endif
