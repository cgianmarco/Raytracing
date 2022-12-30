

static cl_float clamp(cl_float x, cl_float min, cl_float max) {
    if (x < min) return min;
    if (x > max) return max;
    return x;
}


static void writeToFile(const std::string &filename, cl_float3 * data, int width, int height, int num_samples){


    std::ofstream ofs; 
    ofs.open(filename);

    ofs << "P6\n" << width << " " << height << "\n255\n";

    for(int j=height-1; j >=0; --j){
        for(int i=0; i < width; ++i){

        	cl_uchar3 color;
    		color.x = (cl_uchar) (256.0 * clamp(sqrt(data[j * width + i].x / float(num_samples)), 0, 0.999));
    		color.y = (cl_uchar) (256.0 * clamp(sqrt(data[j * width + i].y / float(num_samples)), 0, 0.999));
    		color.z = (cl_uchar) (256.0 * clamp(sqrt(data[j * width + i].z / float(num_samples)), 0, 0.999));

    		// data[j * (scene -> width) + i] = color;

            unsigned char r = color.x;
            unsigned char g = color.y;
            unsigned char b = color.z;
            ofs << r << g << b;

        }
    }
}


static cl_float deg2rad(float deg) 
{ 
    return deg * 3.1415 / 180; 
}


static cl_float3 cl_subtract3f(cl_float3 first, cl_float3 second){

    cl_float3 result;

    result.x = first.x - second.x;
    result.y = first.y - second.y;
    result.z = first.z - second.z;

    return result;

}

cl_float3 vec3ToCL(const glm::vec3 &vector){

    return (cl_float3){vector.x, vector.y, vector.z};

}

void printVec(const cl_float3 &vec){

    std::cout << vec.x << ", " << vec.y << ", " << vec.z << std::endl;

}

void print(const cl_float3 &vec){

    std::cout << vec.x << ", " << vec.y << ", " << vec.z << std::endl;

}

float_ cl_vec_at(const float3_ &vec, int i){

    float_ e;

    if(i == 0)
        e = vec.x;
    if(i == 1)
        e = vec.y;
    if(i == 2)
        e = vec.z;

    return e;

}

float3_ add(float3_ a, float3_ b){

    return { a.x + b.x, a.y + b.y, a.z + b.z };

}

float3_ divide(float3_ a, float_ b){

    return (float3_){ a.x / b, a.y / b, a.z / b };

}

float3_ pow(float3_ a, float_ b){

    return (float3_){ pow(a.x, b), pow(a.y, b), pow(a.z, b) };

}

int_ max_index(float3_ a){

    int_ max_index = 0;

    for(int i=1; i<3; i++)
        if(cl_vec_at(a, i) > cl_vec_at(a, max_index))
            max_index = i;


    return max_index;
}



