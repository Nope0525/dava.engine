// This code contains NVIDIA Confidential Information and is disclosed to you
// under a form of NVIDIA software license agreement provided separately to you.
//
// Notice
// NVIDIA Corporation and its licensors retain all intellectual property and
// proprietary rights in and to this software and related documentation and
// any modifications thereto. Any use, reproduction, disclosure, or
// distribution of this software and related documentation without an express
// license agreement from NVIDIA Corporation is strictly prohibited.
//
// ALL NVIDIA DESIGN SPECIFICATIONS, CODE ARE PROVIDED "AS IS.". NVIDIA MAKES
// NO WARRANTIES, EXPRESSED, IMPLIED, STATUTORY, OR OTHERWISE WITH RESPECT TO
// THE MATERIALS, AND EXPRESSLY DISCLAIMS ALL IMPLIED WARRANTIES OF NONINFRINGEMENT,
// MERCHANTABILITY, AND FITNESS FOR A PARTICULAR PURPOSE.
//
// Information and code furnished is believed to be accurate and reliable.
// However, NVIDIA Corporation assumes no responsibility for the consequences of use of such
// information or for any infringement of patents or other rights of third parties that may
// result from its use. No license is granted by implication or otherwise under any patent
// or patent rights of NVIDIA Corporation. Details are subject to change without notice.
// This code supersedes and replaces all information previously supplied.
// NVIDIA Corporation products are not authorized for use as critical
// components in life support devices or systems without express written approval of
// NVIDIA Corporation.
//
// Copyright (c) 2008-2017 NVIDIA Corporation. All rights reserved.
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  

#ifndef PX_VEHICLE_4WDRIVE_H
#define PX_VEHICLE_4WDRIVE_H
/** \addtogroup vehicle
  @{
*/

#include "vehicle/PxVehicleDrive.h"
#include "vehicle/PxVehicleWheels.h"
#include "vehicle/PxVehicleComponents.h"


#if !PX_DOXYGEN
namespace physx
{
#endif

struct PxFilterData;
class PxGeometry;
class PxPhysics;
class PxBatchQuery;
class PxVehicleDrivableSurfaceToTireFrictionPairs;
class PxShape;
class PxMaterial;
class PxRigidDynamic;

/**
\brief Data structure describing the drive model components of a vehicle with up to 4 driven wheels and up to 16 un-driven wheels.
The drive model incorporates engine, clutch, gears, autobox, differential, and Ackermann steer correction.
@see PxVehicleDriveSimData
*/
class PxVehicleDriveSimData4W : public PxVehicleDriveSimData
{
//= ATTENTION! =====================================================================================
// Changing the data layout of this class breaks the binary serialization format.  See comments for 
// PX_BINARY_SERIAL_VERSION.  If a modification is required, please adjust the getBinaryMetaData 
// function.  If the modification is made on a custom branch, please change PX_BINARY_SERIAL_VERSION
// accordingly.
//==================================================================================================
public:

	friend class PxVehicleDrive4W;

	PxVehicleDriveSimData4W() 
		: PxVehicleDriveSimData()
	{
	}

	/**
	\brief Return the data describing the differential.
	@see PxVehicleDifferential4WData
	*/
	PX_FORCE_INLINE const PxVehicleDifferential4WData& getDiffData() const 
	{
		return mDiff;
	}

	/**
	\brief Return the data describing the Ackermann steer-correction.
	@see PxVehicleAckermannGeometryData
	*/
	PX_FORCE_INLINE const PxVehicleAckermannGeometryData& getAckermannGeometryData() const 
	{
		return mAckermannGeometry;
	}

	/**
	\brief Set the data describing the differential.
	@see PxVehicleDifferential4WData
	*/
	void setDiffData(const PxVehicleDifferential4WData& diff);

	/**
	\brief Set the data describing the Ackermann steer-correction.
	@see PxVehicleAckermannGeometryData
	*/
	void setAckermannGeometryData(const PxVehicleAckermannGeometryData& ackermannData);

private:

