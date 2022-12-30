#include "kernel/constants.h"

#include "kernel/types.h"
#include "common/common.h"

#include "kernel/cl_random.h"
#include "kernel/cl_geometry.h"
#include "kernel/cl_hit.h"
#include "kernel/cl_ray.h"








__kernel void raytrace(global float3 * data,
	constant const cl_Scene * scene,
	constant const cl_Triangle * triangles,
	constant const int * n_triangles,
	constant const clBVHNode * nodes,
	constant const int * n_nodes,
	constant const int * batch_i,
	constant const int * batch_j,
	constant const int * batch_sample,
	constant const cl_Material * materials,
	global ProfilingInfo * profilingInfo){



	int i = (*batch_i) + get_global_id(0);
	int j = scene -> height - (*batch_j) - get_global_id(1) - 1;
	int sample = (*batch_sample) + get_global_id(2);

	uint seed = (sample + ((j * (scene -> width) + i) * 10)) * (get_global_id(0) + get_global_id(1) + sample);

	float u = (i + randRange(&seed, 0, 1)) / (float)(scene -> width - 1);
	float v = (j + randRange(&seed, 0, 1)) / (float)(scene -> height - 1);


	Ray ray;
	ray.origin = scene -> pos;
	ray.direction = normalize(scene -> lower_left_corner + u * scene -> horizontal + v * scene -> vertical - ray.origin);


	float3 color = 0.0;
	float3 throughput = 1.0;

	for(int n=0; n<MAX_DEPTH; n++){

		profilingInfo -> rays += 1;


		HitInfo hit;

		if(!intersect_any(ray.origin, ray.direction, triangles, *n_triangles, nodes, *n_nodes, &hit, profilingInfo)){
			float3 unit_direction = normalize(ray.direction);
			float t = 0.5 * (unit_direction.y + 1.0);
			float3 background = (1.0f - t) * WHITE + t * BACKGROUND_COLOR;

			color += throughput * background;
			break;
		}



		cl_Material material = materials[triangles[hit.objectId].material_id];
		float3 new_origin = hit.hitPoint + hit.normal * BIAS;

		if(material.type == kLambertian){
			float3 scatter_direction = hit.normal + normalize(random_in_unit_sphere(&seed));

			if(near_zero(scatter_direction))
				scatter_direction = hit.normal;


			ray.origin = new_origin;
			ray.direction = scatter_direction;

			throughput *= material.albedo;
		}



		if(material.type  == kMetallic){

			float3 reflected = reflect(normalize(ray.direction), hit.normal);
			float3 scatter_direction = normalize(reflected) + material.value * random_in_unit_sphere(&seed);

			if(dot(scatter_direction, hit.normal) > 0){

				ray.origin = new_origin;
				ray.direction = scatter_direction;
				throughput *= material.albedo;

			}else{

				color = 0.0;
				break;

			}

		}



		if(material.type == kDieletric){

			float front_face = dot(ray.direction, hit.normal) < 0;
			float ir = material.value;

			float refraction_ratio;
			if(front_face)
				refraction_ratio = 1.0f/ir;
			else
				refraction_ratio = ir;

			float3 unit_direction = normalize(ray.direction);
			float cos_theta = fmin(dot(-unit_direction, hit.normal), 1.0f);
			float sin_theta = sqrt(1.0f - cos_theta*cos_theta);

			bool cannot_refract = refraction_ratio * sin_theta > 1.0;
			float3 scatter_direction;

			if (cannot_refract  || reflectance(cos_theta, refraction_ratio) > randRange(&seed, 0, 1))
				scatter_direction = reflect(unit_direction, hit.normal);
			else
				scatter_direction = refract(unit_direction, hit.normal, refraction_ratio);


			ray.origin = new_origin;
			ray.direction = scatter_direction;

			throughput *= material.albedo;
		}



		if(material.type == kDiffuseLight){

			color += throughput * material.albedo;
			throughput *= (float3){0.2, 0.4, 0.6};
			break;

		}


		//Russian Roulette
		//Randomly terminate a path with a probability inversely equal to the throughput
		//float p = clamp(max(throughput.x, max(throughput.y, throughput.z)), 0.0f, 1.0f);
		float p = 0.7;
		if (randRange(&seed, 0, 1) > p) {
			break;
		}

		// Add the energy we 'lose' by randomly terminating paths
		throughput *= 1 / p;


	}



	/* Return pixel color */

	data[j * (scene -> width) + i] += color;




}