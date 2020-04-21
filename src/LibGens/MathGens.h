/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2014 Torus Knot Software Ltd

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-----------------------------------------------------------------------------
*/

//=========================================================================
//	Most of this code has been copied and modified from the Ogre3D Project.
//=========================================================================

#pragma once

#define LIBGENS_MATH_ERROR_MESSAGE_NULL_FILE        "Trying to read math data from unreferenced file."
#define LIBGENS_MATH_ERROR_MESSAGE_WRITE_NULL_FILE  "Trying to write math data to an unreferenced file."

#define LIBGENS_MATH_AXIS_X                      0
#define LIBGENS_MATH_AXIS_Y                      1
#define LIBGENS_MATH_AXIS_Z                      2
#define LIBGENS_MATH_AXIS_W                      3

#define LIBGENS_MATH_SIDE_LEFT                   0
#define LIBGENS_MATH_SIDE_RIGHT                  1

#define LIBGENS_AABB_MAX_START                   9999999
#define LIBGENS_AABB_MIN_END                    -9999999

#define LIBGENS_MATH_EPSILON_TRIANGLE            0.1f
#define LIBGENS_MATH_HALF_PI                     1.570796326f
#define LIBGENS_MATH_PI                          3.141592653f
#define LIBGENS_MATH_COLOR_CHAR                  255.0f
#define LIBGENS_MATH_EPSION_QUATERNION           1e-03

#define LIBGENS_MATH_INT32_TO_RAD                (0.5493164f / 36000.0f * LIBGENS_MATH_PI * 2)
#define LIBGENS_MATH_RAD_TO_DEGREE               180.0f / LIBGENS_MATH_PI

#define LIBGENS_MATH_AXIS_X_TEXT                 "x"
#define LIBGENS_MATH_AXIS_Y_TEXT                 "y"
#define LIBGENS_MATH_AXIS_Z_TEXT                 "z"
#define LIBGENS_MATH_AXIS_W_TEXT                 "w"

#define LIBGENS_MATH_GRAVITY					  35

namespace LibGens {
	class File;
	class Matrix3;
	class Matrix4;
	class Vector3;
	class Color8;
	class Color;

	class Vector2 {
		public:
			float x;
			float y;

			Vector2() : x(0), y(0) {
			}

			Vector2(float x_p, float y_p) : x(x_p), y(y_p) {
			}

			void read(File *file, bool big_endian=true);
			void readHalf(File *file, bool big_endian=true);
			void write(File *file, bool big_endian=true);

			inline bool operator == (const Vector2& vector) {
	            return (x == vector.x && y == vector.y);
			}

			inline bool operator != (const Vector2& vector) {
	            return (x != vector.x || y != vector.y);
			}
			
			inline Vector2 operator + (const Vector2& vector) {
				return Vector2(x+vector.x, y+vector.y);
			}

			inline Vector2 operator - (const Vector2& vector) {
				return Vector2(x-vector.x, y-vector.y);
			}

			inline Vector2 operator * (const float& mult) {
				return Vector2(x*mult, y*mult);
			}

			inline Vector2 operator / (const float& div) {
				return Vector2(x/div, y/div);
			}
	};


	class Vector3 {
		public:
			float x;
			float y;
			float z;

			Vector3() : x(0), y(0), z(0) {
			}

			Vector3(float x_p, float y_p, float z_p) : x(x_p), y(y_p), z(z_p) {
			}

			void read(File *file, bool big_endian=true);
			void readNormal360(File *file, bool big_endian=true);
			void write(File *file, bool big_endian=true);

			void readXML(TiXmlElement *root);
			void readSingleXML(TiXmlElement *root);
			void writeXML(TiXmlElement *root);

			inline bool operator == (const Vector3& vector) {
	            return ( x == vector.x && y == vector.y && z == vector.z );
			}

			inline bool operator != (const Vector3& vector) {
	            return (x != vector.x || y != vector.y || z != vector.z);
			}

			inline Vector3& operator = (const Vector3& vector) {
	            x = vector.x;
				y = vector.y;
				z = vector.z;

				return *this;
			}

			inline Vector3 operator + (const Vector3& vector) {
				return Vector3(x+vector.x, y+vector.y, z+vector.z);
			}

