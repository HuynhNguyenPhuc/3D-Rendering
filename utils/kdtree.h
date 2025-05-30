#ifndef KDTREE_H
#define KDTREE_H

#include "geometry.h"
#include "bbox.h"
#include <vector>
#include <memory>

class KDNode {
public:
    BoundingBox bbox;
    std::shared_ptr<KDNode> left;
    std::shared_ptr<KDNode> right;
    
    size_t triangle_start_index; 
    size_t triangle_count;

    KDNode();

    bool isLeaf() const {
        return left == nullptr && right == nullptr;
    }
};

class KDTree {
public:
    std::shared_ptr<KDNode> root;

    KDTree(std::vector<Triangle*>& triangles_list);

    bool intersect(const Ray& ray, float& t, Triangle*& hit_triangle) const;

private:
    std::vector<Triangle*>& all_triangles; 

    std::shared_ptr<KDNode> build(size_t start, size_t end, int depth);
    
    bool intersectNode(const std::shared_ptr<KDNode> node, const Ray& ray, float& t, Triangle*& hit_triangle) const;
};

#endif // KDTREE_H