	/**
	\brief Differential simulation data
	@see setDiffData, getDiffData
	*/
	PxVehicleDifferential4WData		mDiff;

	/**
	\brief Data for ackermann steer angle computation.
	@see setAckermannGeometryData, getAckermannGeometryData
	*/
	PxVehicleAckermannGeometryData	mAckermannGeometry;

	/**
	\brief Test if the 4W-drive simulation data has been setup with legal data.
	\note Call only after setting all components.
	@see setEnginedata, setClutchData, setGearsData, setAutoboxData, setDiffData, setAckermannGeometryData 
	*/
	bool isValid() const;

//serialization
public:
	PxVehicleDriveSimData4W(const PxEMPTY) : PxVehicleDriveSimData(PxEmpty), mDiff(PxEmpty), mAckermannGeometry(PxEmpty)  {}
	static void getBinaryMetaData(PxOutputStream& stream);
//~serialization
};
PX_COMPILE_TIME_ASSERT(0==(sizeof(PxVehicleDriveSimData4W) & 15));



/**
\brief The ordering of the driven and steered wheels of a PxVehicleDrive4W.

@see PxVehicleWheelsSimData, PxVehicleWheelsDynData
*/

struct PxVehicleDrive4WWheelOrder
{
	enum Enum
	{
		eFRONT_LEFT=0,
		eFRONT_RIGHT,
		eREAR_LEFT,
		eREAR_RIGHT
	};
};

/**
\brief The control inputs for a PxVehicleDrive4W.

@see PxVehicleDriveDynData::setAnalogInput, PxVehicleDriveDynData::getAnalogInput
*/

struct PxVehicleDrive4WControl
{
	enum Enum
	{
		eANALOG_INPUT_ACCEL=0,
		eANALOG_INPUT_BRAKE,		
		eANALOG_INPUT_HANDBRAKE,	
		eANALOG_INPUT_STEER_LEFT,	
		eANALOG_INPUT_STEER_RIGHT,	
		eMAX_NB_DRIVE4W_ANALOG_INPUTS
	};
};

/**
\brief Data structure with instanced dynamics data and configuration data of a vehicle with up to 4 driven wheels and up to 16 non-driven wheels.
*/
class PxVehicleDrive4W : public PxVehicleDrive
{
//= ATTENTION! =====================================================================================
// Changing the data layout of this class breaks the binary serialization format.  See comments for 
// PX_BINARY_SERIAL_VERSION.  If a modification is required, please adjust the getBinaryMetaData 
// function.  If the modification is made on a custom branch, please change PX_BINARY_SERIAL_VERSION
// accordingly.
//==================================================================================================
public:
	friend class PxVehicleUpdate;
		
	/**
	\brief Allocate a PxVehicleDrive4W instance for a 4WDrive vehicle with nbWheels (= 4 + number of un-driven wheels)

	\param[in] nbWheels is the number of vehicle wheels  (= 4 + number of un-driven wheels)

	\return The instantiated vehicle.

	@see free, setup
	*/
	static PxVehicleDrive4W* allocate(const PxU32 nbWheels);

	/**
	\brief Deallocate a PxVehicleDrive4W instance.
	@see allocate
	*/
	void free();

	/**
	\brief Set up a vehicle using simulation data for the wheels and drive model.
	\param[in] physics is a PxPhysics instance that is needed to create special vehicle constraints that are maintained by the vehicle.
	\param[in] vehActor is a PxRigidDynamic instance that is used to represent the vehicle in the PhysX SDK.
	\param[in] wheelsData describes the configuration of all suspension/tires/wheels of the vehicle. The vehicle instance takes a copy of this data.
	\param[in] driveData describes the properties of the vehicle's drive model (gears/engine/clutch/differential/autobox).  The vehicle instance takes a copy of this data.
	\param[in] nbNonDrivenWheels is the number of wheels on the vehicle that cannot be connected to the differential (= numWheels - 4).
	\note It is assumed that the first shapes of the actor are the wheel shapes, followed by the chassis shapes.  To break this assumption use PxVehicleWheelsSimData::setWheelShapeMapping.
	\note wheelsData must contain data for at least 4 wheels.  Unwanted wheels can be disabled with PxVehicleWheelsSimData::disableWheel after calling setup.
	@see allocate, free, setToRestState, PxVehicleWheelsSimData::setWheelShapeMapping
	*/
	void setup
		(PxPhysics* physics, PxRigidDynamic* vehActor,
		 const PxVehicleWheelsSimData& wheelsData, const PxVehicleDriveSimData4W& driveData,
		 const PxU32 nbNonDrivenWheels);

