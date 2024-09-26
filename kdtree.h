#ifndef KDTREE_H
#define KDTREE_H

#include "geometry.h"
#include "bbox.h"
#include <vector>
#include <memory>

class KDNode {
    public:
        BoundingBox bbox;
        std::vector<Triangle*> triangles;
        std::shared_ptr<KDNode> left;
        std::shared_ptr<KDNode> right;

        KDNode();
        KDNode(std::vector<Triangle*>& triangles);
};

class KDTree {
    public:
        std::shared_ptr<KDNode> root;

        KDTree(std::vector<Triangle>& triangles);
        bool intersect(const Ray& ray, float& t, Triangle*& hit_triangle) const;

    private:
        std::shared_ptr<KDNode> build(std::vector<Triangle*> triangles, int depth);
        bool intersectNode(const std::shared_ptr<KDNode> node, const Ray& ray, float& t, Triangle*& hit_triangle) const;
};

#endif // KDTREE_H