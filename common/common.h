#ifndef _COMMONH_
#define _COMMONH_


typedef struct __attribute__ ((packed)) cl_Scene{

	int_ width;
	int_ height;
	int_ batch_i;
	int_ batch_j;
	
	float3_ pos;
	float3_ lower_left_corner;
	float3_ vertical;
	float3_ horizontal;


} cl_Scene;


typedef struct __attribute__ ((packed)) cl_ObjectInfo{

	int_ first_index;
	int_ last_index;

	int_ use_bbox;
	float3_ bbstart;
	float3_ bbend;


} cl_ObjectInfo;


typedef enum MaterialType { kLambertian, kMetallic, kDieletric, kDiffuseLight } MaterialType; 

typedef struct __attribute__ ((packed)) cl_Material{

	MaterialType type;
    float3_ albedo;
    float_ value;

} cl_Material;


typedef struct __attribute__ ((packed)) cl_Triangle{

 	float3_ v0;
 	float3_ v1;
 	float3_ v2;

 	float3_ vn0;
 	float3_ vn1;
 	float3_ vn2;
 	int_ material_id;


 } cl_Triangle;



typedef struct __attribute__ ((packed)) ProfilingInfo{

	int_ bbox_intersections;
	int_ triangle_intersections;
	int_ rays;
	int_ max_depth;

} ProfilingInfo;


typedef struct __attribute__ ((packed)) clBVHNode{

	int_ right, left;
	float3_ bbstart;
	float3_ bbend;
	int_ start, end;

} clBVHNode;


typedef struct __attribute__ ((packed)) Queue{

	int_ values[5000];
	int_ size;

} Queue;


void enqueue(int_ i, Queue * queue){

	queue -> values[queue -> size] = i;
	queue -> size += 1;

}

 #endif