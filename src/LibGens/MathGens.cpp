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

namespace LibGens {
	float asm_rsq(float r) {
		__asm {
			fld1  // r0 = 1.f
			fld r // r1 = r0, r0 = r
			fsqrt // r0 = sqrtf( r0 )
			fdiv  // r0 = r1 / r0
		}
	}

    bool Matrix3::toEulerAnglesZYX(float& rfYAngle, float& rfPAngle, float& rfRAngle) {
        rfPAngle = asin(-m[2][0]);
        if (rfPAngle < LIBGENS_MATH_HALF_PI) {
            if (rfPAngle > -LIBGENS_MATH_HALF_PI) {
                rfYAngle = atan2(m[1][0],m[0][0]);
                rfRAngle = atan2(m[2][1],m[2][2]);
                return true;
            }
            else {
                float fRmY = atan2(-m[0][1],m[0][2]);
                rfRAngle = 0.0f;
                rfYAngle = rfRAngle - fRmY;
                return false;
            }
        }
        else {
            float fRpY = atan2(-m[0][1],m[0][2]);
            rfRAngle = 0.0f;
            rfYAngle = fRpY - rfRAngle;
            return false;
        }
    }

	bool Matrix3::toEulerAnglesYXZ(float& rfYAngle, float& rfPAngle, float& rfRAngle) {
        rfPAngle = asin(-m[1][2]);
        if (rfPAngle < LIBGENS_MATH_HALF_PI) {
            if (rfPAngle > -LIBGENS_MATH_HALF_PI) {
                rfYAngle = atan2(m[0][2],m[2][2]);
                rfRAngle = atan2(m[1][0],m[1][1]);
                return true;
            }
            else {
                float fRmY = atan2(-m[0][1],m[0][0]);
                rfRAngle = 0.0f;
                rfYAngle = rfRAngle - fRmY;
                return false;
            }
        }
        else {
            float fRpY = atan2(-m[0][1],m[0][0]);
            rfRAngle = 0.0f;
            rfYAngle = fRpY - rfRAngle;
            return false;
        }
    }

	bool Matrix3::toEulerAnglesYZX(float& rfYAngle, float& rfPAngle, float& rfRAngle) {
        rfPAngle = asin(m[1][0]);
        if (rfPAngle < LIBGENS_MATH_HALF_PI) {
            if (rfPAngle > -LIBGENS_MATH_HALF_PI) {
                rfYAngle = atan2(-m[2][0],m[0][0]);
                rfRAngle = atan2(-m[1][2],m[1][1]);
                return true;
            }
            else {
                float fRmY = atan2(m[2][1],m[2][2]);
                rfRAngle = 0.0f;
                rfYAngle = rfRAngle - fRmY;
                return false;
            }
        }
        else {
            float fRpY = atan2(m[2][1],m[2][2]);
            rfRAngle = 0.0f;
            rfYAngle = fRpY - rfRAngle;
            return false;
        }
    }

    void Matrix3::fromEulerAnglesZYX(const float& fYAngle, const float& fPAngle, const float& fRAngle) {
        float fCos, fSin;

        fCos = cos(fYAngle);
        fSin = sin(fYAngle);
        Matrix3 kZMat(fCos,-fSin,0.0,fSin,fCos,0.0,0.0,0.0,1.0);

        fCos = cos(fPAngle);
        fSin = sin(fPAngle);
        Matrix3 kYMat(fCos,0.0,fSin,0.0,1.0,0.0,-fSin,0.0,fCos);

        fCos = cos(fRAngle);
        fSin = sin(fRAngle);
        Matrix3 kXMat(1.0,0.0,0.0,0.0,fCos,-fSin,0.0,fSin,fCos);

        *this = kZMat*(kYMat*kXMat);
    }

	
    void Matrix3::fromEulerAnglesYXZ(const float& fYAngle, const float& fPAngle, const float& fRAngle) {
        float fCos, fSin;

        fCos = cos(fYAngle);
        fSin = sin(fYAngle);
        Matrix3 kZMat(fCos,-fSin,0.0,fSin,fCos,0.0,0.0,0.0,1.0);

        fCos = cos(fPAngle);
        fSin = sin(fPAngle);
        Matrix3 kYMat(fCos,0.0,fSin,0.0,1.0,0.0,-fSin,0.0,fCos);

        fCos = cos(fRAngle);
        fSin = sin(fRAngle);
        Matrix3 kXMat(1.0,0.0,0.0,0.0,fCos,-fSin,0.0,fSin,fCos);

        *this = kYMat*(kXMat*kZMat);
    }

	
    void Matrix3::fromEulerAnglesYZX(const float& fYAngle, const float& fPAngle, const float& fRAngle) {
        float fCos, fSin;

        fCos = cos(fYAngle);
        fSin = sin(fYAngle);
        Matrix3 kZMat(fCos,-fSin,0.0,fSin,fCos,0.0,0.0,0.0,1.0);

        fCos = cos(fPAngle);
        fSin = sin(fPAngle);
        Matrix3 kYMat(fCos,0.0,fSin,0.0,1.0,0.0,-fSin,0.0,fCos);

        fCos = cos(fRAngle);
        fSin = sin(fRAngle);
        Matrix3 kXMat(1.0,0.0,0.0,0.0,fCos,-fSin,0.0,fSin,fCos);

        *this = kYMat*(kZMat*kXMat);
    }


