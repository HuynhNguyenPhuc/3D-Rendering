#include "kdtree.h" 
#include <limits>
#include <algorithm>
#include <cmath>
#include <stdexcept>

const int MAX_DEPTH = 20;
const size_t MIN_TRIANGLES_PER_LEAF = 4;

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

        if (totalParentArea <= 1e-6f) { 
            return static_cast<float>(numLeft + numRight);
        }

        float pLeft = leftArea / totalParentArea;
        float pRight = rightArea / totalParentArea;
        
        float cost_traversal = 1.0f;
        return cost_traversal + pLeft * numLeft + pRight * numRight;
    }
}

BoundingBox merge(const std::vector<Triangle*>& triangles, size_t start_idx, size_t end_idx){
    if (start_idx >= end_idx || (start_idx == 0 && end_idx == 0 && triangles.empty())) {
        return BoundingBox();
    }

    if (start_idx >= triangles.size() || end_idx > triangles.size()) {
        throw std::out_of_range("Index Out of Bounds!");
    }

    Vec3 global_min(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    Vec3 global_max(std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest());
    
    for (size_t i = start_idx; i < end_idx; ++i){
        if (!triangles[i]) continue; 
        BoundingBox current_bbox = triangles[i]->getBoundingBox();
        for (int j = 0; j < 3; ++j){
            global_min[j] = std::min(global_min[j], current_bbox.min[j]);
            global_max[j] = std::max(global_max[j], current_bbox.max[j]);
        }
    }
    return BoundingBox(global_min, global_max);
}

//----------------------------------------------------------------//


KDNode::KDNode() 
    : left(nullptr), right(nullptr), triangle_start_index(0), triangle_count(0) {
}

KDTree::KDTree(std::vector<Triangle*>& triangles_list) 
    : all_triangles(triangles_list) {
    if (all_triangles.empty()) {
        root = nullptr;
    } else {
        root = build(0, all_triangles.size(), 0);
    }
}

std::shared_ptr<KDNode> KDTree::build(size_t start, size_t end, int depth) {
    auto node = std::make_shared<KDNode>();
    size_t num_triangles_in_node = end - start;

    node->bbox = merge(all_triangles, start, end);

    if (num_triangles_in_node <= MIN_TRIANGLES_PER_LEAF || depth >= MAX_DEPTH) {
        node->triangle_start_index = start;
        node->triangle_count = num_triangles_in_node;
        return node;
    }

    float best_sah_cost = std::numeric_limits<float>::max();
    int best_split_axis = -1;
    size_t best_split_index = start;

    BoundingBox centroid_bounds = BoundingBox();
    for(size_t i = start; i < end; ++i) {
        centroid_bounds.expand(all_triangles[i]->getBoundingBox().center());
    }

    for (int axis = 0; axis < 3; ++axis) {
        if (centroid_bounds.min[axis] == centroid_bounds.max[axis] && num_triangles_in_node > 1) {
            continue;
        }
        
        std::sort(all_triangles.begin() + start, all_triangles.begin() + end,
            [axis](const Triangle* a, const Triangle* b) {
                return a->getBoundingBox().center()[axis] < b->getBoundingBox().center()[axis];
            }
        );

        std::vector<BoundingBox> right_accumulated_bboxes(num_triangles_in_node);
        BoundingBox current_right_bbox_accum = BoundingBox();

        for (int i = num_triangles_in_node - 1; i >= 0; --i) {
            current_right_bbox_accum.expand(all_triangles[start + i]->getBoundingBox());
            right_accumulated_bboxes[i] = current_right_bbox_accum;
        }

        BoundingBox left_bbox_accum = BoundingBox();
        for (size_t i = 0; i < num_triangles_in_node - 1; ++i) {
            left_bbox_accum.expand(all_triangles[start + i]->getBoundingBox());
            
            BoundingBox right_bbox_to_consider = right_accumulated_bboxes[i + 1];
            
            int num_left = i + 1;
            int num_right = num_triangles_in_node - num_left;
            
            float sah_cost = calculateSAH(left_bbox_accum, right_bbox_to_consider, num_left, num_right, node->bbox);
            
            if (sah_cost < best_sah_cost) {
                best_sah_cost = sah_cost;
                best_split_axis = axis;
                best_split_index = start + i + 1;
            }
        }
    }

    float cost_if_leaf = static_cast<float>(num_triangles_in_node); 
    
    if (best_split_axis == -1 || best_sah_cost >= cost_if_leaf - 1e-4f) {
        node->triangle_start_index = start;
        node->triangle_count = num_triangles_in_node;
        return node;
    }

    std::nth_element(
        all_triangles.begin() + start, 
        all_triangles.begin() + best_split_index,
        all_triangles.begin() + end,
        [best_split_axis](const Triangle* a, const Triangle* b) {
            return a->getBoundingBox().center()[best_split_axis] < b->getBoundingBox().center()[best_split_axis];
        }
    );
    
    node->left = build(start, best_split_index, depth + 1);
    node->right = build(best_split_index, end, depth + 1);

    return node;
}

bool KDTree::intersect(const Ray& ray, float& t, Triangle*& hit_triangle) const {
    if (!root) return false; 
    t = std::numeric_limits<float>::max();
    hit_triangle = nullptr;
    return intersectNode(root, ray, t, hit_triangle);
}

bool KDTree::intersectNode(const std::shared_ptr<KDNode> node, const Ray& ray, float& t, Triangle*& hit_triangle) const {
    if (!node) return false;

    float t_box_min; 
    if (!node->bbox.intersect(ray, t_box_min) || t_box_min >= t) {
        return false;
    }

    if (node->isLeaf()) {
        bool foundHitInLeaf = false;
        if (node->triangle_count == 0) return false;

        for (size_t i = 0; i < node->triangle_count; ++i) {
            Triangle* current_triangle = all_triangles[node->triangle_start_index + i];
            float current_t_triangle;
            if (current_triangle->intersect(ray, current_t_triangle) && 
                current_t_triangle < t && current_t_triangle > 1e-5f) { 
                t = current_t_triangle;
                hit_triangle = current_triangle;
                foundHitInLeaf = true;
            }
        }
        return foundHitInLeaf;
    }

    std::shared_ptr<KDNode> first_child_to_visit = node->left;
    std::shared_ptr<KDNode> second_child_to_visit = node->right;

    float tmin_to_left_child = std::numeric_limits<float>::max();
    float tmin_to_right_child = std::numeric_limits<float>::max();
    
    bool ray_intersects_left_child = first_child_to_visit->bbox.intersect(ray, tmin_to_left_child);
    bool ray_intersects_right_child = second_child_to_visit->bbox.intersect(ray, tmin_to_right_child);

    if (ray_intersects_left_child && ray_intersects_right_child) {
        if (tmin_to_left_child > tmin_to_right_child) {
            std::swap(first_child_to_visit, second_child_to_visit);
        }
    } else if (!ray_intersects_left_child && ray_intersects_right_child) {
        first_child_to_visit = node->right;
        second_child_to_visit = nullptr;
    } else if (ray_intersects_left_child && !ray_intersects_right_child) {
        second_child_to_visit = nullptr;
    } else {
        return false; 
    }

    bool hit_in_first_child = false;
    if (first_child_to_visit) { 
        hit_in_first_child = intersectNode(first_child_to_visit, ray, t, hit_triangle);
    }

    bool hit_in_second_child = false;
    if (second_child_to_visit) { 
        float t_to_second_child_box_min;
        if (second_child_to_visit->bbox.intersect(ray, t_to_second_child_box_min) && t_to_second_child_box_min < t) {
            hit_in_second_child = intersectNode(second_child_to_visit, ray, t, hit_triangle);
        }
    }

    return hit_in_first_child || hit_in_second_child;
}