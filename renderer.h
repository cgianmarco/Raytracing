#include "types.h"
#include "common/common.h"
#include "external/OBJ_Loader.h"




class cl_Renderer{

public:

	cl_Scene scene;
	std::vector<cl_Triangle> triangles;
	std::vector<cl_ObjectInfo> objects;
	std::vector<cl_Material> materials;

	ProfilingInfo profilingInfo;

public:



	cl_Renderer(){

		srand(time(NULL));

		/* Init scene */
		float aspect_ratio = 16.0 / 9.0;
	    cl_int width = 1920;
	    cl_int height = (cl_int)(width / aspect_ratio);

	    std::cout << height << std::endl;

	    float viewport_height = 2.0;
	    float viewport_width = aspect_ratio * viewport_height;
	    float focal_length = 1.0;
	    glm::vec3 pos = glm::vec3(0.0);
	    glm::vec3 horizontal = glm::vec3(viewport_width, 0.0, 0.0 );
	    glm::vec3 vertical = glm::vec3( 0.0, viewport_height, 0.0 );
	    glm::vec3 lower_left_corner = pos - horizontal/2.0f - vertical/2.0f - glm::vec3(0, 0, focal_length);


	    scene.width = width;
	    scene.height = height;
	    scene.pos =  vec3ToCL(pos);
	    scene.horizontal = vec3ToCL(horizontal);
	    scene.vertical = vec3ToCL(vertical);
	    scene.lower_left_corner = vec3ToCL(lower_left_corner);
    
	}