			inline Vector3 operator - (const Vector3& vector) {
				return Vector3(x-vector.x, y-vector.y, z-vector.z);
			}

			inline float distance(const Vector3& rhs) {
	            return (*this - rhs).length();
			}

			inline float squaredDistance(const Vector3& rhs) {
	            return (*this - rhs).squaredLength();
			}

			inline Vector3 operator * (const float& mult) {
				return Vector3(x*mult, y*mult, z*mult);
			}

			inline Vector3 operator / (const float& div) {
				return Vector3(x/div, y/div, z/div);
			}

			inline Vector3 crossProduct(const Vector3& rkVector) const
			{
				return Vector3(
					y * rkVector.z - z * rkVector.y,
					z * rkVector.x - x * rkVector.z,
					x * rkVector.y - y * rkVector.x);
			}

			inline float dotProduct(const Vector3& vec) const
			{
	            return x * vec.x + y * vec.y + z * vec.z;
			}

			inline float absDotProduct(const Vector3& vec) const
			{
	            return abs(x * vec.x) + abs(y * vec.y) + abs(z * vec.z);
			}

			inline bool fastDistCheck(const Vector3& vec, const float& dist) {
				if (abs(vec.x - x) > dist) return false;
				if (abs(vec.y - y) > dist) return false;
				if (abs(vec.z - z) > dist) return false;
				return true;
			}

			inline float length() {
				return sqrt((x * x) + (y * y) + (z * z));
			}

			inline float squaredLength() {
				return (x * x) + (y * y) + (z * z);
			}

			inline float normalise() {
				float fLength = sqrt( x * x + y * y + z * z );

				if (fLength > 0.0f) {
	                float fInvLength = 1.0f / fLength;
	                x *= fInvLength;
					y *= fInvLength;
					z *= fInvLength;
				}

				return fLength;
			}

			Vector2 toBarycentric(Vector3 &a, Vector3 &b, Vector3 &c);
			int relativeCorner(Vector3 &v);
			bool sameSide(Vector3 &p2, Vector3 &a, Vector3 &b);
	};


	
	class Quaternion {
		public:
			float x;
			float y;
			float z;
			float w;

			Quaternion() : x(0), y(0), z(0), w(1.0f) {
			}

			Quaternion(float w_p, float x_p, float y_p, float z_p) : w(w_p), x(x_p), y(y_p), z(z_p) {
			}

			Quaternion(const Matrix3& rot) {
				this->fromRotationMatrix(rot);
			}

			inline Quaternion& operator = (const Quaternion& q) {
				w = q.w;
	            x = q.x;
				y = q.y;
				z = q.z;

				return *this;
			}

			Quaternion Quaternion::operator* (const Quaternion& rkQ) {
				return Quaternion
				(
					w * rkQ.w - x * rkQ.x - y * rkQ.y - z * rkQ.z,
					w * rkQ.x + x * rkQ.w + y * rkQ.z - z * rkQ.y,
					w * rkQ.y + y * rkQ.w + z * rkQ.x - x * rkQ.z,
					w * rkQ.z + z * rkQ.w + x * rkQ.y - y * rkQ.x
				);
			}

			
			Quaternion slerp(float fT, const Quaternion& rkP, const Quaternion& rkQ, bool shortestPath=false);
			float dot (const Quaternion& rkQ) const {
				return w*rkQ.w+x*rkQ.x+y*rkQ.y+z*rkQ.z;
			}

			Quaternion operator+ (const Quaternion& rkQ) const {
				return Quaternion(w+rkQ.w,x+rkQ.x,y+rkQ.y,z+rkQ.z);
			}

			Quaternion operator- () const {
				return Quaternion(-w,-x,-y,-z);
			}

			Quaternion operator* (float fScalar) const {
				return Quaternion(fScalar*w,fScalar*x,fScalar*y,fScalar*z);
			}

			Quaternion operator* (const Quaternion& rkQ) const {
				return Quaternion
				(
					w * rkQ.w - x * rkQ.x - y * rkQ.y - z * rkQ.z,
					w * rkQ.x + x * rkQ.w + y * rkQ.z - z * rkQ.y,
					w * rkQ.y + y * rkQ.w + z * rkQ.x - x * rkQ.z,
					w * rkQ.z + z * rkQ.w + x * rkQ.y - y * rkQ.x
				);
			}