	Matrix3 Matrix3::operator* (const Matrix3& rkMatrix) {
        Matrix3 kProd;
        for (size_t iRow = 0; iRow < 3; iRow++)
        {
            for (size_t iCol = 0; iCol < 3; iCol++)
            {
                kProd.m[iRow][iCol] =
                    m[iRow][0]*rkMatrix.m[0][iCol] +
                    m[iRow][1]*rkMatrix.m[1][iCol] +
                    m[iRow][2]*rkMatrix.m[2][iCol];
            }
        }
        return kProd;
    }

	void Matrix3::QDUDecomposition(Matrix3& kQ, Vector3& kD, Vector3& kU) {
        float fInvLength = m[0][0]*m[0][0] + m[1][0]*m[1][0] + m[2][0]*m[2][0];
		if (fInvLength != 0) fInvLength = asm_rsq(fInvLength);

        kQ[0][0] = m[0][0]*fInvLength;
        kQ[1][0] = m[1][0]*fInvLength;
        kQ[2][0] = m[2][0]*fInvLength;

        float fDot = kQ[0][0]*m[0][1] + kQ[1][0]*m[1][1] + kQ[2][0]*m[2][1];
        kQ[0][1] = m[0][1]-fDot*kQ[0][0];
        kQ[1][1] = m[1][1]-fDot*kQ[1][0];
        kQ[2][1] = m[2][1]-fDot*kQ[2][0];
		fInvLength = kQ[0][1]*kQ[0][1] + kQ[1][1]*kQ[1][1] + kQ[2][1]*kQ[2][1];
		if (fInvLength != 0) fInvLength = asm_rsq(fInvLength);
        
        kQ[0][1] *= fInvLength;
        kQ[1][1] *= fInvLength;
        kQ[2][1] *= fInvLength;

        fDot = kQ[0][0]*m[0][2] + kQ[1][0]*m[1][2] + kQ[2][0]*m[2][2];
        kQ[0][2] = m[0][2]-fDot*kQ[0][0];
        kQ[1][2] = m[1][2]-fDot*kQ[1][0];
        kQ[2][2] = m[2][2]-fDot*kQ[2][0];

        fDot = kQ[0][1]*m[0][2] + kQ[1][1]*m[1][2] + kQ[2][1]*m[2][2];
        kQ[0][2] -= fDot*kQ[0][1];
        kQ[1][2] -= fDot*kQ[1][1];
        kQ[2][2] -= fDot*kQ[2][1];
        fInvLength = kQ[0][2]*kQ[0][2] + kQ[1][2]*kQ[1][2] + kQ[2][2]*kQ[2][2];

		if (fInvLength != 0) fInvLength = asm_rsq(fInvLength);

		kQ[0][2] *= fInvLength;
        kQ[1][2] *= fInvLength;
        kQ[2][2] *= fInvLength;

        float fDet = kQ[0][0]*kQ[1][1]*kQ[2][2] + kQ[0][1]*kQ[1][2]*kQ[2][0] + kQ[0][2]*kQ[1][0]*kQ[2][1] - kQ[0][2]*kQ[1][1]*kQ[2][0] - kQ[0][1]*kQ[1][0]*kQ[2][2] - kQ[0][0]*kQ[1][2]*kQ[2][1];

        if ( fDet < 0.0 ) {
            for (size_t iRow = 0; iRow < 3; iRow++)
                for (size_t iCol = 0; iCol < 3; iCol++)
                    kQ[iRow][iCol] = -kQ[iRow][iCol];
        }


        Matrix3 kR;
        kR[0][0] = kQ[0][0]*m[0][0] + kQ[1][0]*m[1][0] + kQ[2][0]*m[2][0];
        kR[0][1] = kQ[0][0]*m[0][1] + kQ[1][0]*m[1][1] + kQ[2][0]*m[2][1];
        kR[1][1] = kQ[0][1]*m[0][1] + kQ[1][1]*m[1][1] + kQ[2][1]*m[2][1];
        kR[0][2] = kQ[0][0]*m[0][2] + kQ[1][0]*m[1][2] + kQ[2][0]*m[2][2];
        kR[1][2] = kQ[0][1]*m[0][2] + kQ[1][1]*m[1][2] + kQ[2][1]*m[2][2];
        kR[2][2] = kQ[0][2]*m[0][2] + kQ[1][2]*m[1][2] + kQ[2][2]*m[2][2];

        kD.x = kR[0][0];
        kD.y = kR[1][1];
        kD.z = kR[2][2];

        float fInvD0 = 1.0f/kD.x;
        kU.x = kR[0][1]*fInvD0;
        kU.y = kR[0][2]*fInvD0;
        kU.z = kR[1][2]/kD.y;
    }