	void render(){

		//get all platforms (drivers)
	    std::vector<cl::Platform> all_platforms;
	    cl::Platform::get(&all_platforms);

	    cl::Platform default_platform=all_platforms[0];
	 
	    //get default device of the default platform
	    std::vector<cl::Device> all_devices;
	    default_platform.getDevices(CL_DEVICE_TYPE_ALL, &all_devices);

	    cl::Device default_device=all_devices[0];
	 
	    cl::Context context({default_device});
	 
	    cl::Program::Sources sources;
	 
	    std::ifstream t("kernel/raytracing.cl");
		std::string kernel_code((std::istreambuf_iterator<char>(t)),
		                 		 std::istreambuf_iterator<char>());

	    sources.push_back({kernel_code.c_str(),kernel_code.length()});
	 
	    cl::Program program(context,sources);

	    if(program.build({default_device})!=CL_SUCCESS){
	        std::cout<<" Error building: "<<program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(default_device)<<"\n";
	        exit(1);
	    }


	    BVHTreeGenerator tgenerator(triangles);
	    std::cout << "triangles size is " << triangles.size() << std::endl;
	    triangles = tgenerator.triangles;
	    std::cout << "triangles size is " << triangles.size() << std::endl;

	    std::vector<clBVHNode> tree = tgenerator.nodes;



	    std::vector<cl_float3> image(scene.width * scene.height);

	    int n_triangles = triangles.size();
	    int n_nodes = tree.size();
	    std::cout << "triangles size is " << tree.size() << std::endl;

	    profilingInfo.triangle_intersections = 0;
	    profilingInfo.bbox_intersections = 0;
	    profilingInfo.rays = 0;
	    profilingInfo.max_depth = 0;


	    /* Create buffer */
	    cl::Buffer buffer_image(context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, sizeof(cl_float3) * image.size(), &image[0]);
	    cl::Buffer buffer_scene(context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, sizeof(cl_Scene), &scene);
	    cl::Buffer buffer_triangles(context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, sizeof(cl_Triangle) * triangles.size(), &triangles[0]);
	    cl::Buffer buffer_n_triangles(context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, sizeof(cl_int), &n_triangles);
	    cl::Buffer buffer_nodes(context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, sizeof(clBVHNode) * tree.size(), &tree[0]);
	    cl::Buffer buffer_n_nodes(context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, sizeof(cl_int), &n_nodes);
	    cl::Buffer buffer_i(context, CL_MEM_READ_WRITE, sizeof(cl_int));
	    cl::Buffer buffer_j(context, CL_MEM_READ_WRITE, sizeof(cl_int));
	    cl::Buffer buffer_sample(context, CL_MEM_READ_WRITE, sizeof(cl_int));
	    cl::Buffer buffer_materials(context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, sizeof(cl_Material) * materials.size(), &materials[0]);
	    cl::Buffer buffer_profiling(context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, sizeof(ProfilingInfo), &profilingInfo);


	    // Create CommandQueue
	    cl::CommandQueue queue(context,default_device, CL_QUEUE_PROFILING_ENABLE);
	 
	    // Write data to device
	    //queue.enqueueWriteBuffer(buffer_camera,CL_TRUE,0,sizeof(Camera), &camera);

	    /* Create Kernel */
	    cl::Kernel kernel_raytrace=cl::Kernel(program,"raytrace");

	    /* Set argument */
	    kernel_raytrace.setArg(0, buffer_image);
	    kernel_raytrace.setArg(1, buffer_scene);
	    kernel_raytrace.setArg(2, buffer_triangles);
	    kernel_raytrace.setArg(3, buffer_n_triangles);
	    kernel_raytrace.setArg(4, buffer_nodes);
	    kernel_raytrace.setArg(5, buffer_n_nodes);
	    kernel_raytrace.setArg(6, buffer_i);
	    kernel_raytrace.setArg(7, buffer_j);
	    kernel_raytrace.setArg(8, buffer_sample);
	    kernel_raytrace.setArg(9, buffer_materials);
	    kernel_raytrace.setArg(10, buffer_profiling);

	    /* Enqueue range */
	    const int batches_x = 10;
	    const int batches_y = 10;
	    const int batch_height = scene.height/batches_y; //54
	    const int batch_width = scene.width/batches_x;
	    const int num_samples = 100;
	    const int batch_samples = 10;
	    const int sample_size = num_samples/batch_samples;

	    std::cout << "Started rendering" << std::endl;
	    std::clock_t start;
	    double duration;

	    start = std::clock(); // get current time




		int * batch_i_ptr = (int*)queue.enqueueMapBuffer(buffer_i, CL_TRUE, CL_MAP_WRITE, 0, sizeof(int));
		int * batch_j_ptr = (int*)queue.enqueueMapBuffer(buffer_j, CL_TRUE, CL_MAP_WRITE, 0, sizeof(int));
		int * batch_sample_ptr = (int*)queue.enqueueMapBuffer(buffer_sample, CL_TRUE, CL_MAP_WRITE, 0, sizeof(int));

		cl::Event event;
		float elapsedTime = 0;

		for(int k=0; k<batch_samples; k++){
			*batch_sample_ptr = batch_samples * k;

		    for(int i=0; i<batches_x; i++){
			    *batch_i_ptr = batch_width * i;

			    for(int j=0; j<batches_y; j++){
			        *batch_j_ptr = batch_height * j;

			        //queue.enqueueWriteBuffer(buffer_scene, CL_FALSE, 0, sizeof(cl_Scene), &scene);
			        queue.enqueueNDRangeKernel(kernel_raytrace,cl::NullRange, cl::NDRange(batch_width, batch_height, sample_size), cl::NullRange, NULL, &event);
			        event.wait();
			        queue.finish();
			        cl_ulong time_start;
					cl_ulong time_end;

					time_start = event.getProfilingInfo<CL_PROFILING_COMMAND_START>();
					time_end = event.getProfilingInfo<CL_PROFILING_COMMAND_END>();

					elapsedTime += time_end-time_start;
			        //std::cout << "Sample batch number " << k << " of " << batch_samples << "    " <<(i)*batches_y+j+1 <<  "/" << batches_x*batches_y << std::endl;
			    }
			    
			    //std::cout << i+1 <<  "/" << batches_x << std::endl;
			}
			std::cout << k+1 <<  "/" << batch_samples << std::endl;
		}
	    

	    /* Read buffer */
	    queue.enqueueReadBuffer(buffer_image,CL_TRUE,0,sizeof(cl_float3)*image.size(),&image[0]);
	    queue.enqueueReadBuffer(buffer_profiling,CL_TRUE,0,sizeof(ProfilingInfo),&profilingInfo);

	    writeToFile("outputs/output.ppm", &image[0], scene.width, scene.height, num_samples);

	    printf("OpenCl Execution time is: %0.3f seconds \n", elapsedTime / 1000000000.0);
	    printInfo();



	}


