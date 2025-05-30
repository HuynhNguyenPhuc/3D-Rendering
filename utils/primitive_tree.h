#ifndef PRIMITIVE_TREE_H
#define PRIMITIVE_TREE_H

#include "geometry.h"
#include "bbox.h"
#include <vector>
#include <memory>

class PrimitiveNode {
public:
    BoundingBox bbox;
    std::vector<Primitive*> primitives;
    std::shared_ptr<PrimitiveNode> left;
    std::shared_ptr<PrimitiveNode> right; 

    PrimitiveNode();
    PrimitiveNode(const std::vector<Primitive*>& primitives);
};

class PrimitiveTree {
public:
    std::shared_ptr<PrimitiveNode> root;

    PrimitiveTree(std::vector<Primitive*>& primitives);

    bool intersect(const Ray& ray, float& t, Primitive*& hitPrimitive) const;

private:
    std::shared_ptr<PrimitiveNode> build(std::vector<Primitive*>& primitives, int depth);
    bool intersectNode(const std::shared_ptr<PrimitiveNode> node, const Ray& ray, float& t, Primitive*& hitPrimitive) const;
};

#endif // PRIMITIVE_TREE_H 