	void Quaternion::fromXYZInts(int rx, int ry, int rz) {
		float rot_x=rx * LIBGENS_MATH_INT32_TO_RAD;
		float rot_y=ry * LIBGENS_MATH_INT32_TO_RAD;
		float rot_z=rz * LIBGENS_MATH_INT32_TO_RAD;

		Matrix3 mr;
		mr.fromEulerAnglesZYX(rot_z, rot_y, rot_x);
		fromRotationMatrix(mr);
	}

	void Quaternion::fromZXYInts(int rx, int ry, int rz) {
		float rot_x=rx * LIBGENS_MATH_INT32_TO_RAD;
		float rot_y=ry * LIBGENS_MATH_INT32_TO_RAD;
		float rot_z=rz * LIBGENS_MATH_INT32_TO_RAD;

		Matrix3 mr;
		mr.fromEulerAnglesYXZ(rot_z, rot_y, rot_x);
		fromRotationMatrix(mr);
	}

	void Quaternion::fromXZYInts(int rx, int ry, int rz) {
		float rot_x=rx * LIBGENS_MATH_INT32_TO_RAD;
		float rot_y=ry * LIBGENS_MATH_INT32_TO_RAD;
		float rot_z=rz * LIBGENS_MATH_INT32_TO_RAD;

		Matrix3 mr;
		mr.fromEulerAnglesYZX(rot_z, rot_y, rot_x);
		fromRotationMatrix(mr);
	}
	

	void Quaternion::fromRotationMatrix(const Matrix3& kRot) {
        float fTrace = kRot[0][0]+kRot[1][1]+kRot[2][2];
        float fRoot;

        if (fTrace > 0.0) {
            fRoot = sqrt(fTrace + 1.0f);
            w = 0.5f*fRoot;
            fRoot = 0.5f/fRoot;
            x = (kRot[2][1]-kRot[1][2])*fRoot;
            y = (kRot[0][2]-kRot[2][0])*fRoot;
            z = (kRot[1][0]-kRot[0][1])*fRoot;
        }
        else {
            static size_t s_iNext[3] = { 1, 2, 0 };
            size_t i = 0;
            if ( kRot[1][1] > kRot[0][0] )
                i = 1;
            if ( kRot[2][2] > kRot[i][i] )
                i = 2;
            size_t j = s_iNext[i];
            size_t k = s_iNext[j];

            fRoot = sqrt(kRot[i][i]-kRot[j][j]-kRot[k][k] + 1.0f);
            float* apkQuat[3] = { &x, &y, &z };
            *apkQuat[i] = 0.5f*fRoot;
            fRoot = 0.5f/fRoot;
            w = (kRot[k][j]-kRot[j][k])*fRoot;
            *apkQuat[j] = (kRot[j][i]+kRot[i][j])*fRoot;
            *apkQuat[k] = (kRot[k][i]+kRot[i][k])*fRoot;
        }
    }

