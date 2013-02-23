/**
 * @file
 *
 * Copyright (C) 2013 Max Oberberger <max@oberbergers.de>
 *
 * Last modified: 2013 February 23, 20:05:02 by max
 *
 * This file is part of ba-hla.
 * 
 * ba-hla is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * ba-hla is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ba-hla.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <iostream>
#include "ExampleFedAmb.h"
#include <string.h>
#include <RTI/RTI1516fedTime.h>

using namespace std;
using namespace rti1516;

/**
 * NOTE: The main part of the functions in this file is to print something on stdout
 * to see whether the function is called or not.
 */
ExampleFedAmb::ExampleFedAmb()
{
	// initialize all the variable values
	this->federateTime      = 0.0;
	this->federateLookahead = 1.0;

	this->isRegulating  = false;
	this->isConstrained = false;
	this->isAdvancing   = false;
	this->isAnnounced   = false;
	this->isReadyToRun  = false;
}

ExampleFedAmb::~ExampleFedAmb()
	throw( )
{
}

/**
 * convert Logical System time to Federate time which can be understood by RTI
 */
double ExampleFedAmb::convertTime( LogicalTime const &theTime )
{
	RTI1516fedTime castedTime = (RTI1516fedTime)theTime;
	return castedTime.getFedTime();
}

///////////////////////////////////////////////////////////////////////////////
/////////////////////// Synchronization Point Callbacks ///////////////////////
///////////////////////////////////////////////////////////////////////////////
void ExampleFedAmb::synchronizationPointRegistrationSucceeded( std::wstring const &label )
	throw(FederateInternalError)
{
	wcout << L"Successfully registered sync point: " << label << endl;
}

void ExampleFedAmb::synchronizationPointRegistrationFailed( std::wstring const &label,  SynchronizationFailureReason reason )
	throw(FederateInternalError)
{
	wcout << L"Failed to register sync point: " << label << endl;
}

void ExampleFedAmb::announceSynchronizationPoint( std::wstring const &label, VariableLengthData const &theUserSuppliedTag )
	throw(FederateInternalError)
{
	wcout << L"Synchronization point announced: " << label << endl;
	std::wstring compair = L"ReadyToRun";
	if( wcscmp(label.c_str(), compair.c_str()) == 0 )
		this->isAnnounced = true;
}

void ExampleFedAmb::federationSynchronized( std::wstring const &label )
	throw(FederateInternalError)
{
	wcout << L"Federation Synchronized: " << label << endl;
	std::wstring compair = L"ReadyToRun";
	if( wcscmp(label.c_str(), compair.c_str()) == 0 )
		this->isReadyToRun = true;
}

///////////////////////////////////////////////////////////////////////////////
//////////////////////////////// Time Callbacks ///////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ExampleFedAmb::timeRegulationEnabled( LogicalTime const &theFederateTime) 
			throw ( InvalidLogicalTime, 
					NoRequestToEnableTimeRegulationWasPending, 
					FederateInternalError )
{
	this->isRegulating = true;
	this->federateTime = convertTime( theFederateTime );
}

void ExampleFedAmb::timeConstrainedEnabled( LogicalTime const &theFederateTime ) 
			throw ( InvalidLogicalTime, 
					NoRequestToEnableTimeConstrainedWasPending, 
					FederateInternalError)
{
	this->isConstrained = true;
	this->federateTime = convertTime( theFederateTime );
}

