#ifndef VECTOR_H_INCLUDED
#define VECTOR_H_INCLUDED

#include <math.h>

class Vector { 
  float x,y,z;
 public:
  /*
    inline float X() const {return x;}
    inline float Y() const {return y;}
    inline float Z() const {return z;}
  */
  inline Vector(float xx=0.0,float yy=0.0,float zz=0.0) : x(xx),y(yy),z(zz) {}
  inline Vector(float *v) : x(v[0]),y(v[1]),z(v[2]) {}
  inline Vector(const Vector &v) : x(v.x),y(v.y),z(v.z) {}
  inline void Set(float xx,float yy,float zz) {x=xx;y=yy;z=zz;}
  inline Vector &Normalize() {
    float l=1.0f/sqrt(x*x+y*y+z*z);
    x*=l;
    y*=l;
    z*=l;
    return *this;
  }
  inline float Dot(const Vector &v) const {
    return(x*v.x+y*v.y+z*v.z);
  }
  inline Vector Cross(const Vector &v) const {
    return Vector(y*v.z-v.y*z,v.x*z-x*v.z,x*v.y-v.x*y);
  }
  inline float Length() const {
    return sqrt(x*x+y*y+z*z);
  }
  inline float LengthSq() const {
    return x*x+y*y+z*z;
  }
  inline const float &operator[](const unsigned int i) const {return (&x)[i];} 
  inline Vector operator-(const Vector &v) const {
    return Vector(x-v.x,y-v.y,z-v.z);
  }
  inline Vector operator-() const {
    return Vector(-x,-y,-z);
  }
  inline Vector operator+(const Vector &v) const {
    return Vector(x+v.x,y+v.y,z+v.z);
  }
  inline Vector operator*(const float n) const {
    return Vector(x*n,y*n,z*n);
  }
  inline Vector operator*(const Vector v) const {
    return Vector(x*v.x,y*v.y,z*v.z);
  }
  inline Vector operator/(const float n) const {
    float ndiv=1.0f/n;
    return Vector(x*ndiv,y*ndiv,z*ndiv);
  }
  inline Vector &operator-=(const Vector &v) {
    x-=v.x;
    y-=v.y;
    z-=v.z;
    return *this;
  }
  inline Vector &operator+=(const Vector &v) {
    x+=v.x;
    y+=v.y;
    z+=v.z;
    return *this;
  }
  inline Vector &operator*=(const float n) {
    x*=n;
    y*=n;
    z*=n;
    return *this;
  }
  inline Vector &operator*=(const Vector &v) {
    x*=v.x;
    y*=v.y;
    z*=v.z;
    return *this;
  }
  inline Vector &operator/=(const float n) {
    float ndiv=1.0f/n;
    x*=ndiv;
    y*=ndiv;
    z*=ndiv;
    return *this;
  }
  inline Vector &operator=(const Vector &v) {
    x=v.x;
    y=v.y;
    z=v.z;
    return *this;
  }
  inline friend Vector operator*(const float &f,const Vector &v) {
    return v*f;
  }
};

#endif