    void Quaternion::toRotationMatrix(Matrix3& kRot) {
        float fTx = x+x;
        float fTy = y+y;
        float fTz = z+z;
        float fTwx = fTx*w;
        float fTwy = fTy*w;
        float fTwz = fTz*w;
        float fTxx = fTx*x;
        float fTxy = fTy*x;
        float fTxz = fTz*x;
        float fTyy = fTy*y;
        float fTyz = fTz*y;
        float fTzz = fTz*z;

        kRot[0][0] = 1.0f-(fTyy+fTzz);
        kRot[0][1] = fTxy-fTwz;
        kRot[0][2] = fTxz+fTwy;
        kRot[1][0] = fTxy+fTwz;
        kRot[1][1] = 1.0f-(fTxx+fTzz);
        kRot[1][2] = fTyz-fTwx;
        kRot[2][0] = fTxz-fTwy;
        kRot[2][1] = fTyz+fTwx;
        kRot[2][2] = 1.0f-(fTxx+fTyy);
    }

	void Matrix4::decomposition(Vector3& position, Vector3& scale, Quaternion& orientation) {
		Matrix3 m3x3;
		extract3x3Matrix(m3x3);

		Matrix3 matQ;
		Vector3 vecU;
		m3x3.QDUDecomposition(matQ, scale, vecU);

		orientation = Quaternion(matQ);
		position = Vector3(m[0][3], m[1][3], m[2][3]);
	}

	void Matrix4::makeTransform( Vector3& position, Vector3& scale, Quaternion& orientation) {
        Matrix3 rot3x3;
        orientation.toRotationMatrix(rot3x3);

        m[0][0] = scale.x * rot3x3[0][0]; m[0][1] = scale.y * rot3x3[0][1]; m[0][2] = scale.z * rot3x3[0][2]; m[0][3] = position.x;
        m[1][0] = scale.x * rot3x3[1][0]; m[1][1] = scale.y * rot3x3[1][1]; m[1][2] = scale.z * rot3x3[1][2]; m[1][3] = position.y;
        m[2][0] = scale.x * rot3x3[2][0]; m[2][1] = scale.y * rot3x3[2][1]; m[2][2] = scale.z * rot3x3[2][2]; m[2][3] = position.z;
        m[3][0] = 0; m[3][1] = 0; m[3][2] = 0; m[3][3] = 1;
    }

	void Vector2::read(File *file, bool big_endian) {
		file->readFloat32E(&x, big_endian);
		file->readFloat32E(&y, big_endian);
	}

	void Vector2::readHalf(File *file, bool big_endian) {
		file->readFloat16E(&x, big_endian);
		file->readFloat16E(&y, big_endian);
	}

	void Vector2::write(File *file, bool big_endian) {
		if (big_endian) {
			file->writeFloat32BE(&x);
			file->writeFloat32BE(&y);
		}
		else {
			file->writeFloat32(&x);
			file->writeFloat32(&y);
		}
	}

	void Vector3::read(File *file, bool big_endian) {
		file->readFloat32E(&x, big_endian);
		file->readFloat32E(&y, big_endian);
		file->readFloat32E(&z, big_endian);
	}

	void Vector3::readNormal360(File *file, bool big_endian) {
		unsigned int value=0;
		file->readInt32E(&value, big_endian);

		x = ((value&0x00000400 ? -1 : 0) + (float)((value>>2)&0x0FF)  / 256.0f);
		y = ((value&0x00200000 ? -1 : 0) + (float)((value>>13)&0x0FF) / 256.0f);
		z = ((value&0x80000000 ? -1 : 0) + (float)((value>>23)&0x0FF) / 256.0f);
	}

	void Vector3::write(File *file, bool big_endian) {
		if (big_endian) {
			file->writeFloat32BE(&x);
			file->writeFloat32BE(&y);
			file->writeFloat32BE(&z);
		}
		else {
			file->writeFloat32(&x);
			file->writeFloat32(&y);
			file->writeFloat32(&z);
		}
	}

	void Vector3::readXML(TiXmlElement *root) {
		for (TiXmlElement *pElem=root->FirstChildElement(); pElem; pElem=pElem->NextSiblingElement()) {
			string element_name=pElem->ValueStr();
			char *text_ptr=(char *) pElem->GetText();

			if (text_ptr) {
				if      (element_name == LIBGENS_MATH_AXIS_X_TEXT) FromString<float>(x, ToString(text_ptr), std::dec);
				else if (element_name == LIBGENS_MATH_AXIS_Y_TEXT) FromString<float>(y, ToString(text_ptr), std::dec);
				else if (element_name == LIBGENS_MATH_AXIS_Z_TEXT) FromString<float>(z, ToString(text_ptr), std::dec);
			}
		}
	}

