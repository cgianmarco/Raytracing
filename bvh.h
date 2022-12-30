#include "types.h"
#include "common/common.h"
#include <algorithm>


static const int MIN_SIZE = 2;


clBVHNode createBVHNode(const std::vector<cl_Triangle> &triangles){

	clBVHNode node;

	// node.bbstart = 
	// node.bbend = 0.0f;

	calculateBoundingBox(triangles, node);

	node.right = -1;
	node.left = -1;

	node.start = 0;
	node.end = 0;

	return node;

}

class BVHTreeGenerator{

public:

	std::vector<clBVHNode> nodes;
	std::vector<cl_Triangle> triangles;

	BVHTreeGenerator(const std::vector<cl_Triangle> &allTriangles){

		std::vector<std::pair<clBVHNode, std::vector<cl_Triangle>>> tree;

		clBVHNode root = createBVHNode(allTriangles); 

		tree.push_back(std::make_pair(root, allTriangles));

		int i = 0;

		while(i < tree.size()){

			std::vector<cl_Triangle> leftList, rightList;
			divideByRandomDim(tree[i], leftList, rightList);

			if((leftList.size() > MIN_SIZE) && (rightList.size() > MIN_SIZE)){
				tree[i].first.left = tree.size();
				clBVHNode left = createBVHNode(leftList);
				tree.push_back(std::make_pair(left, leftList));

				tree[i].first.right = tree.size();
				clBVHNode right = createBVHNode(rightList);
				tree.push_back(std::make_pair(right, rightList));
			}

			i+=1;

		}



		for(int i=0; i<tree.size(); i++){
			

			bool isLeaf = ((tree[i].first.left == -1) && (tree[i].first.right == -1));
			if(isLeaf){
				std::cout << tree[i].second.size() << std::endl;
				tree[i].first.start = triangles.size();
				for(int j=0; j<tree[i].second.size(); j++)
					triangles.push_back(tree[i].second[j]);
				tree[i].first.end = triangles.size();
			}

			nodes.push_back(tree[i].first);

			//printVec(cl_subtract3f(tree[i].first.bbend, tree[i].first.bbstart));
			printVec(tree[i].first.bbstart);
			printVec(tree[i].first.bbend);

		}


	}


	static bool compareTrianglesByDim(cl_Triangle a, cl_Triangle b){

		return centroid(a).x < centroid(b).x;


	}

	void divideTrianglesAtIndex(int index, const std::vector<cl_Triangle> &triangles, std::vector<cl_Triangle> &leftList, std::vector<cl_Triangle> &rightList){

		leftList.clear();
		rightList.clear();

		for(int i=0; i<triangles.size(); i++){

			if(i < index)
				leftList.push_back(triangles[i]);
			else
				rightList.push_back(triangles[i]);
		}
	}



	void divideByRandomDim(std::pair<clBVHNode, std::vector<cl_Triangle>> &current, 
						   std::vector<cl_Triangle> &leftList, std::vector<cl_Triangle> &rightList){

		



		uint randomDim = max_index(variance(current.second));
		randomDim = 0;
		float min_cost = 1000000000000;
		int min_index = -1;

		for(int j=0; j<3; j++){
			std::sort(current.second.begin(), current.second.end(), [j](cl_Triangle a, cl_Triangle b) { return cl_vec_at(centroid(a), j) < cl_vec_at(centroid(b), j); });


			//randomDim = 0;
			//float_ middle = cl_vec_at(current.first.bbstart, randomDim) + (cl_vec_at(current.first.bbend, randomDim) - cl_vec_at(current.first.bbstart, randomDim))/2;
			//float_ middle = cl_vec_at(getMean(current.second), randomDim);

			

			for(int i=1; i<current.second.size()-1; i++){

				divideTrianglesAtIndex(i, current.second, leftList, rightList);

				float leftArea = getSurfaceArea(leftList);
				float rightArea = getSurfaceArea(rightList);

				// float leftArea = 0.0f;
				// float rightArea = 0.0;

				float cost = leftArea * leftList.size() + rightArea * (current.second.size() - leftList.size());

				if(cost < min_cost){
					min_cost = cost;
					min_index = i;
					randomDim = j;
				}

			}
		}

		std::sort(current.second.begin(), current.second.end(), [randomDim](cl_Triangle a, cl_Triangle b) { return cl_vec_at(centroid(a), randomDim) < cl_vec_at(centroid(b), randomDim); });

		divideTrianglesAtIndex(min_index, current.second, leftList, rightList);

	}

	float3_ variance(std::vector<cl_Triangle> &triangles){

		float3_ variance = (float3_){0.0, 0.0, 0.0};

		float3_ mean = getMean(triangles);

		for(int i=0; i<triangles.size(); i++){

			variance = add(variance, pow(cl_subtract3f(centroid(triangles[i]), mean), 2.0f)); 

		}

		return divide(variance, triangles.size());

	}

	float3_ getMean(std::vector<cl_Triangle> &triangles){

		float3_ mean = (float3_) { 0.0f, 0.0f, 0.0f };

		for(int i=0; i<triangles.size(); i++)
			mean = add(mean, (centroid(triangles[i])));

		mean = divide(mean, triangles.size());

		return mean;

	}



	



};