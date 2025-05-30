#ifndef PRIMITIVE_TREE_H
#define PRIMITIVE_TREE_H

#include "geometry.h"
#include "bbox.h"
#include <vector>
#include <memory>

class PrimitiveNode {
public:
    BoundingBox bbox;
    std::shared_ptr<PrimitiveNode> left;
    std::shared_ptr<PrimitiveNode> right;

    size_t primitive_start_index; 
    size_t primitive_count;

    PrimitiveNode();

    bool isLeaf() const {
        return left == nullptr && right == nullptr;
    }
};

class PrimitiveTree {
public:
    std::shared_ptr<PrimitiveNode> root;

    PrimitiveTree(std::vector<Primitive*>& primitives_list);

    bool intersect(const Ray& ray, float& t, Primitive*& hitPrimitive) const;

private:
    std::vector<Primitive*>& all_primitives; 

    std::shared_ptr<PrimitiveNode> build(size_t start, size_t end, int depth);
    
    bool intersectNode(const std::shared_ptr<PrimitiveNode> node, const Ray& ray, float& t, Primitive*& hitPrimitive) const;
};

#endif // PRIMITIVE_TREE_H