class Scene{

private:
	std::vector<Object> objects;
	std::vector<Material> materials;
	BVHNode tree;

	void updateBHVTree();

public:
	void addPlane();

	void addObject(const Mesh &mesh, const uint materialId){

		Object obj(mesh, materialId);
		objects.push_back(obj);

		updateBHVTree();

	}


}







int main{


	Scene scene;

	uint floorMaterial = scene.addMaterial(kLambertian, albedo);
	uint treeTopMaterial = scene.addMaterial(kLambertian, albedo);
	uint treeBottomMaterial = scene.addMaterial(kDielectric, ior);


	scene.addPlane(position, floorMaterial);

	std::vector<Mesh> tree = Loader::loadObjFile("tree.obj", model);
 
	scene.addObject(tree[0], treeBottomMaterial);
	scene.addObject(tree[1], treeTopMaterial);

	Renderer renderer;
	renderer.render(scene);

	













	

}

