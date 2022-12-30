#define CL_HPP_MINIMUM_OPENCL_VERSION 210
#define CL_HPP_TARGET_OPENCL_VERSION 210

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <math.h>
#include <ctime>


#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>



#include <CL/cl2.hpp>
#include "types.h"
#include "common/common.h"
#include "utils.h"
#include "materials.h"
#include "geometry.h"
#include "bvh.h"
#include "renderer.h"








 
int main(){

    cl_Renderer renderer;


    /* Materials */

    cl_Material floor, tree, deer, light, sky;
    createLambertian(floor, (cl_float3){0.5, 0.5, 0.5});
    createLambertian(sky, (cl_float3){0.01, 0.01, 0.01});
    createMetallic(floor, (cl_float3){0.1, 0.1, 0.1}, 0.0);
    //createLambertian(floor, (cl_float3){0.2, 0.2, 0.2});
    createDieletric(tree, (cl_float3){0.52, 0.21, 0.26}, 1.5);
    createLambertian(tree, (cl_float3){0.1, 0.1, 0.1});
    //createLambertian(tree, (cl_float3){0.5, 0.5, 0.5});
    //createDieletric(tree, (cl_float3){0.8, 0.8, 0.8}, 1.5);
    createDieletric(deer, (cl_float3){0.1, 0.1, 0.1}, 1.5);
    createDiffuseLight(light, (cl_float3){0.8, 28, 35});
    //createMetallic(deer, (cl_float3){0.1, 0.1, 0.1}, 0.0);
    //createMetallic(deer, (cl_float3){0.8, 0.8, 0.8}, 1.0);

    renderer.addMaterial(floor); // 0
    renderer.addMaterial(tree); // 1
    renderer.addMaterial(deer); // 2
    renderer.addMaterial(light); // 3
    renderer.addMaterial(sky); // 4
    renderer.addMaterial(light); // 5




    /* Floor */

    cl_float half = 160.0/2;

    cl_float ypos = -5.0;

    cl_float3 v0 = {-half, ypos, half};
    cl_float3 v1 = {half, ypos, half};
    cl_float3 v2 = {half, ypos, -half};
    cl_float3 v3 = {-half, ypos, -half};

    cl_float3 normal = {0.0, 1.0, 0.0};

    renderer.addPlane(v0, v1, v2, v3, normal, 0);




    /* Back plane */

    cl_float zpos = -40;

    v0 = {-half, half, zpos};
    v1 = {-half, -half, zpos};
    v2 = {half, -half, zpos};
    v3 = {half, half, zpos};

    normal = {0.0, 0.0, 1.0};

    renderer.addPlane(v0, v1, v2, v3, normal, 4);



    /* Add cow */
    glm::mat4 model = glm::mat4(1.0f);
    
    model = glm::translate(model, glm::vec3(0.0, -2.0, -18.0));
    
    model = glm::rotate(model, glm::radians(-45.0f), glm::vec3(0, 1, 0));

    // addMesh(triangles, "cow.geo", model, object);



    /* Trees */

    int n = 5;

    for(int i=0; i<n; i++){

        for(int j=0; j<1; j++){

            model = glm::mat4(1.0f);

            

            model = glm::translate(model, glm::vec3(25.0 * cos(- i * M_PI / (n - 1)), 2.5, -10.0 * sin(i * M_PI / (n - 1))) - glm::vec3(0.0, 0.0, 15.0));
            
            model = glm::translate(model, glm::vec3(0, 0.0, -10.0 * j));

            model = glm::scale(model, glm::vec3(4, 4, 4));

            //if(i != n/2)
                //renderer.loadObj("models/lowpolytree.obj", model, 1);
        }
    }


    /* Object */


    model = glm::mat4(1.0f);
    
    model = glm::translate(model, glm::vec3(-1.5, -5.0, -10.0));

    model = glm::scale(model, glm::vec3(2, 2, 2));

    model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0, 1, 0));

    // renderer.loadObj("present.obj", model, deer);

    model = glm::mat4(1.0f);
    
    model = glm::translate(model, glm::vec3(-1.5, -2.0, -13.0));

    model = glm::scale(model, glm::vec3(2, 2, 2));

    model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0, 1, 0));

    //renderer.loadObj("models/UmbreonLowPoly.obj", model, 2);


    model = glm::mat4(1.0f);
    
    model = glm::translate(model, glm::vec3(0.0, 8.0, -30.0));

    model = glm::scale(model, glm::vec3(2.0, 2.0, 2.0));

    //model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0, 1, 0));

    //renderer.loadObj("models/sphere.obj", model, 5);

    //renderer.splitObjects(3);

    std::cout << "Loaded " << renderer.objects.size() << " objects" << std::endl;

    for(int i=0; i<renderer.objects.size(); i++){

        std::cout << "Object number " << i << " starts at index " << renderer.objects[i].first_index << " and ends at index " << renderer.objects[i].last_index << std::endl;
    }

    std::cout << "Loaded " << renderer.triangles.size() << " triangles" << std::endl;


    renderer.render();
 
    return 0;
}