	void printInfo(){

		std::cout << "Performed " << profilingInfo.bbox_intersections << " box intersections" << std::endl;
		std::cout << "Performed " << profilingInfo.triangle_intersections << " triangle intersections" << std::endl;
		std::cout << "Traced " << profilingInfo.rays <<  " rays" << std::endl;
		std::cout << "max depth " << profilingInfo.max_depth << std::endl;



	}


	void addMaterial(cl_Material material){

		materials.push_back(material);


	}



	void addPlane(cl_float3 v0, cl_float3 v1, cl_float3 v2, cl_float3 v3, cl_float3 normal, int material_id){

		cl_ObjectInfo object;
		object.first_index = triangles.size();

		cl_Triangle t1;
	    cl_Triangle t2;

	    createTriangle(v0, v1, v2, normal, normal, normal, material_id, t1);
	    createTriangle(v0, v2, v3, normal, normal, normal, material_id, t2);

	    triangles.push_back(t1);
	    triangles.push_back(t2);

	    object.use_bbox = 0;

	    object.last_index = triangles.size();

	    objects.push_back(object);


	}


	void loadObj(const std::string &path,
                 const glm::mat4 &model,
                 int material_id){

		glm::mat3 invModel = glm::mat3(glm::transpose(glm::inverse(model)));
	    std::vector<glm::vec3> normals;

	    // Initialize Loader
	    objl::Loader Loader;

	    // Load .obj File
	    bool loadout = Loader.LoadFile(&path[0]);

	    std::cout << "Loaded " << Loader.LoadedMeshes.size() << " meshes" << std::endl;

	    // Check to see if it loaded

	    // If so continue
	    if (loadout){

	        // Create/Open e1Out.txt
	        //std::ofstream file("e1Out.txt");

	        // Go through each loaded mesh and out its contents
	        for (int i = 0; i < Loader.LoadedMeshes.size(); i++)
	        {

	        	/* Create objects info */

	        	cl_ObjectInfo object;
	        	object.use_bbox = 1;

	            // Copy one of the loaded meshes to be our current mesh
	            objl::Mesh curMesh = Loader.LoadedMeshes[i];

	            
	            // // Go through each vertex and print its number,
	            // //  position, normal, and texture coordinate
	            for (int j = 0; j < curMesh.Vertices.size(); j++)
	            {

	                normals.push_back(glm::vec3(0.0));
	            }

	            // // Print Indices
	            // file << "Indices:\n";

	            // Go through every 3rd index and print the
	            //  triangle that these indices represent
	            for (int j = 0; j < curMesh.Indices.size(); j += 3)
	            {   

	                glm::vec3 vnface0 = invModel * glm::vec3(curMesh.Vertices[curMesh.Indices[j]].Normal.X,
	                                                              curMesh.Vertices[curMesh.Indices[j]].Normal.Y,
	                                                              curMesh.Vertices[curMesh.Indices[j]].Normal.Z );

	                glm::vec3 vnface1 = invModel * glm::vec3(curMesh.Vertices[curMesh.Indices[j+1]].Normal.X,
	                                                              curMesh.Vertices[curMesh.Indices[j+1]].Normal.Y,
	                                                              curMesh.Vertices[curMesh.Indices[j+1]].Normal.Z );

	                glm::vec3 vnface2 = invModel * glm::vec3(curMesh.Vertices[curMesh.Indices[j+2]].Normal.X,
	                                                              curMesh.Vertices[curMesh.Indices[j+2]].Normal.Y,
	                                                              curMesh.Vertices[curMesh.Indices[j+2]].Normal.Z );


	                normals[curMesh.Indices[j]] += vnface0;
	                normals[curMesh.Indices[j+1]] += vnface1;
	                normals[curMesh.Indices[j+2]] += vnface2;


	            }

	            

	            for (int j = 0; j < curMesh.Indices.size(); j += 3)
	            {   

	                cl_float3 v0 = vec3ToCL(wDivide(model * glm::vec4(curMesh.Vertices[curMesh.Indices[j]].Position.X,
	                                                                    curMesh.Vertices[curMesh.Indices[j]].Position.Y,
	                                                                    curMesh.Vertices[curMesh.Indices[j]].Position.Z, 1.0)));

	                cl_float3 v1 = vec3ToCL(wDivide(model * glm::vec4(curMesh.Vertices[curMesh.Indices[j+1]].Position.X,
	                                                                    curMesh.Vertices[curMesh.Indices[j+1]].Position.Y,
	                                                                    curMesh.Vertices[curMesh.Indices[j+1]].Position.Z, 1.0)));

	                cl_float3 v2 = vec3ToCL(wDivide(model * glm::vec4(curMesh.Vertices[curMesh.Indices[j+2]].Position.X,
	                                                                    curMesh.Vertices[curMesh.Indices[j+2]].Position.Y,
	                                                                    curMesh.Vertices[curMesh.Indices[j+2]].Position.Z, 1.0)));

	                cl_float3 vn0 = vec3ToCL(glm::normalize(normals[curMesh.Indices[j]]));
	                cl_float3 vn1 = vec3ToCL(glm::normalize(normals[curMesh.Indices[j+1]]));
	                cl_float3 vn2 = vec3ToCL(glm::normalize(normals[curMesh.Indices[j+2]]));

	                cl_Triangle t;
	                createTriangle(v0, v1, v2, vn0, vn1, vn2, material_id, t);

	                // if((j % 200 == 0) && (j != 0)){
	                // 	object.last_index = triangles.size();	
	                // 	objects.push_back(object);                	
	                // }


	                if(j == 0){
	                	object.first_index = triangles.size();
	                	bbInit(object, v0);
	                }

	                else
	                	bbExtend(object, v0);
	                	
	                bbExtend(object, v1);
	                bbExtend(object, v2);


	                triangles.push_back(t);

	            
	            }

	            object.last_index = triangles.size();
	            objects.push_back(object);

	        }


	    }

	}


