#include "Vector3.hpp"
#include "Quaternion.hpp"


Quaternion Vector3::ToQuaternion() const
{
	const float DEG_TO_RAD = PI / ( 180.f );
	double x = X*DEG_TO_RAD;
	double y = Y*DEG_TO_RAD;
	double z = Z*DEG_TO_RAD;


	double cx = cos( x * 0.5 );
	double cy = cos( y * 0.5 );
	double cz = cos( z * 0.5 );
	double sx = sin( x * 0.5 );
	double sy = sin( y * 0.5 );
	double sz = sin( z * 0.5 );
	Quaternion q;

	//q.X = cx * sy * sz + cy * cz * sx;
	//q.Y = cx * cz * sy - cy * sx * sz;
	//q.Z = cx * cy * sz - cz * sx * sy;
	//q.W = sx * sy * sz + cx * cy * cz;

	q.X = cz*sx*sy - sz*cx*cy;
	q.Y = -cz*sx*cy - sz*cx*sy;
	q.Z = cz*cx*sy - sz*sx*cy;
	q.W = cz*cx*cy + sz*sx*sy;

	return q;
}