			float Norm () const {
				return w*w+x*x+y*y+z*z;
			}

			float normalise(void) {
				float len = Norm();
				float factor = 1.0f / sqrt(len);
				*this = *this * factor;
				return len;
			}

			Quaternion inverse() const {
				float fNorm = w*w+x*x+y*y+z*z;
				if (fNorm > 0.0) {
					float fInvNorm = 1.0f/fNorm;
					return Quaternion(w*fInvNorm, -x*fInvNorm, -y*fInvNorm, -z*fInvNorm);
				}
				else {
					return Quaternion(0, 0, 0, 0);
				}
			}

			Vector3 operator* (Vector3& v) const {
				Vector3 uv, uuv;
				Vector3 qvec(x, y, z);
				uv = qvec.crossProduct(v);
				uuv = qvec.crossProduct(uv);
				uv = uv * (2.0f * w);
				uuv = uuv * 2.0f;

				return v + uv + uuv;

			}

			Vector3 toLostWorldEuler() const {
				return Vector3( asinf(-2 * (y*z - w*x)),
								atan2f(2*(x*z + w*y), powf(w,2) - powf(x,2) - powf(y,2) + powf(z,2)),
								atan2f(2*(x*y + w*z), powf(w,2) - powf(x,2) + powf(y,2) - powf(z,2))
							  );
			}

			float getYaw();
			float getYawDegrees();

			void fromXYZInts(int rx, int ry, int rz);
			void fromZXYInts(int rx, int ry, int rz);
			void fromXZYInts(int rx, int ry, int rz);
			void fromLostWorldEuler(Vector3 euler);
			void fromAngleAxis(float rfAngle, Vector3 rkAxis);
			void fromRotationMatrix(const Matrix3& kRot);
			void toRotationMatrix(Matrix3& kRot);

			void read(File *file);
			void write(File *file);
			void readXML(TiXmlElement *root);
			void readSingleXML(TiXmlElement *root);
			void writeXML(TiXmlElement *root);
	};


	class Triangle {
		public:
			Vector3 point[3];
			Vector3 edges[3];
			Vector3 edges_dir[3];
			float lengths[3];

			Vector3 normal;
			Vector3 tangent;
			Vector3 bitangent;

			Triangle(Vector3 v1, Vector3 v2, Vector3 v3) {
				point[0] = v1;
				point[1] = v2;
				point[2] = v3;

				edges[0] = point[1] - point[0];
				edges[1] = point[2] - point[1];
				edges[2] = point[0] - point[2];

				edges_dir[0] = edges[0];
				edges_dir[0].normalise();
				edges_dir[1] = edges[1];
				edges_dir[1].normalise();
				edges_dir[2] = edges[2];
				edges_dir[2].normalise();

				lengths[0] = edges[0].length();
				lengths[1] = edges[1].length();
				lengths[2] = edges[2].length();

				normal = (point[1] - point[0]).crossProduct(point[2] - point[0]);
				normal.normalise();

				Vector3 c1 = normal.crossProduct(Vector3(0.0, 0.0, 1.0));
				Vector3 c2 = normal.crossProduct(Vector3(0.0, 1.0, 0.0));

				if(c1.length() > c2.length()) tangent = c1;
				else tangent = c2;
				tangent.normalise();
				bitangent = tangent.crossProduct(normal);
			}

			bool degenerate() {
				return (!normal.length());
			}

			void generatePoints(vector<Vector3> *list, float unit_size=1.0f);

			float getArea() {
				float p=(lengths[0]+lengths[1]+lengths[2])/2.0f;
				return sqrt(p*(p-lengths[0])*(p-lengths[1])*(p-lengths[2]));
			}

			Vector3 center() {
				return (point[0]+point[1]+point[2])/3.0f;
			}

			bool intersects(Vector3 &p);
	};

	
	class Color8 {
		public:
			unsigned char r, g, b, a;

			Color8() : r(255), g(255), b(255), a(255) {
			}