	void bbInit(cl_ObjectInfo &object, cl_float3 v){

		object.bbstart = v;
		object.bbend = v;

	}


	void bbExtend(cl_ObjectInfo &object, cl_float3 v){

			object.bbstart.x = std::min(object.bbstart.x, v.x);
			object.bbstart.y = std::min(object.bbstart.y, v.y);
			object.bbstart.z = std::min(object.bbstart.z, v.z);

			object.bbend.x = std::max(object.bbend.x, v.x);
			object.bbend.y = std::max(object.bbend.y, v.y);
			object.bbend.z = std::max(object.bbend.z, v.z);

	}



	void splitObjects(int n_splits){

		std::vector<cl_ObjectInfo> temp = objects;

		for(int i=0; i<n_splits; i++){

			std::vector<cl_ObjectInfo> to_add;

			for(int i=0; i<temp.size(); i++)
				splitObject(temp[i], triangles, to_add);

			//merge_vectors(objects, to_add);
			temp = to_add;

			// objects.resize(objects.size() + list_right.size());
	  //   	std::move(list_left.begin(), list_left.end(), &triangles[start]);
	  //   	std::move(list_right.begin(), list_right.end(), &triangles[start] + list_left.size());


		}

		objects = temp;

	}


	cl_float cl_vec_at(const cl_float3 &vec, int i){

		cl_float e;

		if(i == 0)
			e = vec.x;
		if(i == 1)
			e = vec.y;
		if(i == 2)
			e = vec.z;

		return e;

	}


