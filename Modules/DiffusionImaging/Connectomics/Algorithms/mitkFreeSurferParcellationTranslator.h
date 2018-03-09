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

#ifndef mitkFreeSurferParcellationTranslator_h
#define mitkFreeSurferParcellationTranslator_h

// MITK
#include <MitkConnectomicsExports.h>
#include <itkObject.h>
#include <mitkDataNode.h>
#include <mitkLookupTable.h>
#include <mitkTransferFunction.h>
// VTK
#include <vtkSmartPointer.h>

using namespace mitk;

namespace mitk
{
  /**
   * @brief The mitkFreeSurferParcellationTranslator class
   *
   * This class provides a ready to use lookup table for freesurfer processed images and further functions e.g. to get the name assigned to a given label.
   * Additional you can assign the lookup table or transferfunction by using the AssignLookupTable(DataNode* node) or AssignTransferfunction(DataNode* node) methods.
   *
   */
  class MITKCONNECTOMICS_EXPORT FreeSurferParcellationTranslator : public itk::Object
  {
      /*###########      Constructors, Typedefs and Enumerations      ###########*/
    public:
      typedef std::map<std::string, std::string>  NameTable;
      mitkClassMacroItkParent( FreeSurferParcellationTranslator, itk::Object )
      itkNewMacro(    FreeSurferParcellationTranslator )
    protected:
      FreeSurferParcellationTranslator();
      ~FreeSurferParcellationTranslator() override;


      /*###########      Methods      ###########*/
    public:
      /**
       * @brief Assign the lookup table to the given node
       * @param node the lookup table should be assigned to
       */
      virtual void                      AssignLookupTable( DataNode::Pointer node ) const;
      /**
       * @brief Assign the color transfer function to the given node
       * @param node the transferfunction should be assigned to
       */
      virtual void                      AssignTransferFunction( DataNode::Pointer node ) const;
      /**
       * @brief Get the label assigned to the given name
       * @param name the label should be assigned to
       * @return label
       */
      virtual const std::string              GetLabel( const std::string & name ) const;
      /**
       * @brief Get the label assigned to the given name
       * @param name the label should be assigned to
       * @return label
       */
      virtual int                       GetLabelAsNumber( const std::string & name ) const;
      /**
       * @brief Return the lookup table
       * @return lookup table
       */
      virtual LookupTable::Pointer      GetLookupTable() const;
      /**
       * @brief Get the name assigned to the given label
       * @param label
       * @return name
       */
      virtual const std::string              GetName( const std::string & label ) const;
      /**
       * @brief Get the name assigned to the given label
       * @param label
       * @return name
       */
      virtual const std::string              GetName( int label ) const;
      /**
       * @brief Get the transfer function
       * @return transfer function
       */
      virtual TransferFunction::Pointer GetTransferFunction() const;
    private:
      static LookupTable::Pointer       CreateLookupTable();
      static TransferFunction::Pointer  CreateTransferFunction();
      static NameTable                  CreateNameTable();


      /*###########   Static Members   ###########*/
    private:
      static LookupTable::Pointer       m_LookupTable;
      static TransferFunction::Pointer  m_TransferFunction;
      static const NameTable            m_NameTable;
  };
}
#endif // mitkFreeSurferParcellationTranslator_h