			Color8(unsigned char r_p, unsigned char g_p, unsigned char b_p, unsigned char a_p) : r(r_p), g(g_p), b(b_p), a(a_p) {
			}

			inline Color8& operator = (const Color8& col) {
	            r = col.r;
				g = col.g;
				b = col.b;
				a = col.a;

				return *this;
			}
	};

	class Color {
		public:
			float r, g, b, a;

			Color() : r(1), g(1), b(1), a(1) {
			}


			Color(float r_p, float g_p, float b_p) : r(r_p), g(g_p), b(b_p), a(1) {
			}

			Color(float r_p, float g_p, float b_p, float a_p) : r(r_p), g(g_p), b(b_p), a(a_p) {
			}

			Color(unsigned char *rgba) : r(((float)rgba[0])/LIBGENS_MATH_COLOR_CHAR), 
										 g(((float)rgba[1])/LIBGENS_MATH_COLOR_CHAR), 
										 b(((float)rgba[2])/LIBGENS_MATH_COLOR_CHAR), 
										 a(((float)rgba[3])/LIBGENS_MATH_COLOR_CHAR) {
			}

			Color(Color8 col) {
				Color::Color((unsigned char *) &col);
			}

			inline bool operator == (const Color& color) {
	            return ((r == color.r) && (g == color.g) && (b == color.b) && (a == color.a));
			}

			inline bool operator != (const Color& color) {
	            return ((r != color.r) || (g != color.g) || (b != color.b) || (a != color.a));
			}

			inline Color& operator = (const Color& col) {
	            r = col.r;
				g = col.g;
				b = col.b;
				a = col.a;

				return *this;
			}

			void read(File *file, bool big_endian=false);
			void readARGB8(File *file);
			void readABGR8(File *file);
			void readRGBA8(File *file);

			void write(File *file, bool big_endian=false);
			void writeARGB8(File *file);

			void interpolate(const Color &v, const float m) {
				r+=(v.r-r)*m;
				g+=(v.g-g)*m;
				b+=(v.b-b)*m;
				a+=(v.a-a)*m;
			}

			void setSaturation(const float &multiplier) {
				float m=(r+g+b)/3.0f;

				r+=(m-r)*(1.0f-multiplier);
				g+=(m-g)*(1.0f-multiplier);
				b+=(m-b)*(1.0f-multiplier);
			}
	};

	class Matrix3 {
		public:
			float m[3][3];

			Matrix3 () {}

			inline float* operator[] (size_t iRow) const
			{
				return const_cast<float*>(m[iRow]);
			}

			explicit Matrix3 (const float arr[3][3]) {
				memcpy(m,arr,9*sizeof(float));
			}

			inline Matrix3 (const Matrix3& rkMatrix) {
				memcpy(m,rkMatrix.m,9*sizeof(float));
			}

			Matrix3 (float fEntry00, float fEntry01, float fEntry02,
					float fEntry10, float fEntry11, float fEntry12,
					float fEntry20, float fEntry21, float fEntry22) {
				m[0][0] = fEntry00; m[0][1] = fEntry01; m[0][2] = fEntry02;
				m[1][0] = fEntry10; m[1][1] = fEntry11; m[1][2] = fEntry12;
				m[2][0] = fEntry20; m[2][1] = fEntry21; m[2][2] = fEntry22;
			}

			inline Matrix3& operator= (const Matrix3& rkMatrix) {
				memcpy(m,rkMatrix.m,9*sizeof(float));
				return *this;
			}

			friend Vector3 operator* (const Vector3& rkPoint, const Matrix3& rkMatrix) {
				Vector3 kProd;
				kProd.x = rkPoint.x*rkMatrix.m[0][0] + rkPoint.y*rkMatrix.m[1][0] + rkPoint.z*rkMatrix.m[2][0];
				kProd.y = rkPoint.x*rkMatrix.m[0][1] + rkPoint.y*rkMatrix.m[1][1] + rkPoint.z*rkMatrix.m[2][1];
				kProd.z = rkPoint.x*rkMatrix.m[0][2] + rkPoint.y*rkMatrix.m[1][2] + rkPoint.z*rkMatrix.m[2][2];
				return kProd;
			}

