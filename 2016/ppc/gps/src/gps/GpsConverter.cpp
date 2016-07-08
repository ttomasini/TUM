/**
 * \copyright
 * (c) 2012 - 2015 E.S.R. Labs GmbH (http://www.esrlabs.com)
 * All rights reserved.
 */

#include "gps/GpsConverter.h"
#include "ac/IGpsACPusher.h"
#include "can/canframes/CANFrame.h"
#include "can/ICANTransceiver.h"
#include <stdio.h>
#include <math.h>

using namespace can;

namespace gps {

GpsConverter::GpsConverter(ICANTransceiver& transceiver, IGpsACPusher& acPusher) :
		fCanTransceiver(transceiver), fCanFilter(GPS_FRAME_ID, GPS_FRAME_ID), fAcPusher(
				acPusher) {
}

void GpsConverter::frameReceived(const CANFrame& canFrame) {
	const uint8* payload = canFrame.getPayload();
	// TODO implement conversion to arc-msec and call IGpsACPusher

	sint32 latInMs = 0; // here add your converted lat
	sint32 longInMs = 0; // here add your converted long

	//Building the coordiantes out of the payload
	sint32 latitude = static_cast<sint32>(payload[4] | (payload[5] << 8)
			| (payload[6] << 16) | (payload[7] << 24));
	sint32 longitude = static_cast<sint32>(payload[0] | (payload[1] << 8)
			| (payload[2] << 16) | (payload[3] << 24));

	printf("lat raw: %d, long raw: %d\n", latitude, longitude);

	//check for valid Data
	if (!checkValid(latitude) && !checkValid(longitude)) {
		return;
	}

	//calculate raw to angle factor with scaling for precission
	const sint32 fak = 180 * 9000000000 / (pow(2, 31) - 1);
	//calculate descaling factor including the conversion to degree ms
	const sint32 divfak = 9000000000 / 3600000;
	printf("fak: %d\n", fak);

	//division first because of limited length of variable
	latInMs = latitude/divfak;
	longInMs = longitude/divfak;

	printf("lat: %d, long: %d\n", latInMs, longInMs);

	latInMs *= fak;
	longInMs *= fak;

	printf("lat: %d, long: %d\n", latInMs, longInMs);


	if (latInMs != fLastLatInMs || longInMs != fLastLongInMs) {
		// value changed
		fAcPusher.pushGPSCoordinates(latInMs, longInMs);
		fLastLatInMs = latInMs;
		fLastLongInMs = longInMs;
	}
}

bool GpsConverter::checkValid(sint32 value) {
	if (value == 0x7fffffff || value == 0xffffffff || value == 0x80000000) {
		return false;
	}
	return true;
}

} // namespace gps