	void Vector3::readSingleXML(TiXmlElement *root) {
		char *text_ptr=(char *) root->GetText();

		if (text_ptr) {
			sscanf(text_ptr, "%f %f %f", &x, &y, &z);
		}
	}

	void Vector3::writeXML(TiXmlElement *root) {
		TiXmlElement* xRoot=new TiXmlElement(LIBGENS_MATH_AXIS_X_TEXT);
		TiXmlText* xValue=new TiXmlText(ToString(x));
		xRoot->LinkEndChild(xValue);
		root->LinkEndChild(xRoot);

		TiXmlElement* yRoot=new TiXmlElement(LIBGENS_MATH_AXIS_Y_TEXT);
		TiXmlText* yValue=new TiXmlText(ToString(y));
		yRoot->LinkEndChild(yValue);
		root->LinkEndChild(yRoot);

		TiXmlElement* zRoot=new TiXmlElement(LIBGENS_MATH_AXIS_Z_TEXT);
		TiXmlText* zValue=new TiXmlText(ToString(z));
		zRoot->LinkEndChild(zValue);
		root->LinkEndChild(zRoot);
	}


	void Color::read(File *file, bool big_endian) {
		if (big_endian) {
			file->readFloat32BE(&r);
			file->readFloat32BE(&g);
			file->readFloat32BE(&b);
			file->readFloat32BE(&a);
		}
		else {
			file->readFloat32(&r);
			file->readFloat32(&g);
			file->readFloat32(&b);
			file->readFloat32(&a);
		}
	}

	void Color::write(File *file, bool big_endian) {
		if (big_endian) {
			file->writeFloat32BE(&r);
			file->writeFloat32BE(&g);
			file->writeFloat32BE(&b);
			file->writeFloat32BE(&a);
		}
		else {
			file->writeFloat32(&r);
			file->writeFloat32(&g);
			file->writeFloat32(&b);
			file->writeFloat32(&a);
		}
	}

	void Color::readARGB8(File *file) {
		unsigned char a_c=0;
		unsigned char r_c=0;
		unsigned char g_c=0;
		unsigned char b_c=0;

		file->readUChar(&a_c);
		file->readUChar(&r_c);
		file->readUChar(&g_c);
		file->readUChar(&b_c);

		a = ((float) a_c) / LIBGENS_MATH_COLOR_CHAR;
		r = ((float) r_c) / LIBGENS_MATH_COLOR_CHAR;
		g = ((float) g_c) / LIBGENS_MATH_COLOR_CHAR;
		b = ((float) b_c) / LIBGENS_MATH_COLOR_CHAR;
	}

	void Color::readABGR8(File *file) {
		unsigned char a_c=0;
		unsigned char r_c=0;
		unsigned char g_c=0;
		unsigned char b_c=0;

		file->readUChar(&a_c);
		file->readUChar(&b_c);
		file->readUChar(&g_c);
		file->readUChar(&r_c);

		a = ((float) a_c) / LIBGENS_MATH_COLOR_CHAR;
		r = ((float) r_c) / LIBGENS_MATH_COLOR_CHAR;
		g = ((float) g_c) / LIBGENS_MATH_COLOR_CHAR;
		b = ((float) b_c) / LIBGENS_MATH_COLOR_CHAR;
	}

	void Color::readRGBA8(File *file) {
		unsigned char a_c=0;
		unsigned char r_c=0;
		unsigned char g_c=0;
		unsigned char b_c=0;

		
		file->readUChar(&r_c);
		file->readUChar(&g_c);
		file->readUChar(&b_c);
		file->readUChar(&a_c);

		a = ((float) a_c) / LIBGENS_MATH_COLOR_CHAR;
		r = ((float) r_c) / LIBGENS_MATH_COLOR_CHAR;
		g = ((float) g_c) / LIBGENS_MATH_COLOR_CHAR;
		b = ((float) b_c) / LIBGENS_MATH_COLOR_CHAR;
	}

	void Color::writeARGB8(File *file) {
		unsigned char a_c=(unsigned char)(a*LIBGENS_MATH_COLOR_CHAR);
		unsigned char r_c=(unsigned char)(r*LIBGENS_MATH_COLOR_CHAR);
		unsigned char g_c=(unsigned char)(g*LIBGENS_MATH_COLOR_CHAR);
		unsigned char b_c=(unsigned char)(b*LIBGENS_MATH_COLOR_CHAR);

		file->writeUChar(&a_c);
		file->writeUChar(&r_c);
		file->writeUChar(&g_c);
		file->writeUChar(&b_c);
	}