			Matrix3 operator* (const Matrix3& rkMatrix);

			void QDUDecomposition(Matrix3& kQ, Vector3& kD, Vector3& kU);
			bool toEulerAnglesZYX(float& rfYAngle, float& rfPAngle, float& rfRAngle);
			bool toEulerAnglesYXZ(float& rfYAngle, float& rfPAngle, float& rfRAngle);
			bool toEulerAnglesYZX(float& rfYAngle, float& rfPAngle, float& rfRAngle);
			void fromEulerAnglesZYX(const float& fYAngle, const float& fPAngle, const float& fRAngle);
			void fromEulerAnglesYXZ(const float& fYAngle, const float& fPAngle, const float& fRAngle);
			void fromEulerAnglesYZX(const float& fYAngle, const float& fPAngle, const float& fRAngle);
    };

	class Matrix4 {
		public:
			float m[4][4];
			void read(File *file, bool big_endian=true);
			void write(File *file, bool big_endian=true);

			Matrix4() {
				for (size_t x=0; x<4; x++) {
					for (size_t y=0; y<4; y++) {
						if (x == y) m[x][y] = 1.0;
						else m[x][y] = 0.0;
					}
				}
			}

			inline float* operator[] (size_t iRow) const
			{
				return const_cast<float*>(m[iRow]);
			}

			Matrix4(float m00, float m01, float m02, float m03,
				    float m10, float m11, float m12, float m13,
					float m20, float m21, float m22, float m23,
					float m30, float m31, float m32, float m33) {

				m[0][0] = m00; m[0][1] = m01; m[0][2] = m02; m[0][3] = m03;
				m[1][0] = m10; m[1][1] = m11; m[1][2] = m12; m[1][3] = m13;
				m[2][0] = m20; m[2][1] = m21; m[2][2] = m22; m[2][3] = m23;
				m[3][0] = m30; m[3][1] = m31; m[3][2] = m32; m[3][3] = m33;
			}

			inline Vector3 operator * ( const Vector3 &v ) const {
				Vector3 r;

				float fInvW = 1.0f / ( m[3][0] * v.x + m[3][1] * v.y + m[3][2] * v.z + m[3][3] );

				r.x = ( m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z + m[0][3] ) * fInvW;
				r.y = ( m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z + m[1][3] ) * fInvW;
				r.z = ( m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z + m[2][3] ) * fInvW;

				return r;
			}

			void extract3x3Matrix(Matrix3& m3x3) {
				m3x3.m[0][0] = m[0][0];
				m3x3.m[0][1] = m[0][1];
				m3x3.m[0][2] = m[0][2];
				m3x3.m[1][0] = m[1][0];
				m3x3.m[1][1] = m[1][1];
				m3x3.m[1][2] = m[1][2];
				m3x3.m[2][0] = m[2][0];
				m3x3.m[2][1] = m[2][1];
				m3x3.m[2][2] = m[2][2];
			}

			void decomposition(Vector3& position, Vector3& scale, Quaternion& orientation);
			void makeTransform(Vector3& position, Vector3& scale, Quaternion& orientation);
			
