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

#include "mitkPlanarFigureCompositeWriter.h"
#include <itksys/SystemTools.hxx>
#include <mitkAbstractFileWriter.h>
#include <mitkCustomMimeType.h>
#include "mitkDiffusionIOMimeTypes.h"
#include <boost/property_tree/ptree.hpp>
#define RAPIDXML_NO_EXCEPTIONS
#include <boost/property_tree/xml_parser.hpp>
#include <mitkPlanarFigureComposite.h>

mitk::PlanarFigureCompositeWriter::PlanarFigureCompositeWriter()
    : mitk::AbstractFileWriter(mitk::PlanarFigureComposite::GetStaticNameOfClass(), mitk::DiffusionIOMimeTypes::PLANARFIGURECOMPOSITE_MIMETYPE(), "Planar Figure Composite Writer")
{
    RegisterService();
}

mitk::PlanarFigureCompositeWriter::PlanarFigureCompositeWriter(const mitk::PlanarFigureCompositeWriter & other)
    :mitk::AbstractFileWriter(other)
{}

mitk::PlanarFigureCompositeWriter::~PlanarFigureCompositeWriter()
{}

mitk::PlanarFigureCompositeWriter * mitk::PlanarFigureCompositeWriter::Clone() const
{
    return new mitk::PlanarFigureCompositeWriter(*this);
}

void mitk::PlanarFigureCompositeWriter::Write()
{

    std::ostream* out;
    std::ofstream outStream;

    if( this->GetOutputStream() )
    {
        out = this->GetOutputStream();
    }else{
        outStream.open( this->GetOutputLocation().c_str() );
        out = &outStream;
    }

    if ( !out->good() )
    {
        mitkThrow() << "Stream not good.";
    }

    try
    {
        const std::string& locale = "C";
        const std::string& currLocale = setlocale( LC_ALL, nullptr );
        setlocale(LC_ALL, locale.c_str());

        std::locale previousLocale(out->getloc());
        std::locale I("C");
        out->imbue(I);

        std::string filename = this->GetOutputLocation().c_str();

        mitk::PlanarFigureComposite::ConstPointer input = dynamic_cast<const mitk::PlanarFigureComposite*>(this->GetInput());
        std::string ext = itksys::SystemTools::GetFilenameLastExtension(this->GetOutputLocation().c_str());

        boost::property_tree::ptree type;
        type.put("comptype", input->getOperationType());
        boost::property_tree::xml_writer_settings<std::string> writerSettings(' ', 2);
        boost::property_tree::xml_parser::write_xml(filename, type, std::locale(), writerSettings);

        setlocale(LC_ALL, currLocale.c_str());
    }
    catch(...)
    {
        throw;
    }
}
