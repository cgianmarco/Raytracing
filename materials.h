#include "types.h"
#include "common/common.h"


void createLambertian(cl_Material &mat, cl_float3 albedo){

	mat.type = kLambertian;
	mat.albedo = albedo;
}

void createMetallic(cl_Material &mat, cl_float3 albedo, cl_float fuzz){

	mat.type = kMetallic;
	mat.albedo = albedo;
	mat.value = fuzz;


}

void createDieletric(cl_Material &mat, cl_float3 albedo, cl_float index_of_refraction){
	mat.type = kDieletric;
	mat.albedo = albedo;
	mat.value = index_of_refraction;

}

void createDiffuseLight(cl_Material &mat, cl_float3 albedo){
	mat.type = kDiffuseLight;
	mat.albedo = albedo;

}