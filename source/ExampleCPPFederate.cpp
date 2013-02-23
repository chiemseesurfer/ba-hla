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
#include <memory.h>
#include "ExampleFedAmb.h"
#include "ExampleCPPFederate.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <RTI/RTIambassadorFactory.h>
#include <RTI/RTIambassador.h>
#include <RTI/RTI1516fedTime.h>
#include <RTI/LogicalTimeInterval.h>

using namespace std;
using namespace rti1516;

ExampleCPPFederate::ExampleCPPFederate()
{
}

ExampleCPPFederate::~ExampleCPPFederate()
{
	if( this->fedamb )
		delete this->fedamb;
}

/** 
 * convert normal C++-Strings to C++-Wstrings 
 * Wstrings are used by openrti
 */
std::wstring 
ExampleCPPFederate::convertStringToWstring(const std::string& str) 
{ 
    const std::ctype<wchar_t>& CType = std::use_facet<std::ctype<wchar_t> >(std::locale()); 
    std::vector<wchar_t> wideStringBuffer(str.length()); 
    CType.widen(str.data(), str.data() + str.length(), &wideStringBuffer[0]); 
    return std::wstring(&wideStringBuffer[0], wideStringBuffer.size()); 
}

/**
 * Everything will be handled here
 *
 * @param federateName How the federate is called in simulation
 * @return void
 */
void ExampleCPPFederate::runFederate( std::string federateName, std::string fom, std::string address )
{
	///
	/// 1. create the RTIambassador
	///
	rti1516::RTIambassadorFactory* factory = new rti1516::RTIambassadorFactory();
	std::vector<std::wstring> args; //!< vector args contains all necessary rti parameters
	args.push_back(L"protocol=rti");
	args.push_back(L"address=" + convertStringToWstring(address));
	
	this->rtiamb = factory->createRTIambassador(args);

	///
	/// 2. create and join to the federation
	/// NOTE: some other federate may have already created the federation,
	///       in that case, we'll just try and join it
	///
	try
	{
		rtiamb->createFederationExecution( L"ExampleFederation",
        convertStringToWstring(fom));
		cout << "Created Federation" << endl;
	}
	catch( FederationExecutionAlreadyExists exists )
	{
		cout << "Didn't create federation, it already existed" << endl;
	}

	/////////////////////////////
	/// 3. join the federation
	/////////////////////////////
	/// create the federate ambassador and join the federation
	this->fedamb = new ExampleFedAmb();
	rtiamb->joinFederationExecution( convertStringToWstring(federateName),
    convertStringToWstring("ExampleFederation"), *fedamb );
	cout << "Joined Federation as " << federateName << endl;

	/// initialize the handles - have to wait until we are joined
	initializeHandles();

	/////////////////////////////////
	/// 4. announce the sync point
	/////////////////////////////////
	/// announce a sync point to get everyone on the same page. if the point
	/// has already been registered, we'll get a callback saying it failed,
	/// but we don't care about that, as long as someone registered it
	rtiamb->registerFederationSynchronizationPoint(
    convertStringToWstring(READY_TO_RUN), toVariableLengthData("") );
    cout << "SynchronizationPoint registered" << endl;
	while( fedamb->isAnnounced == false )
	{
        rtiamb->evokeCallback(12.0);
	}

	/// WAIT FOR USER TO KICK US OFF.\n
	/// So that there is time to add other federates, we will wait until the
	/// user hits enter before proceeding. That was, you have time to start
	/// other federates.
	waitForUser();

	////////////////////////////////////////////////////////
	/// 5. achieve the point and wait for synchronization
	////////////////////////////////////////////////////////
	/// tell the RTI we are ready to move past the sync point and then wait
	/// until the federation has synchronized on
	rtiamb->synchronizationPointAchieved( convertStringToWstring(READY_TO_RUN) );
	wcout << L"Achieved sync point: " << READY_TO_RUN << L", waiting for federation..." << endl;
	while( fedamb->isReadyToRun == false )
	{
        rtiamb->evokeCallback(12.0);
	}

	/////////////////////////////
	// 6. enable time policies
	/////////////////////////////
	// in this section we enable/disable all time policies
	// note that this step is optional!
	enableTimePolicy();
	cout << "Time Policy Enabled" << endl;

	//////////////////////////////
	// 7. publish and subscribe
	//////////////////////////////
	// in this section we tell the RTI of all the data we are going to
	// produce, and all the data we want to know about
	publishAndSubscribe();
	cout << "Published and Subscribed" << endl;

	/////////////////////////////////////
	// 8. register an object to update
	/////////////////////////////////////
	ObjectInstanceHandle objectHandle = registerObject();
	wcout << L"Registered Object, handle=" << objectHandle << endl;

	////////////////////////////////////
	// 9. do the main simulation loop
	////////////////////////////////////
	// here is where we do the meat of our work. in each iteration, we will
	// update the attribute values of the object we registered, and will
	// send an interaction.
	int i;
	for( i = 0; i < 20; i++ )
	{
		/// 9.1 update the attribute values of the instance
		updateAttributeValues( objectHandle );

		/// 9.2 send an interaction
		sendInteraction();

		/// 9.3 request a time advance and wait until we get it
		advanceTime( 1.0 );
		cout << "Time Advanced to " << fedamb->federateTime << endl;
	}

	//////////////////////////////////////
	// 10. delete the object we created
	//////////////////////////////////////
	deleteObject( objectHandle );
	wcout << "Deleted Object, handle=" << objectHandle << endl;

	////////////////////////////////////
	// 11. resign from the federation
	////////////////////////////////////
	rtiamb->resignFederationExecution( NO_ACTION );
	cout << "Resigned from Federation" << endl;

	////////////////////////////////////////
	// 12. try and destroy the federation
	////////////////////////////////////////
	// NOTE: we won't die if we can't do this because other federates
	//       remain. in that case we'll leave it for them to clean up
	try
	{
		rtiamb->destroyFederationExecution( L"ExampleFederation" );
		cout << "Destroyed Federation" << endl;
	}
	catch( FederationExecutionDoesNotExist dne )
	{
		cout << "No need to destroy federation, it doesn't exist" << endl;
	}
	catch( FederatesCurrentlyJoined fcj )
	{
		cout << "Didn't destroy federation, federates still joined" << endl;
	}
}