	cl_float calculateMeanDim(std::vector<cl_Triangle> &triangles, int start, int end, int axis){

		cl_float sum = 0.0;

		for(int i=start; i<end; i++){

			sum += cl_vec_at(centroid(triangles[i]), axis);

		}

		return sum / (end - start);

	}


	void splitObject(const cl_ObjectInfo &object, std::vector<cl_Triangle> &triangles, std::vector<cl_ObjectInfo> &to_add){

		if((object.use_bbox == 0) || (object.last_index - object.first_index) < 20){
			to_add.push_back(object);
			return;
		}

		int start = object.first_index;
		int end = object.last_index;


		std::vector<cl_Triangle> list_left;
		std::vector<cl_Triangle> list_right;

		int random_axis = rand() % 3;
		cl_float middle = cl_vec_at(object.bbstart, random_axis) + (cl_vec_at(object.bbend, random_axis) - cl_vec_at(object.bbstart, random_axis))/2;
		//cl_float middle = calculateMeanDim(triangles, start, end, random_axis);

		for(int i=start; i<end; i++){

			if(cl_vec_at(centroid(triangles[i]), random_axis) < middle)
				list_left.push_back(triangles[i]);
			else
				list_right.push_back(triangles[i]);

		}

		std::cout << "left size is "<< list_left.size() << std::endl;
		std::cout << "right size is " << list_right.size() << std::endl;



		cl_ObjectInfo left, right;

		left.first_index = start;
		left.last_index = start + list_left.size();

		right.first_index = start + list_left.size();
		right.last_index = end;

		//triangles.resize(list_left.size() + list_right.size());
	    std::move(list_left.begin(), list_left.end(), &triangles[start]);
	    std::move(list_right.begin(), list_right.end(), &triangles[start] + list_left.size());

	    calculateBoundingBox(left, triangles);
	    calculateBoundingBox(right, triangles);

	    left.use_bbox = 1;
	    right.use_bbox = 1;

	    if(list_left.size() > 0)
			to_add.push_back(left);

		if(list_right.size() > 0)
			to_add.push_back(right);

		

	    

	}



	void calculateTriangleBoundingBox(const cl_Triangle &triangle, cl_float3 &bbstart, cl_float3 &bbend){

		bbstart = (cl_float3) { std::min(triangle.v0.x, std::min(triangle.v1.x, triangle.v2.x)),
								std::min(triangle.v0.y, std::min(triangle.v1.y, triangle.v2.y)),
								std::min(triangle.v0.z, std::min(triangle.v1.z, triangle.v2.z)) };

		bbend = (cl_float3) { std::max(triangle.v0.x, std::max(triangle.v1.x, triangle.v2.x)),
								std::max(triangle.v0.y, std::max(triangle.v1.y, triangle.v2.y)),
								std::max(triangle.v0.z, std::max(triangle.v1.z, triangle.v2.z)) };

	}


	void extendBoundingBox(const cl_Triangle &triangle, cl_float3 &bbstart, cl_float3 &bbend){

		cl_float3 other_start, other_end;

		calculateTriangleBoundingBox(triangle, other_start, other_end);

		bbstart = (cl_float3) { std::min(bbstart.x, other_start.x),
								std::min(bbstart.y, other_start.y),
								std::min(bbstart.z, other_start.z) };

		bbend = (cl_float3) {	std::max(bbend.x, other_end.x),
								std::max(bbend.y, other_end.y),
								std::max(bbend.z, other_end.z) };


	}

	void calculateBoundingBox(cl_ObjectInfo &object, const std::vector<cl_Triangle> &triangles){


		int start = object.first_index;
		int end = object.last_index;

		cl_float3 bbstart;
		cl_float3 bbend;

		calculateTriangleBoundingBox(triangles[start], bbstart, bbend);

		for(int i=start+1; i<end; i++)
			extendBoundingBox(triangles[i], bbstart, bbend);

		object.bbstart = bbstart;
		object.bbend = bbend;

	}


};