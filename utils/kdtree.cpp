#include "kdtree.h"
#include "bbox.h"
#include <limits>
#include <algorithm> 
#include <cmath>

/*
 * Support functions
 */
namespace {
    float calculateSAH(const BoundingBox& left, const BoundingBox& right, int numLeft, int numRight) {
        float leftArea = left.getSurfaceArea();
        float rightArea = right.getSurfaceArea();
        float totalArea = left.expand(right).getSurfaceArea();

        float pLeft = leftArea / totalArea;   
        float pRight = rightArea / totalArea;

        return 1.0f + pLeft * numLeft + pRight * numRight;
    }
}

BoundingBox merge(std::vector<Triangle*>& triangles){
    if (triangles.empty()){
        throw std::invalid_argument("Triangle list cannot be empty");
    }

    Vec3 _min(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    Vec3 _max(std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest());

    for (const auto& triangle: triangles){
        BoundingBox bbox = triangle->getBoundingBox();
        for (int i = 0; i< 3; i++){
            _min[i] = std::min(_min[i], bbox.min[i]);
            _max[i] = std::max(_max[i], bbox.max[i]);
        }
    }
    return BoundingBox(_min, _max);
}

bool hasIntersectPrimitives(const std::shared_ptr<KDNode> node, const Ray& ray, float& t, Triangle*& hitTriangle) {
    bool hit = false;

    for (const auto& triangle : node->triangles) {
        float curr_t;
        if (triangle->intersect(ray, curr_t) && curr_t < t) {
            t = curr_t;
            hitTriangle = triangle;
            hit = true;
        }
    }
    return hit;
}
//----------------------------------------------------------------//

KDNode::KDNode() : left(nullptr), right(nullptr) {}

KDNode::KDNode(std::vector<Triangle*>& triangles) : triangles(triangles), left(nullptr), right(nullptr) {
    if (!this->triangles.empty()) {
        bbox = merge(this->triangles);
    }
}

KDTree::KDTree(std::vector<Triangle*>& triangles) {
    root = KDTree::build(triangles, 0);
}



std::shared_ptr<KDNode> KDTree::build(std::vector<Triangle*> triangles, int depth) {
    if (triangles.empty()) {
        return nullptr;
    }

    int maxDepth = 8 + 1.3 * log2(triangles.size());
    
    if (triangles.size() <= 2 || depth >= maxDepth) {
        return std::make_shared<KDNode>(triangles);
    }

    int bestAxis = -1;
    float bestCost = std::numeric_limits<float>::max();
    std::vector<Triangle*> leftTriangles, rightTriangles;

    for (int axis = 0; axis < 3; axis++) {
        std::sort(triangles.begin(), triangles.end(), [axis](Triangle* a, Triangle* b) {
            return a->getBoundingBox().center()[axis] < b->getBoundingBox().center()[axis];
        });

        for (size_t i = 1; i < triangles.size(); i++) {
            std::vector<Triangle*> left(triangles.begin(), triangles.begin() + i);
            std::vector<Triangle*> right(triangles.begin() + i, triangles.end());

            auto leftBBox = merge(left);
            auto rightBBox = merge(right);

            float sahCost = calculateSAH(leftBBox, rightBBox, left.size(), right.size());

            if (sahCost < bestCost) {
                bestCost = sahCost;
                bestAxis = axis;
                leftTriangles = left;
                rightTriangles = right;
            }
        }
    }

    if (bestAxis == -1) {
        return std::make_shared<KDNode>(triangles);
    }

    auto leftNode = build(leftTriangles, depth + 1);
    auto rightNode = build(rightTriangles, depth + 1);

    auto node = std::make_shared<KDNode>();
    node->bbox = (leftNode->bbox).expand(rightNode->bbox);
    node->left = leftNode;
    node->right = rightNode;

    return node;
}

/*
 * Nearest neighbor traversal
 */
bool KDTree::intersectNode(const std::shared_ptr<KDNode> node, const Ray& ray, float& t, Triangle*& hitTriangle) const {
    if (!node) return false;

    float tmin, tmax;
    if (!node->bbox.intersect(ray, tmin, tmax)) return false;

    if (tmin - t > 1e-6) return false; // No need to check further intersections

    
    // If it is the leaf node, check interesections with its primitives
    if (!node->left && !node->right) {
        return hasIntersectPrimitives(node, ray, t, hitTriangle);
    }

    bool hit = false;

    /*
     * Determine the order of traversing the children.
     * Why do we choose this order? This is because, along the longest axis, 
     * we have split the space such that the left part lies on one side of the axis 
     * and the right part lies on the other. Therefore, if the projection of the 
     * ray direction onto the longest axis is negative, the ray will intersect 
     * the right node first, followed by the left node, and so on.
     */
    std::shared_ptr<KDNode> firstNode, secondNode;
    if (ray.direction[node->bbox.getLongestAxis()] < 0) {
        firstNode = node->right;
        secondNode = node->left;
    } else {
        firstNode = node->left;
        secondNode = node->right;
    }

    // Traverse the nearer child first
    if (intersectNode(firstNode, ray, t, hitTriangle)) {
        hit = true;
    }

    // Traverse the further child only if not hit the first child
    if (!hit && intersectNode(secondNode, ray, t, hitTriangle)) {
        hit = true;
    }

    return hit;
}

bool KDTree::intersect(const Ray& ray, float& t, Triangle*& hitTriangle) const {
    t = std::numeric_limits<float>::max();
    return intersectNode(root, ray, t, hitTriangle);
}