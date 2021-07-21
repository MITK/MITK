// -----------------------------------------------------------------------
// MatchPoint - DKFZ translational registration framework
//
// Copyright (c) German Cancer Research Center (DKFZ),
// Software development for Integrated Diagnostics and Therapy (SIDT).
// ALL RIGHTS RESERVED.
// See mapCopyright.txt or
// http://www.dkfz.de/en/sidt/projects/MatchPoint/copyright.html
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notices for more information.
//
//------------------------------------------------------------------------
/*!
// @file
// @version $Revision$ (last changed revision)
// @date    $Date$ (last change date)
// @author  $Author$ (last changed by)
// Subversion HeadURL: $HeadURL$
*/


#ifndef __MITK_EVENTS_H
#define __MITK_EVENTS_H

#include "itkEventObject.h"
#include <MitkSegmentationExports.h>

//#include "mapConfigure.h"
//#include "mapMacros.h"
//#include "mapMAPCoreExports.h"

/*#if (defined(_WIN32) || defined(WIN32)) && !defined(MAP_STATIC)
#ifdef MAPUtilities_EXPORTS
#define MAPUtilities_EXPORT __declspec(dllexport)
#else
#define MAPUtilities_EXPORT __declspec(dllimport)
#endif
#else
#define MAPUtilities_EXPORT
#endif
#ifndef _CMAKE_MODULENAME
#ifdef MAPUtilities_EXPORTS
#define _CMAKE_MODULENAME "MAPUtilities"
#endif
#endif

#if (defined(_WIN32) || defined(WIN32)) && !defined(MAP_STATIC)
#ifdef MAPCore_EXPORTS
#define MAPCore_EXPORT __declspec(dllexport)
#else
#define MAPCore_EXPORT __declspec(dllimport)
#endif
#else
#define MAPCore_EXPORT
#endif
#ifndef _CMAKE_MODULENAME
#ifdef MAPCore_EXPORTS
#define _CMAKE_MODULENAME "MAPCore"
#endif
#endif*/

namespace mitk
{
	namespace core
	{

		/** @class EventObject
		 * @brief Base object for all MatchPoint events
		 * @ingroup Events
		 * */
		class MITKSEGMENTATION_EXPORT EventObject : public itk::AnyEvent
		{
		public:
			using Self = EventObject;
			using Superclass = itk::AnyEvent;
			EventObject(void* pData = nullptr, std::string  comment = "");

			~EventObject() override;

			void* getData() const;

			const std::string& getComment() const;

			const char* GetEventName() const override;

			bool CheckEvent(const ::itk::EventObject* e) const override;

			::itk::EventObject* MakeObject() const override;

			EventObject(const Self& s);

			void Print(std::ostream& os) const override;

		private:
			void* _pData;
			std::string _comment;
			void operator=(const Self&) = delete;
		};

		/*! @def mapEventMacro
		* Helper macro that creates events objects for map.
		*/
#define maEventMacro( classname , super, export_tag ) \
	/*! @ingroup Events */  \
	class export_tag classname : public super { \
	public: \
		typedef classname Self; \
		typedef super Superclass; \
		classname(void* pData = NULL, const std::string& comment = ""): Superclass(pData, comment) {}\
		virtual ~classname() {} \
		virtual const char * GetEventName() const { return #classname; } \
		virtual bool CheckEvent(const ::itk::EventObject* e) const \
		{ return dynamic_cast<const Self*>(e); } \
		virtual ::itk::EventObject* MakeObject() const \
		{ return new Self(*this); } \
		classname(const Self& s) : Superclass(s) {}; \
	private: \
		void operator=(const Self&); \
	}
	}  // namespace core

	namespace events
	{
		/*!@class AnyMatchPointEvent
		 * @brief General event that can be used if you want to observe any event defined by MatchPoint.
		 * @ingroup Events
		 */
		using AnyMatchPointEvent = core::EventObject;

		/*!@class ServiceEvent
		 * @brief General/base event concering a service strukture (e.g. ServiceStacks) or its processing.
		 * @ingroup Events
		 */
		maEventMacro(ServiceEvent, AnyMatchPointEvent, MITKSEGMENTATION_EXPORT);

		/*!@class KernelEvent
		 * @brief General/base event concering a registration kernel.
		 * @ingroup Events
		 */
		maEventMacro(KernelEvent, AnyMatchPointEvent, MITKSEGMENTATION_EXPORT);

		/*!@class KernelFieldComputationEvent
		 * @brief Event concering the processing of a registration kernel.
		 * @ingroup Events
		 */
		maEventMacro(KernelFieldComputationEvent, KernelEvent, MITKSEGMENTATION_EXPORT);

		/*!@class RegistrationEvent
		 * @brief Event concering the registration of data.
		 * @ingroup Events
		 */
		maEventMacro(RegistrationEvent, AnyMatchPointEvent, MITKSEGMENTATION_EXPORT);

		/*!@class TaskBatchEvent
		 * @brief General/base event concering a task batch (e.g. MappingTaskBatch) or its processing.
		 * @ingroup Events
		 */
		maEventMacro(TaskBatchEvent, AnyMatchPointEvent, MITKSEGMENTATION_EXPORT);
	}  // namespace events
}  // namespace map

#endif