			inline Matrix4 concatenate(const Matrix4 &m2) {
				Matrix4 r;
				r.m[0][0] = m[0][0] * m2.m[0][0] + m[0][1] * m2.m[1][0] + m[0][2] * m2.m[2][0] + m[0][3] * m2.m[3][0];
				r.m[0][1] = m[0][0] * m2.m[0][1] + m[0][1] * m2.m[1][1] + m[0][2] * m2.m[2][1] + m[0][3] * m2.m[3][1];
				r.m[0][2] = m[0][0] * m2.m[0][2] + m[0][1] * m2.m[1][2] + m[0][2] * m2.m[2][2] + m[0][3] * m2.m[3][2];
				r.m[0][3] = m[0][0] * m2.m[0][3] + m[0][1] * m2.m[1][3] + m[0][2] * m2.m[2][3] + m[0][3] * m2.m[3][3];

				r.m[1][0] = m[1][0] * m2.m[0][0] + m[1][1] * m2.m[1][0] + m[1][2] * m2.m[2][0] + m[1][3] * m2.m[3][0];
				r.m[1][1] = m[1][0] * m2.m[0][1] + m[1][1] * m2.m[1][1] + m[1][2] * m2.m[2][1] + m[1][3] * m2.m[3][1];
				r.m[1][2] = m[1][0] * m2.m[0][2] + m[1][1] * m2.m[1][2] + m[1][2] * m2.m[2][2] + m[1][3] * m2.m[3][2];
				r.m[1][3] = m[1][0] * m2.m[0][3] + m[1][1] * m2.m[1][3] + m[1][2] * m2.m[2][3] + m[1][3] * m2.m[3][3];

				r.m[2][0] = m[2][0] * m2.m[0][0] + m[2][1] * m2.m[1][0] + m[2][2] * m2.m[2][0] + m[2][3] * m2.m[3][0];
				r.m[2][1] = m[2][0] * m2.m[0][1] + m[2][1] * m2.m[1][1] + m[2][2] * m2.m[2][1] + m[2][3] * m2.m[3][1];
				r.m[2][2] = m[2][0] * m2.m[0][2] + m[2][1] * m2.m[1][2] + m[2][2] * m2.m[2][2] + m[2][3] * m2.m[3][2];
				r.m[2][3] = m[2][0] * m2.m[0][3] + m[2][1] * m2.m[1][3] + m[2][2] * m2.m[2][3] + m[2][3] * m2.m[3][3];

				r.m[3][0] = m[3][0] * m2.m[0][0] + m[3][1] * m2.m[1][0] + m[3][2] * m2.m[2][0] + m[3][3] * m2.m[3][0];
				r.m[3][1] = m[3][0] * m2.m[0][1] + m[3][1] * m2.m[1][1] + m[3][2] * m2.m[2][1] + m[3][3] * m2.m[3][1];
				r.m[3][2] = m[3][0] * m2.m[0][2] + m[3][1] * m2.m[1][2] + m[3][2] * m2.m[2][2] + m[3][3] * m2.m[3][2];
				r.m[3][3] = m[3][0] * m2.m[0][3] + m[3][1] * m2.m[1][3] + m[3][2] * m2.m[2][3] + m[3][3] * m2.m[3][3];

	            return r;
			}

			Matrix4 operator * (const Matrix4 &m2) {
	            return concatenate(m2);
			}

			Matrix4 inverse()  {
				float m00 = m[0][0], m01 = m[0][1], m02 = m[0][2], m03 = m[0][3];
				float m10 = m[1][0], m11 = m[1][1], m12 = m[1][2], m13 = m[1][3];
				float m20 = m[2][0], m21 = m[2][1], m22 = m[2][2], m23 = m[2][3];
				float m30 = m[3][0], m31 = m[3][1], m32 = m[3][2], m33 = m[3][3];

				float v0 = m20 * m31 - m21 * m30;
				float v1 = m20 * m32 - m22 * m30;
				float v2 = m20 * m33 - m23 * m30;
				float v3 = m21 * m32 - m22 * m31;
				float v4 = m21 * m33 - m23 * m31;
				float v5 = m22 * m33 - m23 * m32;

				float t00 = + (v5 * m11 - v4 * m12 + v3 * m13);
				float t10 = - (v5 * m10 - v2 * m12 + v1 * m13);
				float t20 = + (v4 * m10 - v2 * m11 + v0 * m13);
				float t30 = - (v3 * m10 - v1 * m11 + v0 * m12);

				float invDet = 1 / (t00 * m00 + t10 * m01 + t20 * m02 + t30 * m03);

				float d00 = t00 * invDet;
				float d10 = t10 * invDet;
				float d20 = t20 * invDet;
				float d30 = t30 * invDet;
	
				float d01 = - (v5 * m01 - v4 * m02 + v3 * m03) * invDet;
				float d11 = + (v5 * m00 - v2 * m02 + v1 * m03) * invDet;
				float d21 = - (v4 * m00 - v2 * m01 + v0 * m03) * invDet;
				float d31 = + (v3 * m00 - v1 * m01 + v0 * m02) * invDet;

				v0 = m10 * m31 - m11 * m30;
				v1 = m10 * m32 - m12 * m30;
				v2 = m10 * m33 - m13 * m30;
				v3 = m11 * m32 - m12 * m31;
				v4 = m11 * m33 - m13 * m31;
				v5 = m12 * m33 - m13 * m32;

				float d02 = + (v5 * m01 - v4 * m02 + v3 * m03) * invDet;
				float d12 = - (v5 * m00 - v2 * m02 + v1 * m03) * invDet;
				float d22 = + (v4 * m00 - v2 * m01 + v0 * m03) * invDet;
				float d32 = - (v3 * m00 - v1 * m01 + v0 * m02) * invDet;

				v0 = m21 * m10 - m20 * m11;
				v1 = m22 * m10 - m20 * m12;
				v2 = m23 * m10 - m20 * m13;
				v3 = m22 * m11 - m21 * m12;
				v4 = m23 * m11 - m21 * m13;
				v5 = m23 * m12 - m22 * m13;

				float d03 = - (v5 * m01 - v4 * m02 + v3 * m03) * invDet;
				float d13 = + (v5 * m00 - v2 * m02 + v1 * m03) * invDet;
				float d23 = - (v4 * m00 - v2 * m01 + v0 * m03) * invDet;
				float d33 = + (v3 * m00 - v1 * m01 + v0 * m02) * invDet;

				return Matrix4(
					d00, d01, d02, d03,
					d10, d11, d12, d13,
					d20, d21, d22, d23,
					d30, d31, d32, d33);
			}