void ExampleFedAmb::timeAdvanceGrant( LogicalTime const &theTime) 
			throw ( InvalidLogicalTime, 
					JoinedFederateIsNotInTimeAdvancingState, 
					FederateInternalError)
{
	this->isAdvancing = false;
	this->federateTime = convertTime( theTime );
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////// Object Management Callbacks /////////////////////////
///////////////////////////////////////////////////////////////////////////////

//                         //
// Discover Object Methods //
//                         //
void ExampleFedAmb::discoverObjectInstance( ObjectInstanceHandle theObject, 
											  ObjectClassHandle theObjectClass, 
											  std::wstring const &theObjectInstanceName) 
			throw ( CouldNotDiscover, 
					ObjectClassNotKnown, 
					FederateInternalError )
{
	wcout << L"Discoverd Object: handle=" << theObject
	     << L", classHandle=" << theObjectClass
	     << L", name=" << theObjectInstanceName << endl;
}

//                                 // 
// Reflect Attribute Value Methods //
//                                 // 
void ExampleFedAmb::reflectAttributeValues( ObjectInstanceHandle theObject, 
											 AttributeHandleValueMap const &theAttributeValues, 
											 VariableLengthData const &theUserSuppliedTag, 
											 OrderType sentOrder, 
											 TransportationType theType) 
			throw ( ObjectInstanceNotKnown, 
					AttributeNotRecognized, 
					AttributeNotSubscribed, 
					FederateInternalError )
{
	wcout << L"Reflection Received:" << endl;
}

void ExampleFedAmb::reflectAttributeValues( ObjectInstanceHandle theObject, 
											 AttributeHandleValueMap const &theAttributeValues, 
											 VariableLengthData const &theUserSuppliedTag, 
											 OrderType sentOrder, 
											 TransportationType theType, 
											 RegionHandleSet const &theSentRegionHandleSet) 
			throw ( ObjectInstanceNotKnown, 
					AttributeNotRecognized, 
					AttributeNotSubscribed, 
					FederateInternalError )
{
	wcout << L"Reflection Received:"<< endl;
}

/**
 * convert variableLengthData to Wstring (deserialize)\n
 * for serialize function see ExampleCPPFederate class
 */
std::wstring
ExampleFedAmb::variableLengthDataToWstring(const rti1516::VariableLengthData&
variableLengthData)
{
    if (!variableLengthData.size())
        return std::wstring();
    return std::wstring((const wchar_t*) variableLengthData.data(),
    variableLengthData.size() / sizeof(std::wstring::value_type));
}

/**
 * receiveInteraction with some detailed output
 */
void ExampleFedAmb::receiveInteraction( InteractionClassHandle theInteraction, ParameterHandleValueMap const &theParameterValues, VariableLengthData const &theUserSuppliedTag, OrderType sentOrder, TransportationType theType) throw (InteractionClassNotRecognized, InteractionParameterNotRecognized, InteractionClassNotSubscribed, FederateInternalError )
{
	wcout << L"Interaction Received:";
	
	/// print the handle
	wcout << " handle=" << theInteraction;
	/// print the tag
	wcout << ", tag=" << variableLengthDataToWstring(theUserSuppliedTag);
	
	/// print the attribute information
	wcout << ", parameterCount=" << theParameterValues.size();
    for(ParameterHandleValueMap::const_iterator i = theParameterValues.begin();
    i!= theParameterValues.end(); i++)
	{
		/// print the parameter handle
		wcout << "\tparamHandle=" << i->first;
		/// print the parameter value
		std::wstring value = variableLengthDataToWstring(i->second);
				
		wcout << ", paramValue=" << value << endl;
	}
}

void ExampleFedAmb::receiveInteraction( InteractionClassHandle theInteraction, 
        ParameterHandleValueMap const &theParameterValues, 
        VariableLengthData const &theUserSuppliedTag, 
        OrderType sentOrder, 
        TransportationType theType, 
        RegionHandleSet const &theSentRegionHandleSet) 
    throw (InteractionClassNotRecognized, 
        InteractionParameterNotRecognized, 
        InteractionClassNotSubscribed, 
        FederateInternalError )
{
	wcout << L"Interaction Received:" << endl;
}
void ExampleFedAmb::receiveInteraction ( InteractionClassHandle theInteraction, 
        ParameterHandleValueMap const &theParameterValues, 
        VariableLengthData const &theUserSuppliedTag, 
        OrderType sentOrder, 
        TransportationType theType, 
        LogicalTime const &theTime, 
        OrderType receivedOrder) 
    throw (InteractionClassNotRecognized, 
        InteractionParameterNotRecognized, 
        InteractionClassNotSubscribed, 
        FederateInternalError)
{
	wcout << L"Interaction Received:" << endl;
}
void ExampleFedAmb::receiveInteraction ( InteractionClassHandle theInteraction, 
        ParameterHandleValueMap const &theParameterValues, 
        VariableLengthData const &theUserSuppliedTag, 
        OrderType sentOrder, 
        TransportationType theType, 
        LogicalTime const &theTime, 
        OrderType receivedOrder, 
        RegionHandleSet const &theSentRegionHandleSet) 
    throw (InteractionClassNotRecognized, 
        InteractionParameterNotRecognized, 
        InteractionClassNotSubscribed, 
        FederateInternalError)
{
	wcout << L"Interaction Received:" << endl;
}

void ExampleFedAmb::receiveInteraction (InteractionClassHandle theInteraction, 
        ParameterHandleValueMap const &theParameterValues, 
        VariableLengthData const &theUserSuppliedTag, 
        OrderType sentOrder, 
        TransportationType theType, 
        LogicalTime const &theTime, 
        OrderType receivedOrder, 
        MessageRetractionHandle theHandle) 
    throw (InteractionClassNotRecognized, 
        InteractionParameterNotRecognized, 
        InteractionClassNotSubscribed, 
        InvalidLogicalTime, 
        FederateInternalError)
{
	wcout << L"Interaction Received:" << endl;
}

void ExampleFedAmb::receiveInteraction (InteractionClassHandle theInteraction, 
        ParameterHandleValueMap const &theParameterValues, 
        VariableLengthData const &theUserSuppliedTag, 
        OrderType sentOrder, 
        TransportationType theType, 
        LogicalTime const &theTime, 
        OrderType receivedOrder, 
        MessageRetractionHandle theHandle, 
        RegionHandleSet const &theSentRegionHandleSet) 
    throw (InteractionClassNotRecognized, 
        InteractionParameterNotRecognized, 
        InteractionClassNotSubscribed, 
        InvalidLogicalTime, 
        FederateInternalError)
{
	wcout << L"Interaction Received:" << endl;
}

//                       //
// Remove Object Methods //
//                       //
void ExampleFedAmb::removeObjectInstance( ObjectInstanceHandle theObject, 
		VariableLengthData const &theUserSuppliedTag, 
		OrderType sentOrder) 
	throw (ObjectInstanceNotKnown, FederateInternalError )
{
	wcout << L"Object Removed: handle=" << theObject << endl;
}

void ExampleFedAmb::removeObjectInstance( ObjectInstanceHandle theObject, 
		VariableLengthData const &theUserSuppliedTag, 
		OrderType sentOrder, 
		LogicalTime const &theTime, 
		OrderType receivedOrder) 
	throw (ObjectInstanceNotKnown, FederateInternalError )
{
	wcout << L"Object Removed: handle=" << theObject << endl;
}

void ExampleFedAmb::removeObjectInstance (ObjectInstanceHandle theObject, 
		VariableLengthData const &theUserSuppliedTag, 
		OrderType sentOrder, 
		LogicalTime const &theTime, 
		OrderType receivedOrder, 
		MessageRetractionHandle theHandle) 
	throw (ObjectInstanceNotKnown, 
	InvalidLogicalTime, FederateInternalError)
{
	wcout << L"Object Removed: handle=" << theObject << endl;
}
