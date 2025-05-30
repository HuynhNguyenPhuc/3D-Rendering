#ifndef VEC3_H
#define VEC3_H

class Vec3 {
    public:
        float x, y, z;
        Vec3();
        Vec3(float value);
        Vec3(float x, float y, float z);
        float operator [] (int idx) const;
        float& operator [] (int idx);
        float length() const;
        bool operator == (const Vec3& v) const;
        bool operator != (const Vec3& v) const;
        Vec3 operator + (const Vec3& v) const;
        void operator += (const Vec3& v); 
        Vec3 operator - () const;
        Vec3 operator - (const Vec3& v) const;
        Vec3 operator * (float k) const;
        Vec3 operator * (const Vec3& v) const;
        Vec3 operator / (float k) const;
        float dot(const Vec3& v) const;
        Vec3 cross(const Vec3& v) const;
        Vec3 normalize() const;
};

#endif // VEC3_H