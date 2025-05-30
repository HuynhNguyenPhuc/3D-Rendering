#include "primitive_tree.h"
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


BoundingBox merge(const std::vector<Primitive*>& primitives) {
    if (primitives.empty()) {
        throw std::invalid_argument("Primitive list cannot be empty");
    }

    Vec3 _min(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    Vec3 _max(std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest());

    for (const auto& primitive : primitives) {
        BoundingBox bbox = primitive->getBoundingBox();
        for (int i = 0; i < 3; ++i) {
            _min[i] = std::min(_min[i], bbox.min[i]);
            _max[i] = std::max(_max[i], bbox.max[i]);
        }
    }

    return BoundingBox(_min, _max);
}

//-----------------------------------------------------//

PrimitiveNode::PrimitiveNode() : left(nullptr), right(nullptr) {}

PrimitiveNode::PrimitiveNode(const std::vector<Primitive*>& primitives) 
    : primitives(primitives), left(nullptr), right(nullptr) {
    if (!primitives.empty()) {
        bbox = merge(primitives);
    }
}

PrimitiveTree::PrimitiveTree(std::vector<Primitive*>& primitives) {
    root = PrimitiveTree::build(primitives, 0);
}

std::shared_ptr<PrimitiveNode> PrimitiveTree::build(std::vector<Primitive*>& primitives, int depth) {
    if (primitives.empty()) return nullptr;

    int maxDepth = 8 + 1.3 * log2(primitives.size());

    if (primitives.size() <= 2 || depth >= maxDepth) {
        return std::make_shared<PrimitiveNode>(primitives);
    }

    int bestAxis = -1;
    float bestCost = std::numeric_limits<float>::max();
    std::vector<Primitive*> leftPrimitives, rightPrimitives;

    for (int axis = 0; axis < 3; ++axis) {
        std::sort(primitives.begin(), primitives.end(), [axis](Primitive*& a, Primitive*& b) {
            return a->getBoundingBox().center()[axis] < b->getBoundingBox().center()[axis];
        });

        for (size_t i = 1; i < primitives.size(); ++i) {
            std::vector<Primitive*> left(primitives.begin(), primitives.begin() + i);
            std::vector<Primitive*> right(primitives.begin() + i, primitives.end());

            BoundingBox leftBBox = merge(left);
            BoundingBox rightBBox = merge(right);

            float sahCost = calculateSAH(leftBBox, rightBBox, left.size(), right.size());

            if (sahCost < bestCost) {
                bestCost = sahCost;
                bestAxis = axis;
                leftPrimitives = left;
                rightPrimitives = right;
            }
        }
    }

    if (bestAxis == -1) {
        return std::make_shared<PrimitiveNode>(primitives);
    }

    auto leftNode = build(leftPrimitives, depth + 1);
    auto rightNode = build(rightPrimitives, depth + 1);

    auto node = std::make_shared<PrimitiveNode>();
    node->bbox = leftNode->bbox.expand(rightNode->bbox);
    node->left = leftNode;
    node->right = rightNode;

    return node;
}

// Intersection traversal
bool PrimitiveTree::intersect(const Ray& ray, float& t, Primitive*& hitPrimitive) const {
    t = std::numeric_limits<float>::max();
    return intersectNode(root, ray, t, hitPrimitive);
}

bool PrimitiveTree::intersectNode(const std::shared_ptr<PrimitiveNode> node, const Ray& ray, float& t, Primitive*& hitPrimitive) const {
    if (!node) return false;

    float tmin, tmax;
    if (!node->bbox.intersect(ray, tmin, tmax)) return false;

    if (tmin - t > 1e-6) return false;

    // If it is the leaf node, check interesections with its primitives
    if (!node->left && !node->right) {
        bool hit = false;
        for (const auto& primitive: node->primitives) {
            float curr_t;
            if (primitive->intersect(ray, curr_t) && curr_t < t) {
                t = curr_t;
                hitPrimitive = primitive;
                hit = true;
            }
        }
        return hit;
    }

    bool hit = false;

    std::shared_ptr<PrimitiveNode> firstNode, secondNode;
    if (ray.direction[node->bbox.getLongestAxis()] < 0) {
        firstNode = node->right;
        secondNode = node->left;
    } else {
        firstNode = node->left;
        secondNode = node->right;
    }

    // Traverse the nearer child first
    if (intersectNode(firstNode, ray, t, hitPrimitive)) {
        hit = true;
    }

    // Traverse the further child only if not hit the first child
    if (!hit && intersectNode(secondNode, ray, t, hitPrimitive)) {
        hit = true;
    }

    return hit;
}