///////////////////////////////////////////////////////////////////////////////
//////////////////////////////// Helper Methods ///////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/**
 * This method will get all the relevant handle information from the RTIambassador
 */
void ExampleCPPFederate::initializeHandles()
{
	this->aHandle  = rtiamb->getObjectClassHandle( L"HLAobjectRoot.A" );
	this->aaHandle = rtiamb->getAttributeHandle( this->aHandle, L"aa" );
	this->abHandle = rtiamb->getAttributeHandle( this->aHandle, L"ab" );
	this->acHandle = rtiamb->getAttributeHandle( this->aHandle, L"ac" );

	this->xHandle  = rtiamb->getInteractionClassHandle( L"HLAinteractionRoot.X" );
	this->xaHandle = rtiamb->getParameterHandle( this->xHandle,
    convertStringToWstring("xa") );
	this->xbHandle = rtiamb->getParameterHandle( this->xHandle,
    convertStringToWstring("xb") );
}

/**
 * Blocks until the user hits enter
 */
void ExampleCPPFederate::waitForUser()
{
	cout << " >>>>>>>>>> Press Enter to Continue <<<<<<<<<<" << endl;
	string line;
	getline( cin, line );
}

/**
 * This method will attempt to enable the various time related properties for
 * the federate
 */
void ExampleCPPFederate::enableTimePolicy()
{
	/////////////////////////////
	/// enable time regulation
	/////////////////////////////
    HLAfloat64Interval lookahead( fedamb->federateLookahead );
	rtiamb->enableTimeRegulation( lookahead );

    /// wait for callback
	while( fedamb->isRegulating == false )
	{
        rtiamb->evokeCallback(12.0);
	}

	/////////////////////////////
	/// enable time constrained
	/////////////////////////////
	rtiamb->enableTimeConstrained();

	/// wait for callback
	while( fedamb->isConstrained == false )
	{
        rtiamb->evokeCallback(12.0);
	}
}

/**
 * This method will inform the RTI about the types of data that the federate will
 * be creating, and the types of data we are interested in hearing about as other
 * federates produce it.
 */
void ExampleCPPFederate::publishAndSubscribe()
{
	/////////////////////////////////////////////
	/// publish all attributes of ObjectRoot.A
	/////////////////////////////////////////////
	/// before we can register instance of the object class ObjectRoot.A and
	/// update the values of the various attributes, we need to tell the RTI
	/// that we intend to publish this information\n

	/// package the information into a handle set
	AttributeHandleSet attributes;
	attributes.insert( this->aaHandle );
	attributes.insert( this->abHandle );
	attributes.insert( this->acHandle );

	/// do the actual publication
	rtiamb->publishObjectClassAttributes( this->aHandle, attributes );

	//////////////////////////////////////////////////
	/// subscribe to all attributes of ObjectRoot.A
	//////////////////////////////////////////////////
	/// we also want to hear about the same sort of information as it is
	/// created and altered in other federates, so we need to subscribe to it
	rtiamb->subscribeObjectClassAttributes( this->aHandle, attributes, true);

	//////////////////////////////////////////////////////
	/// publish the interaction class InteractionRoot.X
	//////////////////////////////////////////////////////
	/// we want to send interactions of type InteractionRoot.X, so we need
	/// to tell the RTI that we're publishing it first. We don't need to
	/// inform it of the parameters, only the class, making it much simpler

	/// do the publication
	rtiamb->publishInteractionClass( this->xHandle );

	/////////////////////////////////////////////////////
	/// subscribe to the InteractionRoot.X interaction //
	/////////////////////////////////////////////////////
	/// we also want to receive other interaction of the same type that are
	/// sent out by other federates, so we have to subscribe to it first
	rtiamb->subscribeInteractionClass( this->xHandle );
}