	/**
	\brief Allocate and set up a vehicle using simulation data for the wheels and drive model.
	\param[in] physics is a PxPhysics instance that is needed to create special vehicle constraints that are maintained by the vehicle.
	\param[in] vehActor is a PxRigidDynamic instance that is used to represent the vehicle in the PhysX SDK.
	\param[in] wheelsData describes the configuration of all suspension/tires/wheels of the vehicle. The vehicle instance takes a copy of this data.
	\param[in] driveData describes the properties of the vehicle's drive model (gears/engine/clutch/differential/autobox).  The vehicle instance takes a copy of this data.
	\param[in] nbNonDrivenWheels is the number of wheels on the vehicle that cannot be connected to the differential (= numWheels - 4).
	\note It is assumed that the first shapes of the actor are the wheel shapes, followed by the chassis shapes.  To break this assumption use PxVehicleWheelsSimData::setWheelShapeMapping.
	\note wheelsData must contain data for at least 4 wheels.  Unwanted wheels can be disabled with PxVehicleWheelsSimData::disableWheel after calling setup.
	\return The instantiated vehicle.
	@see allocate, free, setToRestState, PxVehicleWheelsSimData::setWheelShapeMapping
	*/
	static PxVehicleDrive4W* create
		(PxPhysics* physics, PxRigidDynamic* vehActor,
		 const PxVehicleWheelsSimData& wheelsData, const PxVehicleDriveSimData4W& driveData,
		 const PxU32 nbNonDrivenWheels);

	/**
	\brief Set a vehicle to its rest state.  Aside from the rigid body transform, this will set the vehicle and rigid body 
	to the state they were in immediately after setup or create.
	\note Calling setToRestState invalidates the cached raycast hit planes under each wheel meaning that suspension line
	raycasts need to be performed at least once with PxVehicleSuspensionRaycasts before calling PxVehicleUpdates. 
	@see setup, create, PxVehicleSuspensionRaycasts, PxVehicleUpdates
	*/
	void setToRestState();

	/**
	\brief Simulation data that describes the configuration of the vehicle's drive model.
	@see setup, create
	*/
	PxVehicleDriveSimData4W mDriveSimData;

private:

	/**
	\brief Test if the instanced dynamics and configuration data has legal values.
	*/
	bool isValid() const;	

//serialization
protected:
									PxVehicleDrive4W();
									~PxVehicleDrive4W(){}
	virtual		bool				isKindOf(const char* name)	const	{ return !::strcmp("PxVehicleDrive4W", name) || PxBase::isKindOf(name); }
public:
	static		PxVehicleDrive4W*	createObject(PxU8*& address, PxDeserializationContext& context);
	static		void				getBinaryMetaData(PxOutputStream& stream);
									PxVehicleDrive4W(PxBaseFlags baseFlags) : PxVehicleDrive(baseFlags), mDriveSimData(PxEmpty)	{}
	virtual		const char*			getConcreteTypeName() const			{ return "PxVehicleDrive4W";	}
//~serialization	
};
PX_COMPILE_TIME_ASSERT(0==(sizeof(PxVehicleDrive4W) & 15));

#if !PX_DOXYGEN
} // namespace physx
#endif

/** @} */
#endif //PX_VEHICLE_4WDRIVE_H
