#include "Vector3.h"
#include "Quaternion.h"

using namespace realtime_srv;


Quaternion Vector3::ToQuaternion() const {
	const float DEG_TO_RAD = PI / ( 180.f );
	float x = X*DEG_TO_RAD;
	float y = Y*DEG_TO_RAD;
	float z = Z*DEG_TO_RAD;


	float cx = ( float )cos( x * 0.5 );
	float cy = ( float )cos( y * 0.5 );
	float cz = ( float )cos( z * 0.5 );
	float sx = ( float )sin( x * 0.5 );
	float sy = ( float )sin( y * 0.5 );
	float sz = ( float )sin( z * 0.5 );
	Quaternion q;

	//q.X = cx * sy * sz + cy * cz * sx;
	//q.Y = cx * cz * sy - cy * sx * sz;
	//q.Z = cx * cy * sz - cz * sx * sy;
	//q.W = sx * sy * sz + cx * cy * cz;

	//   ≈‰UE4
	q.X = cz*sx*sy - sz*cx*cy;
	q.Y = -cz*sx*cy - sz*cx*sy;
	q.Z = cz*cx*sy - sz*sx*cy;
	q.W = cz*cx*cy + sz*sx*sy;

	return q;
}

