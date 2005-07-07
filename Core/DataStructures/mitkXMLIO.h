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

#ifndef XMLIO_H_HEADER_INCLUDED
#define XMLIO_H_HEADER_INCLUDED

namespace mitk {

	class XMLWriter;
	class XMLReader;

	class XMLIO {
	
	public:

		/**
		 *
		 */
		virtual bool WriteXML( XMLWriter& xmlWriter );

		/**
		 *
		 */
		virtual bool ReadXML( XMLReader& xmlReader );
	};

}
#endif // XMLIO_H_HEADER_INCLUDED
