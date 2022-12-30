/* Hit */

typedef struct HitInfo{

	int objectId;
	float u;
	float v;
	float t;
	float3 hitPoint;
	float3 normal;

} HitInfo;





void swap(float * x, float * y){

	float temp = *y;
	*y = *x;
	*x = temp;

}


bool bbIntersect(float3 origin, float3 direction, constant const clBVHNode * object){


	float3 min = object -> bbstart;
	float3 max = object -> bbend;


	float tmin = (min.x - origin.x) / direction.x; 
    float tmax = (max.x - origin.x) / direction.x; 
 
    if (tmin > tmax) swap(&tmin, &tmax); 
 
    float tymin = (min.y - origin.y) / direction.y; 
    float tymax = (max.y - origin.y) / direction.y; 
 
    if (tymin > tymax) swap(&tymin, &tymax); 
 
    if ((tmin > tymax) || (tymin > tmax)) 
        return false; 
 
    if (tymin > tmin) 
        tmin = tymin; 
 
    if (tymax < tmax) 
        tmax = tymax; 
 
    float tzmin = (min.z - origin.z) / direction.z; 
    float tzmax = (max.z - origin.z) / direction.z; 
 
    if (tzmin > tzmax) swap(&tzmin, &tzmax); 
 
    if ((tmin > tzmax) || (tzmin > tmax)) 
        return false; 
 
    if (tzmin > tmin) 
        tmin = tzmin; 
 
    if (tzmax < tmax) 
        tmax = tzmax; 
 
    return true; 



}


// TO FIX
bool intersect_any(const float3 origin, const float3 direction, 
				   constant const cl_Triangle* triangles, 
				   const int n_triangles, 
				   constant const clBVHNode * nodes, 
				   const int n_nodes, 
				   HitInfo * closest,
				   global ProfilingInfo * profilingInfo){

	float_ closest_t = INF_DIST;
	int_ closest_id = -1;
	float3 coords;

	Queue queue;
	queue.size = 0;
	enqueue(0, &queue);

	int j=0;
	
	while(j < queue.size){

		// if(j > profilingInfo -> max_depth)
		// 	profilingInfo -> max_depth = j;




		//constant const clBVHNode * current = &nodes[queue.values[j]];

		//profilingInfo -> bbox_intersections += 1;

		if(bbIntersect(origin, direction, &nodes[queue.values[j]])){

			bool has_left = nodes[queue.values[j]].left != -1;
			bool has_right = nodes[queue.values[j]].right != -1;

			if(has_left)
				enqueue(nodes[queue.values[j]].left, &queue);

			if(has_right)
				enqueue(nodes[queue.values[j]].right, &queue);

			if((!has_left) && (!has_right)){
                     
				int start = nodes[queue.values[j]].start;
				int end = nodes[queue.values[j]].end;

				for(int i=start; i<end; i++){

					//profilingInfo -> triangle_intersections += 1;
					intersect(&triangles[i], &origin, &direction, &coords);

					if((coords.z < closest_t) && (coords.z > EPSILON) && (coords.z < INF_DIST)){

						closest_t = coords.z;
						closest_id = i;
					}
				}

			}
		}

		j += 1;
	}

	intersect(&triangles[closest_id], &origin, &direction, &coords);
	

	if(closest_t < INF_DIST){

		closest -> u = coords.x;
		closest -> v = coords.y;
		closest -> t = coords.z;

		closest -> objectId = closest_id;

		closest -> hitPoint = origin + closest -> t * direction;
		closest -> normal = normal_at_point(&triangles[closest -> objectId], closest -> u, closest -> v, closest -> t);
		return true;
	}

	return false;

}


