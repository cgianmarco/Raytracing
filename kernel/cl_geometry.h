

 void intersect(constant const cl_Triangle * triangle, const float3 * origin, const float3 * direction, float3 * coords){


 	float3 edge0 = triangle -> v1 - triangle -> v0;
 	float3 edge1 = triangle -> v2 - triangle -> v0;


 	float3 pvec = cross(*direction, edge1);
	float det = dot(edge0, pvec);
	

	if(det < EPSILON){
		*coords = FAR_AWAY;
		return;
		
	}

	float invDet = 1 / det;




	float3 tvec = *origin - triangle -> v0;
	float u = dot(tvec, pvec) * invDet;



	if((u < 0) || u > 1){
		*coords = FAR_AWAY;
		return;
	}





	float3 qvec = cross(tvec, edge0);
	float v = dot(*direction, qvec) * invDet;
	if((v < 0) || (u + v > 1)){
		*coords = FAR_AWAY;
		return;
	}



	float t = dot(edge1, qvec) * invDet;

	

	*coords = (float3){u, v, t};


 }


 float3 normal_at_point(constant const cl_Triangle * triangle, float u, float v, float t){

	float3 norm = normalize((1 - u - v) * triangle -> vn0 + u * triangle -> vn1 + v * triangle -> vn2);
	//float3 norm = normalize((triangle -> vn0 + triangle -> vn1 + triangle -> vn2)/3.0f);

	return norm;
}

bool near_zero(float3 vec) {
    // Return true if the vector is close to zero in all dimensions.
    const float s = 1e-8;
    return (fabs(vec.x) < s) && (fabs(vec.y) < s) && (fabs(vec.z) < s);
}

float3 reflect(float3 v, float3 n) {
    return v - 2 * dot(v,n) * n;
}

float3 refract(float3 uv, float3 n, float etai_over_etat) {
    float cos_theta = fmin(dot(-uv, n), 1.0f);
    float3 r_out_perp =  etai_over_etat * (uv + cos_theta*n);
    float3 r_out_parallel = -sqrt(fabs(1.0f - pow(length(r_out_perp), 2))) * n;
    return r_out_perp + r_out_parallel;
}

float reflectance(float cosine, float ref_idx) {
    // Use Schlick's approximation for reflectance.
    float r0 = (1-ref_idx) / (1+ref_idx);
    r0 = r0*r0;
    return r0 + (1-r0)*pow((1 - cosine),5);
}