	void Quaternion::read(File *file) {
		file->readFloat32BE(&x);
		file->readFloat32BE(&y);
		file->readFloat32BE(&z);
		file->readFloat32BE(&w);
	}

	void Quaternion::write(File *file) {
		file->writeFloat32BE(&x);
		file->writeFloat32BE(&y);
		file->writeFloat32BE(&z);
		file->writeFloat32BE(&w);
	}

	void Quaternion::fromAngleAxis(float rfAngle, Vector3 rkAxis) {
        float fHalfAngle ( 0.5*rfAngle );
        float fSin = sin(fHalfAngle);
        w = cos(fHalfAngle);
        x = fSin*rkAxis.x;
        y = fSin*rkAxis.y;
        z = fSin*rkAxis.z;
    }

	 Quaternion Quaternion::slerp(float fT, const Quaternion& rkP, const Quaternion& rkQ, bool shortestPath) {
        float fCos = rkP.dot(rkQ);
        Quaternion rkT;

        if (fCos < 0.0f && shortestPath) {
            fCos = -fCos;
            rkT = -rkQ;
        }
        else {
            rkT = rkQ;
        }

        if (abs(fCos) < 1 - LIBGENS_MATH_EPSION_QUATERNION) {
            float fSin = sqrt(1 - pow(fCos, 2));
            float fAngle = atan2(fSin, fCos);
            float fInvSin = 1.0f / fSin;
            float fCoeff0 = sin((1.0f - fT) * fAngle) * fInvSin;
            float fCoeff1 = sin(fT * fAngle) * fInvSin;
            return (rkP * fCoeff0) + (rkT * fCoeff1);
        }
        else {
            Quaternion t = (rkP * (1.0f - fT))  + (rkT * fT);
            t.normalise();
            return t;
        }
    }

	float Quaternion::getYaw() {
		float fTx  = 2.0f*x;
		float fTy  = 2.0f*y;
		float fTz  = 2.0f*z;
		float fTwy = fTy*w;
		float fTxx = fTx*x;
		float fTxz = fTz*x;
		float fTyy = fTy*y;

		return atan2(fTxz+fTwy, 1.0f-(fTxx+fTyy));
	}

	float Quaternion::getYawDegrees() {
		return getYaw() * LIBGENS_MATH_RAD_TO_DEGREE;
	}

	void Quaternion::readXML(TiXmlElement *root) {
		for(TiXmlElement *pElem=root->FirstChildElement(); pElem; pElem=pElem->NextSiblingElement()) {
			string element_name=pElem->ValueStr();
			char *text_ptr=(char *) pElem->GetText();

			if (text_ptr) {
				if      (element_name == LIBGENS_MATH_AXIS_X_TEXT) FromString<float>(x, ToString(text_ptr), std::dec);
				else if (element_name == LIBGENS_MATH_AXIS_Y_TEXT) FromString<float>(y, ToString(text_ptr), std::dec);
				else if (element_name == LIBGENS_MATH_AXIS_Z_TEXT) FromString<float>(z, ToString(text_ptr), std::dec);
				else if (element_name == LIBGENS_MATH_AXIS_W_TEXT) FromString<float>(w, ToString(text_ptr), std::dec);
			}
		}
	}

	void Quaternion::readSingleXML(TiXmlElement *root) {
		char *text_ptr=(char *) root->GetText();

		if (text_ptr) {
			sscanf(text_ptr, "%f %f %f %f", &x, &y, &z, &w);
		}
	}

	void Quaternion::writeXML(TiXmlElement *root) {
		TiXmlElement* xRoot=new TiXmlElement(LIBGENS_MATH_AXIS_X_TEXT);
		TiXmlText* xValue=new TiXmlText(ToString(x));
		xRoot->LinkEndChild(xValue);
		root->LinkEndChild(xRoot);

		TiXmlElement* yRoot=new TiXmlElement(LIBGENS_MATH_AXIS_Y_TEXT);
		TiXmlText* yValue=new TiXmlText(ToString(y));
		yRoot->LinkEndChild(yValue);
		root->LinkEndChild(yRoot);

		TiXmlElement* zRoot=new TiXmlElement(LIBGENS_MATH_AXIS_Z_TEXT);
		TiXmlText* zValue=new TiXmlText(ToString(z));
		zRoot->LinkEndChild(zValue);
		root->LinkEndChild(zRoot);

		TiXmlElement* wRoot=new TiXmlElement(LIBGENS_MATH_AXIS_W_TEXT);
		TiXmlText* wValue=new TiXmlText(ToString(w));
		wRoot->LinkEndChild(wValue);
		root->LinkEndChild(wRoot);
	}