			inline Matrix4 transpose(void) const {
	            return Matrix4(m[0][0], m[1][0], m[2][0], m[3][0],
							   m[0][1], m[1][1], m[2][1], m[3][1],
							   m[0][2], m[1][2], m[2][2], m[3][2],
                               m[0][3], m[1][3], m[2][3], m[3][3]);
			}
	};

	Matrix4 convertBoneMatrixToRegular(Matrix4 m);

	class AABB {
		public:
			Vector3 start;
			Vector3 end;

			AABB() {
				reset();
			}

			AABB(Vector3 A, Vector3 B, Vector3 C) {
				reset();
				addPoint(A);
				addPoint(B);
				addPoint(C);
			}

			void reset() {
				start.x = LIBGENS_AABB_MAX_START;
				start.y = LIBGENS_AABB_MAX_START;
				start.z = LIBGENS_AABB_MAX_START;

				end.x = LIBGENS_AABB_MIN_END;
				end.y = LIBGENS_AABB_MIN_END;
				end.z = LIBGENS_AABB_MIN_END;
			}

			void read(File *file);
			void write(File *file);
			void addPoint(Vector3 point) {
				if (point.x < start.x) start.x = point.x;
				if (point.y < start.y) start.y = point.y;
				if (point.z < start.z) start.z = point.z;

				if (point.x > end.x) end.x = point.x;
				if (point.y > end.y) end.y = point.y;
				if (point.z > end.z) end.z = point.z;
			}

			AABB half(int axis, int side);

			float centerX();
			float centerY();
			float centerZ();
			float sizeX();
			float sizeY();
			float sizeZ();
			float sizeMax();
			Vector3 center();
			float size();

			Vector3 corner(int type);

			inline AABB& operator = (const AABB& aabb) {
	            start = aabb.start;
				end = aabb.end;

				return *this;
			}

			bool intersects(const AABB& aabb);
			bool intersects(const Vector3& v);
			AABB intersection(const AABB& aabb);
			void merge(const AABB& aabb);

			
			void expandX(float v) {
				start.x -= v;
				end.x   += v;
			}

			void expandY(float v) {
				start.y -= v;
				end.y   += v;
			}

			void expandZ(float v) {
				start.z -= v;
				end.z   += v;
			}

			void expand(float v) {
				start.x -= v;
				start.y -= v;
				start.z -= v;
				end.x   += v;
				end.y   += v;
				end.z   += v;
			}

			void transform(Matrix4 mat) {
				Vector3 corners[8];
				for (int c = 0; c < 8; c++)
					corners[c] = mat * corner(c);

				reset();
				for (int c = 0; c < 8; c++)
					addPoint(corners[c]);
			}
	};
};