/**
 * This method will register an instance of the class ObjectRoot.A and will
 * return the federation-wide unique handle for that instance. Later in the
 * simulation, we will update the attribute values for this instance
 */
ObjectInstanceHandle ExampleCPPFederate::registerObject()
{
	return rtiamb->registerObjectInstance( rtiamb->getObjectClassHandle(L"HLAobjectRoot.A") );
}

/**
 * something like a serialize function.\n
 * The RTI does not know everything about datatypes. It just knows a byte
 * format. To get this openrti offers VariableLengthData.
 */
rti1516::VariableLengthData
ExampleCPPFederate::toVariableLengthData(const char* s){
    rti1516::VariableLengthData variableLengthData;
    if (s)
        variableLengthData.setData(s, strlen(s));
    return variableLengthData;
}

/**
 * This method will update all the values of the given object instance. It will
 * set each of the values to be a string which is equal to the name of the
 * attribute plus the current time. eg "aa:10.0" if the time is 10.0.
 * <p/>
 * Note that we don't actually have to update all the attributes at once, we
 * could update them individually, in groups or not at all!
 */
void ExampleCPPFederate::updateAttributeValues( ObjectInstanceHandle objectHandle )
{
	////////////////////////////////////////////////
	/// create the necessary container and values
	///////////////////////////////////////////////
	/// create the collection to store the values in, as you can see
	/// this is quite a lot of work
	AttributeHandleValueMap attributes;

	/// generate the new values
	char aaValue[16], abValue[16], acValue[16];
	sprintf( aaValue, "aa:%f", getLbts() );
    attributes[aaHandle] = toVariableLengthData(aaValue);
    attributes[abHandle] = toVariableLengthData(abValue);
    attributes[acHandle] = toVariableLengthData(acValue);

	///////////////////////////
	/// do the actual update
	///////////////////////////
	rtiamb->updateAttributeValues( objectHandle, attributes,
    toVariableLengthData("hi!") );

	/// note that if you want to associate a particular timestamp with the
	/// update. here we send another update, this time with a timestamp:
	RTI1516fedTime time = fedamb->federateTime + fedamb->federateLookahead;
	rtiamb->updateAttributeValues( objectHandle, attributes,
    toVariableLengthData("hi!"), time );
}

/**
 * This method will send out an interaction of the type InteractionRoot.X. Any
 * federates which are subscribed to it will receive a notification the next time
 * they tick(). Here we are passing only two of the three parameters we could be
 * passing, but we don't actually have to pass any at all!
 */
void ExampleCPPFederate::sendInteraction()
{
	////////////////////////////////////////////////
	/// create the necessary container and values
	////////////////////////////////////////////////
	/// create the collection to store the values in
	ParameterHandleValueMap parameters;

	/// generate the new values
	char xaValue[16], xbValue[16];
	sprintf( xaValue, "xa:%f", getLbts() );
    parameters[xaHandle] = toVariableLengthData(xaValue);

	///////////////////////////
	/// send the interaction
	///////////////////////////
	rtiamb->sendInteraction( xHandle, parameters, toVariableLengthData("hi!") );

	/// if you want to associate a particular timestamp with the
	/// interaction, you will have to supply it to the RTI. Here
	/// we send another interaction, this time with a timestamp:
	RTI1516fedTime time = fedamb->federateTime + fedamb->federateLookahead;
	rtiamb->sendInteraction( xHandle, parameters, toVariableLengthData("hi!"), time );
}

/**
 * This method will request a time advance to the current time, plus the given
 * timestep. It will then wait until a notification of the time advance grant
 * has been received.
 */
void ExampleCPPFederate::advanceTime( double timestep )
{
	/// request the advance
	fedamb->isAdvancing = true;
	RTI1516fedTime newTime = (fedamb->federateTime + timestep);
	rtiamb->timeAdvanceRequest( newTime );

	/// wait for the time advance to be granted. ticking will tell the
	/// LRC to start delivering callbacks to the federate
	while( fedamb->isAdvancing )
	{
        rtiamb->evokeCallback(12.0);
	}
}

/**
 * This method will attempt to delete the object instance of the given
 * handle. We can only delete objects we created, or for which we own the
 * privilegeToDelete attribute.
 */
void ExampleCPPFederate::deleteObject( ObjectInstanceHandle objectHandle )
{
	rtiamb->deleteObjectInstance( objectHandle, toVariableLengthData(""));
}

double ExampleCPPFederate::getLbts()
{
	return (fedamb->federateTime + fedamb->federateLookahead);
}