	void Matrix4::read(File *file, bool big_endian) {
		for (size_t x=0; x<4; x++) {
			for (size_t y=0; y<4; y++) {
				if (big_endian) file->readFloat32BE(&m[x][y]);
				else file->readFloat32(&m[x][y]);
			}
		}
	}

	void Matrix4::write(File *file, bool big_endian) {
		for (size_t x=0; x<4; x++) {
			for (size_t y=0; y<4; y++) {
				if (big_endian) file->writeFloat32BE(&m[x][y]);
				else file->writeFloat32(&m[x][y]);
			}
		}
	}

	Matrix4 convertBoneMatrixToRegular(Matrix4 m) {
		Matrix3 m3;
		m.extract3x3Matrix(m3);
		Vector3 position=Vector3(m[0][3], m[1][3], m[2][3]);
		position = position * m3;
		position = position * (-1);
		Vector3 pos, scal;
		Quaternion rot;
		m.decomposition(pos, scal, rot);
		m.makeTransform(position, scal, rot);
		return m;
	}

	void AABB::read(File *file) {
		if (!file) {
			Error::addMessage(Error::NULL_REFERENCE, LIBGENS_MATH_ERROR_MESSAGE_NULL_FILE);
			return;
		}

		size_t header_address=file->getCurrentAddress();

		file->readFloat32BE(&start.x);
		file->readFloat32BE(&end.x);
		file->readFloat32BE(&start.y);
		file->readFloat32BE(&end.y);
		file->readFloat32BE(&start.z);
		file->readFloat32BE(&end.z);
	}

	void AABB::write(File *file) {
		if (!file) {
			Error::addMessage(Error::NULL_REFERENCE, LIBGENS_MATH_ERROR_MESSAGE_WRITE_NULL_FILE);
			return;
		}

		size_t header_address=file->getCurrentAddress();

		file->writeFloat32BE(&start.x);
		file->writeFloat32BE(&end.x);
		file->writeFloat32BE(&start.y);
		file->writeFloat32BE(&end.y);
		file->writeFloat32BE(&start.z);
		file->writeFloat32BE(&end.z);
	}


	float AABB::centerX() {
		return (start.x+end.x)/2.0f;
	}

	float AABB::centerY() {
		return (start.y+end.y)/2.0f;
	}

	float AABB::centerZ() {
		return (start.z+end.z)/2.0f;
	}

	float AABB::sizeX() {
		return end.x-start.x;
	}

	float AABB::sizeY() {
		return end.y-start.y;
	}

	float AABB::sizeZ() {
		return end.z-start.z;
	}

	float AABB::sizeMax() {
		float max_size=sizeX();
		float temp_size=sizeY();
		if (temp_size > max_size) max_size = temp_size;

		temp_size=sizeZ();
		if (temp_size > max_size) max_size = temp_size;

		return max_size;
	}

	float AABB::size() {
		return sizeX() * sizeY() * sizeZ();
	}

	Vector3 AABB::center() {
		return (start+end)/2.0f;
	}

	Vector3 AABB::corner(int type) {
		switch (type) {
			case 0 :
				return Vector3(start.x, start.y, start.z);
			case 1 :
				return Vector3(start.x, start.y, end.z);
			case 2 :
				return Vector3(start.x, end.y, start.z);
			case 3 :
				return Vector3(start.x, end.y, end.z);
			case 4 :
				return Vector3(end.x, start.y, start.z);
			case 5 :
				return Vector3(end.x, start.y, end.z);
			case 6 :
				return Vector3(end.x, end.y, start.z);
			case 7 :
				return Vector3(end.x, end.y, end.z);
		}

		return Vector3(0, 0, 0);
	}

	AABB AABB::half(int axis, int side) {
		AABB result=*this;

		switch (axis) {
			case LIBGENS_MATH_AXIS_X :
				if (side == LIBGENS_MATH_SIDE_LEFT) result.end.x = result.centerX();
				else if (side == LIBGENS_MATH_SIDE_RIGHT) result.start.x = result.centerX();
				break;
			case LIBGENS_MATH_AXIS_Y :
				if (side == LIBGENS_MATH_SIDE_LEFT) result.end.y = result.centerY();
				else if (side == LIBGENS_MATH_SIDE_RIGHT) result.start.y = result.centerY();
				break;
			case LIBGENS_MATH_AXIS_Z :
				if (side == LIBGENS_MATH_SIDE_LEFT) result.end.z = result.centerZ();
				else if (side == LIBGENS_MATH_SIDE_RIGHT) result.start.z = result.centerZ();
				break;
			default :
				result.start = Vector3();
				result.end = Vector3();
				break;
		}

		return result;
	}

