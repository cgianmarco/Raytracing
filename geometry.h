#include "types.h"
#include "common/common.h"




void createTriangle(cl_float3 vert0, cl_float3 vert1, cl_float3 vert2, 
                          cl_float3 vertNorm0, cl_float3 vertNorm1, cl_float3 vertNorm2, 
                          int mat, cl_Triangle &t){

    t.v0 = vert0;
    t.v1 = vert1;
    t.v2 = vert2;

    t.vn0 = vertNorm0;
    t.vn1 = vertNorm1;
    t.vn2 = vertNorm2;
    t.material_id = mat;

}


float3_ centroid(cl_Triangle &t){

    cl_float3 result = { (t.v0.x + t.v1.x + t.v2.x)/3,
                         (t.v0.y + t.v1.y + t.v2.y)/3,
                         (t.v0.z + t.v1.z + t.v2.z)/3, };

    return result;

}



// TO FIX

glm::vec3 wDivide(const glm::vec4 &vec){

    return glm::vec3(vec.x/vec.w, vec.y/vec.w, vec.z/vec.w);

}


void calculateTriangleBoundingBox(const cl_Triangle &triangle, clBVHNode &node){

  node.bbstart = (float3_) { std::min(triangle.v0.x, std::min(triangle.v1.x, triangle.v2.x)),
              std::min(triangle.v0.y, std::min(triangle.v1.y, triangle.v2.y)),
              std::min(triangle.v0.z, std::min(triangle.v1.z, triangle.v2.z)) };

  node.bbend = (float3_) { std::max(triangle.v0.x, std::max(triangle.v1.x, triangle.v2.x)),
              std::max(triangle.v0.y, std::max(triangle.v1.y, triangle.v2.y)),
              std::max(triangle.v0.z, std::max(triangle.v1.z, triangle.v2.z)) };

}


void extendBoundingBox(const cl_Triangle &triangle, clBVHNode &node){

  clBVHNode other;

  calculateTriangleBoundingBox(triangle, other);

  node.bbstart = (float3_) { std::min(node.bbstart.x, other.bbstart.x),
              std::min(node.bbstart.y, other.bbstart.y),
              std::min(node.bbstart.z, other.bbstart.z) };

  node.bbend = (float3_) { std::max(node.bbend.x, other.bbend.x),
              std::max(node.bbend.y, other.bbend.y),
              std::max(node.bbend.z, other.bbend.z) };


}



void calculateBoundingBox(const std::vector<cl_Triangle> &triangles, clBVHNode &node){

  calculateTriangleBoundingBox(triangles[0], node);

  for(int i=1; i<triangles.size(); i++)
    extendBoundingBox(triangles[i], node);

}


void getTriangleBoundingBox(const cl_Triangle &triangle, float3_ &bbstart, float3_ &bbend){

  bbstart = (float3_) { std::min(triangle.v0.x, std::min(triangle.v1.x, triangle.v2.x)),
              std::min(triangle.v0.y, std::min(triangle.v1.y, triangle.v2.y)),
              std::min(triangle.v0.z, std::min(triangle.v1.z, triangle.v2.z)) };

  bbend = (float3_) { std::max(triangle.v0.x, std::max(triangle.v1.x, triangle.v2.x)),
              std::max(triangle.v0.y, std::max(triangle.v1.y, triangle.v2.y)),
              std::max(triangle.v0.z, std::max(triangle.v1.z, triangle.v2.z)) };

}


void getExtendedBoundingBox(const cl_Triangle &triangle, float3_ &bbstart, float3_ &bbend){

  float3_ other_bbstart, other_bbend;
  getTriangleBoundingBox(triangle, other_bbstart, other_bbend);

  bbstart = (float3_) { std::min(bbstart.x, other_bbstart.x),
              std::min(bbstart.y, other_bbstart.y),
              std::min(bbstart.z, other_bbstart.z) };

  bbend = (float3_) { std::max(bbend.x, other_bbend.x),
              std::max(bbend.y, other_bbend.y),
              std::max(bbend.z, other_bbend.z) };


}

void getBoundingBox(const std::vector<cl_Triangle> &triangles, float3_ &bbstart, float3_ &bbend){

  getTriangleBoundingBox(triangles[0], bbstart, bbend);

  for(int i=1; i<triangles.size(); i++)
    getExtendedBoundingBox(triangles[i], bbstart, bbend);

}

float getSurfaceArea(const std::vector<cl_Triangle> &triangles){

  float3_ bbstart, bbend;
  getBoundingBox(triangles, bbstart, bbend);

  float3_ size = cl_subtract3f(bbend, bbstart);
  return 2.0 * (size.x * size.y + size.x * size.z + size.y * size.z);


}






