/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITK_PLANAR_FIGURE_IO_H
#define MITK_PLANAR_FIGURE_IO_H

#include <mitkAbstractFileIO.h>
#include <mitkPlanarFigure.h>

class TiXmlElement;

namespace mitk
{
  /**
  * Reads/Writes a PlanarFigure to a file
  * @ingroup Process
  */
  class PlanarFigureIO : public mitk::AbstractFileIO
  {
  public:
    typedef mitk::PlanarFigure InputType;

    PlanarFigureIO();

    // -------------- AbstractFileReader -------------

    using AbstractFileReader::Read;

    ConfidenceLevel GetReaderConfidenceLevel() const override;

    // -------------- AbstractFileWriter -------------

    void Write() override;
    ConfidenceLevel GetWriterConfidenceLevel() const override;

  protected:
    /**
    * @brief Reads a number of mitk::PlanarFigures from the file system
    * @return a vector of mitk::PlanarFigures
    * @throws throws an mitk::Exception if an error ocurrs during parsing the nrrd header
    */
    std::vector<itk::SmartPointer<BaseData>> DoRead() override;

    using DoubleList = std::list<double>;
    /**
    * \brief parses the element for the attributes name0 to nameN, where "name" and the number of attributes
    * to read are passed as argument. Returns a list of double vales.
    * \param[in] e the TiXmlElement that will be parsed
    * \param[in] attributeNameBase the basic name of the parameters
    * \param[in] count the number of parameters
    * \return returns a mitk::Point3D with the values x,y,z
    */
    DoubleList GetDoubleAttributeListFromXMLNode(TiXmlElement* e, const char* attributeNameBase, unsigned int count);

    /**
    * \brief parses the element for the attributes x,y,z and returns a mitk::Vector3D filled with these values
    * \param[in] e the TiXmlElement that will be parsed
    * \return returns a mitk::Vector3D with the values x,y,z
    */
    static mitk::Vector3D GetVectorFromXMLNode(TiXmlElement* e);

    /**
    * \brief parses the element for the attributes x,y,z and returns a mitk::Point3D filled with these values
    * \param[in] e the TiXmlElement that will be parsed
    * \return returns a mitk::Point3D with the values x,y,z
    */
    static mitk::Point3D GetPointFromXMLNode(TiXmlElement* e);

    /**Documentation
    * \brief creates a TinyXML element that contains x, y, and z values
    *
    * \param[in] name the name of the XML element
    * \param[in] v the vector or point that contains the x, y and z values
    * \return returns a TiXmlElement named name and three attributes x, y and z.
    */
    static TiXmlElement* CreateXMLVectorElement(const char* name, itk::FixedArray<mitk::ScalarType, 3> v);

  private:
    PlanarFigureIO *IOClone() const override;
  };
} // end of namespace mitk

#endif // MITK_PLANAR_FIGURE_IO_H