	bool AABB::intersects(const AABB& aabb) {
		if (end.x < aabb.start.x) return false;
		if (end.y < aabb.start.y) return false;
		if (end.z < aabb.start.z) return false;
		if (start.x > aabb.end.x) return false;
		if (start.y > aabb.end.y) return false;
		if (start.z > aabb.end.z) return false;

		return true;
	}

	bool AABB::intersects(const Vector3& v) {
		return ((v.x >= start.x)  &&  (v.x <= end.x)  && (v.y >= start.y)  &&  (v.y <= end.y)  && (v.z >= start.z)  &&  (v.z <= end.z));
	}

	void AABB::merge(const AABB& aabb) {
		if (aabb.start.x < start.x) start.x = aabb.start.x;
		if (aabb.start.y < start.y) start.y = aabb.start.y;
		if (aabb.start.z < start.z) start.z = aabb.start.z;

		if (aabb.end.x > end.x) end.x = aabb.end.x;
		if (aabb.end.y > end.y) end.y = aabb.end.y;
		if (aabb.end.z > end.z) end.z = aabb.end.z;
	}

	int Vector3::relativeCorner(Vector3 &v) {
		if (v.x <= x) {
			if (v.y <= y) {
				if (v.z <= z) return 0;
				else return 1;
			}
			else {
				if (v.z <= z) return 2;
				else return 3;
			}
		}
		else {
			if (v.y <= y) {
				if (v.z <= z) return 4;
				else return 5;
			}
			else {
				if (v.z <= z) return 6;
				else return 7;
			}
		}

		return 0;
	}

	Vector2 Vector3::toBarycentric(Vector3 &a, Vector3 &b, Vector3 &c) {
		Vector2 uv;
	    Vector3 v0 = c - a;
	    Vector3 v1 = b - a;
	    Vector3 v2 = *this - a;

	    float dot00 = v0.dotProduct(v0);
	    float dot01 = v0.dotProduct(v1);
	    float dot02 = v0.dotProduct(v2);
	    float dot11 = v1.dotProduct(v1);
		float dot12 = v1.dotProduct(v2);

		float invDenom = 1.f / ((dot00 * dot11) - (dot01 * dot01));
		uv.x = (dot11 * dot02 - dot01 * dot12) * invDenom;
		uv.y = (dot00 * dot12 - dot01 * dot02) * invDenom;

		return uv;
	}

	bool Vector3::sameSide(Vector3 &p2, Vector3 &a, Vector3 &b) {
		Vector3 cp1 = (b-a).crossProduct(*this-a);
		Vector3 cp2 = (b-a).crossProduct(p2-a);
		if (cp1.dotProduct(cp2) >= 0) return true;
		else return false;
	}

	bool Triangle::intersects(Vector3 &p) {
		if (p.sameSide(point[0], point[1], point[2]) && p.sameSide(point[1], point[0], point[2]) && p.sameSide(point[2], point[0], point[1])) return true;
		else return false;
	}

	void Triangle::generatePoints(vector<Vector3> *list, float unit_size) {
		if (degenerate()) return;

		float area=getArea();
		Vector3 cent=center();

		list->push_back(cent);
		
		if ((getArea() < unit_size) || (lengths[0] < unit_size) || (lengths[1] < unit_size) || (lengths[2] < unit_size)) {
			return;
		}
		else {
			size_t step=1;

			while (true) {
				float radius=step*unit_size;
				float perimeter=2*radius*LIBGENS_MATH_PI;
				float angle=0;
				int circle_count = (int)(perimeter/(unit_size));
				float angle_step=(LIBGENS_MATH_PI/(float)circle_count)*2;
				size_t circles=0;

				for (int i=0; i<circle_count; i++) {
					Vector3 point=cent + tangent*cos(angle)*radius + bitangent*sin(angle)*radius;
					if (intersects(point)) {
						list->push_back(point);
						circles++;
					}
					angle+=angle_step;
				}

				if (!circles) {
					break;
				}

				step++;
			}
		}
    }
};