#include "geometry/ray3.hpp"

#include "geometry/geometry.hpp"

#include <cmath>

namespace cg
{

Ray3::Ray3() : o{0.0f, 0.0f, 0.0f}, d{1.0f, 0.0f, 0.0f} {}

Ray3::Ray3(const Point3 &p1, const Point3 &p2, bool normalize)
{
   o = p1;
   d = p2 - p1;
   if(normalize)
      d.normalize();
}

Ray3::Ray3(const Point3 &origin, const Vector3 &dir) : o(origin), d(dir) {}

Ray3::Ray3(const Point3 &origin, const Vector3 &dir, bool normalize)
{
    o = origin;
    d = dir;
    if(normalize) { d.normalize(); }
}

Ray3 Ray3::reflect(const Point3 &int_pt, const Vector3 &n) const
{
    // Required in 605.767
    return Ray3();
}

RayRefractionResult Ray3::refract(const Point3 &int_pt, Vector3 &n, float u1, float u2) const
{
    // Required in 605.767
    return RayRefractionResult{};
}

Point3 Ray3::intersect(float t) const
{
    // Student to define - Module 5
    return o + d * t;
}

RayObjectIntersectResult Ray3::intersect(const Plane &p) const
{
    // Required in 605.767
    return RayObjectIntersectResult{false, 0.0f};
}
RayObjectIntersectResult Ray3::intersect(const BoundingSphere &sphere) const
{
   //vector from ray origin to sphere center
   Vector3 oc = sphere.center - o;
   
   // project oc onto the ray direction to find the closest point 
   float t_closest = oc.dot(d);

   //find the closest point on the ray to the sphere center 
   Point3 closest_point = o + d * t_closest;

   //Calculate the perpendicular distance from sphere center to ray 
   Vector3 perp = sphere.center - closest_point;
   float perp_dis_square = perp.dot(perp);
   float radius_sq = sphere.radius * sphere.radius;

   //back out if perpendicular distance > radius 
   if(perp_dis_square > radius_sq)
      return { false, 0.0f};

   //use pythagorean for the half chord
   float half_chord = std::sqrt(radius_sq - perp_dis_square);

   //caculate the two intersect points along the ray 
   float t1 = t_closest - half_chord; //near
   float t2 = t_closest + half_chord; //far 

   //return the nearest intersection that is in front of the ray 
   if(t1 >= 0.0f)
      return {true, t1};
   else if(t2 >= 0.0f)
      return {true, t2};
   else 
      return {false, 0.0f};
}

RayObjectIntersectResult Ray3::intersect(const AABB &box) const
{
    // Required in 605.767
    return {false, 0.0f};
}

RayObjectIntersectResult Ray3::intersect(const std::vector<Point3> &polygon,
                                         const Vector3             &normal) const
{
    // Required in 605.767
    return {false, 0.0f};
}

RayTriangleIntersectResult
    Ray3::intersect(const Point3 &v0, const Point3 &v1, const Point3 &v2) const
{
    // Required in 605.767
    return {false, 0.0f, 0.0f, 0.0f};
}

bool Ray3::does_intersect_exist(const Point3 &v0, const Point3 &v1, const Point3 &v2) const
{
    // Required in 605.767
    return false;
}

RayMeshIntersectResult Ray3::intersect(const std::vector<Point3>   &vertex_list,
                                       const std::vector<uint16_t> &face_list,
                                       float                        t_min) const
{
    // Required in 605.767
    return {false, 0.0f, 0.0f, 0.0f, 0};
}

bool Ray3::does_intersect_exist(const std::vector<Point3>   &vertex_list,
                                const std::vector<uint16_t> &face_list,
                                float                        t_min) const
{
    // Required in 605.767
    return false;
}

bool Ray3::does_intersect_exist(const std::vector<VertexAndNormal> &vertex_list,
                                const std::vector<uint16_t>        &face_list,
                                float                               t_min) const
{
    // Required in 605.767
    return false;
}

} // namespace cg
