#include "primitive_tree.h"
#include <limits>
#include <algorithm>
#include <cmath>
#include <stdexcept>

const int MAX_DEPTH = 20;
const size_t MIN_PRIMITIVES_PER_LEAF = 4;

namespace {
    float calculateSAH(const BoundingBox& left_bbox, const BoundingBox& right_bbox, int numLeft, int numRight, const BoundingBox& parent_bbox) {
        /*
        Surface Area Heuristic (SAH) cost.
        */
        if (numLeft == 0 || numRight == 0) {
            return static_cast<float>(numLeft + numRight); 
        }

        float leftArea = left_bbox.getSurfaceArea();
        float rightArea = right_bbox.getSurfaceArea();
        float totalParentArea = parent_bbox.getSurfaceArea();

        if (totalParentArea <= 0.0f) {
            return static_cast<float>(numLeft + numRight);
        }

        float pLeft = leftArea / totalParentArea;
        float pRight = rightArea / totalParentArea;
        
        float cost_traversal = 1.0f; 
        return cost_traversal + pLeft * numLeft + pRight * numRight;
    }
}

BoundingBox merge(const std::vector<Primitive*>& primitives, size_t start_idx, size_t end_idx) {
    if (start_idx >= end_idx || (start_idx == 0 && end_idx == 0 && primitives.empty())) {
        return BoundingBox();
    }

    if (start_idx >= primitives.size() || end_idx > primitives.size()) {
        throw std::out_of_range("Index Out of Bounds!");
    }

    Vec3 _min(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    Vec3 _max(std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest());

    for (size_t i = start_idx; i < end_idx; ++i) {
        const auto& primitive = primitives[i];
        BoundingBox bbox = primitive->getBoundingBox();
        for (int j = 0; j < 3; ++j) {
            _min[j] = std::min(_min[j], bbox.min[j]);
            _max[j] = std::max(_max[j], bbox.max[j]);
        }
    }
    return BoundingBox(_min, _max);
}

//-----------------------------------------------------//

PrimitiveNode::PrimitiveNode() 
    : left(nullptr), right(nullptr), primitive_start_index(0), primitive_count(0) {
}

PrimitiveTree::PrimitiveTree(std::vector<Primitive*>& primitives_list)
    : all_primitives(primitives_list) {
    if (all_primitives.empty()) {
        root = nullptr;
    } else {
        root = build(0, all_primitives.size(), 0);
    }
}

std::shared_ptr<PrimitiveNode> PrimitiveTree::build(size_t start, size_t end, int depth) {
    auto node = std::make_shared<PrimitiveNode>();
    size_t num_primitives_in_node = end - start;

    node->bbox = merge(all_primitives, start, end);

    if (num_primitives_in_node <= MIN_PRIMITIVES_PER_LEAF || depth >= MAX_DEPTH) {
        node->primitive_start_index = start;
        node->primitive_count = num_primitives_in_node;
        return node;
    }

    float best_sah_cost = std::numeric_limits<float>::max();
    int best_split_axis = -1;
    size_t best_split_index = start;

    BoundingBox centroid_bounds = BoundingBox();
    for(size_t i = start; i < end; ++i) {
        centroid_bounds.expand(all_primitives[i]->getBoundingBox().center());
    }


    for (int axis = 0; axis < 3; ++axis) {
        if (centroid_bounds.min[axis] == centroid_bounds.max[axis] && num_primitives_in_node > 1) {
            continue;
        }
        
        std::sort(all_primitives.begin() + start, all_primitives.begin() + end,
            [axis](const Primitive* a, const Primitive* b) {
                return a->getBoundingBox().center()[axis] < b->getBoundingBox().center()[axis];
            }
        );

        std::vector<BoundingBox> right_accumulated_bboxes(num_primitives_in_node);
        BoundingBox current_right_bbox_accum = BoundingBox();
        
        for (int i = num_primitives_in_node - 1; i >= 0; --i) {
            current_right_bbox_accum.expand(all_primitives[start + i]->getBoundingBox());
            right_accumulated_bboxes[i] = current_right_bbox_accum;
        }

        BoundingBox left_bbox_accum = BoundingBox();
        for (size_t i = 0; i < num_primitives_in_node - 1; ++i) {
            left_bbox_accum.expand(all_primitives[start + i]->getBoundingBox());
            
            BoundingBox right_bbox_to_consider = right_accumulated_bboxes[i + 1];
            
            int num_left = i + 1;
            int num_right = num_primitives_in_node - num_left;
            
            float sah_cost = calculateSAH(left_bbox_accum, right_bbox_to_consider, num_left, num_right, node->bbox);
            
            if (sah_cost < best_sah_cost) {
                best_sah_cost = sah_cost;
                best_split_axis = axis;
                best_split_index = start + i + 1;
            }
        }
    }

    float cost_if_leaf = static_cast<float>(num_primitives_in_node); 
    
    if (best_split_axis == -1 || best_sah_cost >= cost_if_leaf - 1e-4f) {
        node->primitive_start_index = start;
        node->primitive_count = num_primitives_in_node;
        return node;
    }

    std::nth_element(
        all_primitives.begin() + start, 
        all_primitives.begin() + best_split_index,
        all_primitives.begin() + end,
        [best_split_axis](const Primitive* a, const Primitive* b) {
            return a->getBoundingBox().center()[best_split_axis] < b->getBoundingBox().center()[best_split_axis];
        }
    );

    node->left = build(start, best_split_index, depth + 1);
    node->right = build(best_split_index, end, depth + 1);

    return node;
}

// Intersection traversal
bool PrimitiveTree::intersect(const Ray& ray, float& t, Primitive*& hitPrimitive) const {
    if (!root) return false;
    t = std::numeric_limits<float>::max();
    hitPrimitive = nullptr;
    return intersectNode(root, ray, t, hitPrimitive);
}

bool PrimitiveTree::intersectNode(const std::shared_ptr<PrimitiveNode> node, const Ray& ray, float& t, Primitive*& hitPrimitive) const {
    if (!node) return false;

    float t_box_min;
    if (!node->bbox.intersect(ray, t_box_min) || t_box_min >= t) {
        return false;
    }

    if (node->isLeaf()) {
        bool foundHitInLeaf = false;
        for (size_t i = 0; i < node->primitive_count; ++i) {
            Primitive* current_primitive = all_primitives[node->primitive_start_index + i];
            float current_t_primitive;
            if (current_primitive->intersect(ray, current_t_primitive) && current_t_primitive < t) {
                t = current_t_primitive;
                hitPrimitive = current_primitive;
                foundHitInLeaf = true;
            }
        }
        return foundHitInLeaf;
    }

    std::shared_ptr<PrimitiveNode> first_child_to_visit = node->left;
    std::shared_ptr<PrimitiveNode> second_child_to_visit = node->right;

    float tmin_left_child, tmin_right_child;
    bool intersects_left_child = first_child_to_visit->bbox.intersect(ray, tmin_left_child);
    bool intersects_right_child = second_child_to_visit->bbox.intersect(ray, tmin_right_child);

    if (intersects_left_child && intersects_right_child) {
        if (tmin_left_child > tmin_right_child) {
            std::swap(first_child_to_visit, second_child_to_visit);
        }
    } else if (!intersects_left_child && intersects_right_child) {
        first_child_to_visit = node->right;
        second_child_to_visit = nullptr;
    } else if (intersects_left_child && !intersects_right_child) {
        second_child_to_visit = nullptr;
    } else {
        return false;
    }


    bool hit1 = false;
    if (first_child_to_visit) {
        hit1 = intersectNode(first_child_to_visit, ray, t, hitPrimitive);
    }

    bool hit2 = false;
    if (second_child_to_visit) { 
        float t_second_child_box_min;
        if (second_child_to_visit->bbox.intersect(ray, t_second_child_box_min) && t_second_child_box_min < t) {
            hit2 = intersectNode(second_child_to_visit, ray, t, hitPrimitive);
        }
    }

    return hit1 || hit2;
}