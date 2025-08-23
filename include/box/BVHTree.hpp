#ifndef RENDERERBUILD_BVHTREE_HPP
#define RENDERERBUILD_BVHTREE_HPP

#include <hittable/Transform.hpp>

namespace renderer {
    class BVHTree {
    public:
        static constexpr Uint32 PRIMITIVE_COUNT_PER_LEAF_NODE = 4;

        struct BVHTreeNode {
            //当前节点的包围盒
            BoundingBox boundingBox;

            /*
             * 一个叶子节点可以包含多个图元：如果primitiveCount大于0，则为叶子节点
             * 需要一个另外的图元索引数组承接此处的索引
             * 如果为叶子节点，则index为图元索引数组的起始下标
             * 如果为中间节点，则index为左子节点的下标
             */
            size_t primitiveCount {};
            size_t index {};
        };

    private:
        //构建过程的任务结构体
        struct BuildingTask {
            //当前任务包含的图元列表，当只有一个图元时成为叶子节点
            //使用起始下标和元素个数表示当前任务对象，而不需要拷贝所有任务对象
            size_t primitiveStartIndex;
            size_t primitiveCount;

            //该任务对应的节点在线性数组中的位置
            size_t nodeIndex;
        };

        //构造BVH树的过程中统一数据表示形式
        struct PrimitiveInfo {
            //图元的包围盒和重心，决定如何分割图元列表
            BoundingBox boundingBox;
            Point3 centroid;

            //图元的标识符
            PrimitiveType type {};
            size_t index {}; //在原始数组中的引用
        };

        //为图元列表构造包围盒
        static BoundingBox constructListBoundingBox(const std::vector<PrimitiveInfo> & primitives, size_t startIndex, size_t endIndex) {
            BoundingBox ret = primitives[startIndex].boundingBox;
            for (size_t i = startIndex + 1; i < (endIndex > primitives.size() ? primitives.size() : endIndex); i++) {
                ret = BoundingBox(ret, primitives[i].boundingBox);
            }
            return ret;
        }

    public:
        /*
         * 使用物体列表构造BVH节点数组
         * 构建方式为迭代式构建，广度优先。经典递归式构建为深度优先
         * 由CPU执行
         */
        static std::pair<std::vector<BVHTreeNode>, std::vector<std::pair<PrimitiveType, size_t>>> constructBVHTree(
                const std::vector<Sphere> & spheres,
                const std::vector<Triangle> & triangles,
                const std::vector<Parallelogram> & parallelograms,
                const std::vector<Transform> & transforms,
                const std::vector<Box> & boxes)
        {
            //构造统一数据列表
            std::vector<PrimitiveInfo> spherePrimitiveArray(spheres.size(), PrimitiveInfo());
            for (size_t i = 0; i < spheres.size(); i++) {
                auto & element = spherePrimitiveArray[i];
                element.boundingBox = spheres[i].constructBoundingBox(); //预存储包围盒，不用在构造整体包围盒时重复计算图元包围盒
                element.centroid = spheres[i].center.origin;
                element.type = PrimitiveType::SPHERE;
                element.index = i;
            }

            std::vector<PrimitiveInfo> trianglePrimitiveArray(triangles.size(), PrimitiveInfo());
            for (size_t i = 0; i < triangles.size(); i++) {
                auto & element = trianglePrimitiveArray[i];
                element.boundingBox = triangles[i].constructBoundingBox();
                element.centroid = triangles[i].centroid();
                element.type = PrimitiveType::TRIANGLE;
                element.index = i;
            }

            std::vector<PrimitiveInfo> parallelogramPrimitiveArray(parallelograms.size(), PrimitiveInfo());
            for (size_t i = 0; i < parallelograms.size(); i++) {
                auto & element = parallelogramPrimitiveArray[i];
                element.boundingBox = parallelograms[i].constructBoundingBox();
                element.centroid = parallelograms[i].centroid();
                element.type = PrimitiveType::PARALLELOGRAM;
                element.index = i;
            }

            std::vector<PrimitiveInfo> transformPrimitiveArray(transforms.size(), PrimitiveInfo());
            for (size_t i = 0; i < transforms.size(); i++) {
                auto & element = transformPrimitiveArray[i];
                element.boundingBox = transforms[i].transformedBoundingBox;
                element.centroid = transforms[i].transformedCentroid;
                element.type = PrimitiveType::TRANSFORM;
                element.index = i;
            }

            std::vector<PrimitiveInfo> boxPrimitiveArray(boxes.size(), PrimitiveInfo());
            for (size_t i = 0; i < boxes.size(); i++) {
                auto & element = boxPrimitiveArray[i];
                element.boundingBox = boxes[i].constructBoundingBox();
                element.centroid = boxes[i].centroid();
                element.type = PrimitiveType::BOX;
                element.index = i;
            }

            std::vector<PrimitiveInfo> primitiveArray;
            primitiveArray.insert(primitiveArray.end(), spherePrimitiveArray.begin(), spherePrimitiveArray.end());
            primitiveArray.insert(primitiveArray.end(), trianglePrimitiveArray.begin(), trianglePrimitiveArray.end());
            primitiveArray.insert(primitiveArray.end(), parallelogramPrimitiveArray.begin(), parallelogramPrimitiveArray.end());
            primitiveArray.insert(primitiveArray.end(), transformPrimitiveArray.begin(), transformPrimitiveArray.end());
            primitiveArray.insert(primitiveArray.end(), boxPrimitiveArray.begin(), boxPrimitiveArray.end());

            //分配存储空间，有N个叶子节点的二叉树共有2N-1个节点
            std::vector<BVHTreeNode> ret(2 * primitiveArray.size() - 1, BVHTreeNode());

            //图元索引数组
            std::vector<std::pair<PrimitiveType, size_t>> primitiveIndexArray(primitiveArray.size());

            //当前分配的节点数量
            size_t nodeCount = 0;
            //任务队列
            std::queue<BuildingTask> queue;

            //创建根节点任务，将整个物体数组加入根任务
            queue.push({0, primitiveArray.size(), 0});
            nodeCount++;

            while (!queue.empty()) {
                //弹出一个任务，根据类型选择不同的处理方式
                auto task = queue.front();
                queue.pop();

                auto & node = ret[task.nodeIndex];
                if (task.primitiveCount <= PRIMITIVE_COUNT_PER_LEAF_NODE) {
                    //叶子节点
                    //将当前task的所有图元添加到叶子节点中
                    node.primitiveCount = task.primitiveCount;
                    node.index = primitiveIndexArray.size();
                    node.boundingBox = constructListBoundingBox(primitiveArray, task.primitiveStartIndex, task.primitiveStartIndex + task.primitiveCount);
                    for (size_t i = 0; i < task.primitiveCount; i++) {
                        primitiveIndexArray.emplace_back(primitiveArray[task.primitiveStartIndex + i].type, primitiveArray[task.primitiveStartIndex + i].index);
                    }
                } else {
                    //中间节点，为左右子节点分配空间（分配索引空间）
                    const size_t leftChildIndex = nodeCount++;
                    const size_t rightChildIndex = nodeCount++;

                    //随机选择轴排序
                    const int axis = randomInt(0, 2);
                    std::sort(primitiveArray.begin() + (int)task.primitiveStartIndex,
                              primitiveArray.begin() + (int)(task.primitiveStartIndex + task.primitiveCount),
                              [axis](const PrimitiveInfo & a, const PrimitiveInfo & b) {
                                  return a.centroid[axis] < b.centroid[axis];});

                    //创建当前节点
                    node.boundingBox = constructListBoundingBox(primitiveArray, task.primitiveStartIndex, task.primitiveStartIndex + task.primitiveCount);
                    node.primitiveCount = 0;
                    node.index = leftChildIndex;

                    //分割图元列表，根据空间排序结果确定左右子树的所有图元
                    //创建左右节点的子任务并推到队列中，下一次循环先处理左子节点
                    const size_t mid = task.primitiveCount / 2;

                    queue.push({task.primitiveStartIndex, mid, leftChildIndex});
                    queue.push({task.primitiveStartIndex + mid, task.primitiveCount - mid, rightChildIndex});
                }
            }
            return {ret, primitiveIndexArray};
        }

        //相交测试（递归式）
        /*
        static bool traverse(const std::vector<BVHTreeNode> & nodeArray, const std::vector<Sphere> & primitives,
                             const Ray & ray, const Range & range, HitRecord & record, size_t index)
        {
            //检查是否和当前节点的包围盒相交
            if (!nodeArray[index].boundingBox.hit(ray, range)) {
                return false;
            }

            //分为检查下一级包围盒和物体本身两种可能
            if (nodeArray[index].isLeaf) {
                return primitives[nodeArray[index].primitiveID].hit(ray, range, record);
            } else {
                //递归检查左右子节点
                const size_t leftID = nodeArray[index].leftChildID;
                const size_t rightID = leftID + 1;

                //如果左侧已经发生碰撞，则忽略t值更大的碰撞
                const bool hitLeft = traverse(nodeArray, primitives, ray, range, record, leftID);
                const bool hitRight = traverse(nodeArray, primitives, ray, Range(range.min, hitLeft ? record.t : range.max), record, rightID);
                return hitLeft || hitRight;

                *
                HitRecord leftRecord, rightRecord;
                const bool hitLeft = traverse(nodeArray, primitives, ray, range, leftRecord, leftID);
                Range rightRange = range;
                if (hitLeft) {
                    rightRange.max = leftRecord.t; // 缩小右子树的检测范围
                }
                const bool hitRight = traverse(nodeArray, primitives, ray, rightRange, rightRecord, rightID);
                if (hitLeft && hitRight) {
                    record = (leftRecord.t < rightRecord.t) ? leftRecord : rightRecord;
                    return true;
                }
                if (hitLeft) { record = leftRecord; return true; }
                if (hitRight) { record = rightRecord; return true; }
                return false;
                 *
            }
        }
        */

        //相交测试（栈迭代式），由GPU线程执行
        static bool hit(const BVHTreeNode * tree, const std::pair<PrimitiveType, size_t> * indexArray,
                        const Sphere * spheres,
                        const Triangle * triangles,
                        const Parallelogram * parallelograms,
                        const Transform * transforms,
                        const Box * boxes,
                        const Ray & ray, const Range & range, HitRecord & record)
        {
            //return traverse(nodeArray, primitives, ray, range, record, 0);

            //待访问节点索引
            size_t stack[64];      //GPU上不能使用std::stack
            size_t topIndex = 0;   //栈的当前size
            stack[topIndex++] = 0; //stack.push(0)

            HitRecord tempRecord;
            bool isHit = false;
            Range currentRange(range);

            while (topIndex > 0) {
                const size_t index = stack[--topIndex]; //弹出栈顶元素。前置--对应后置++

                //检查是否和当前节点的包围盒相交
                double t;
                if (!tree[index].boundingBox.hit(ray, currentRange, t)) {
                    continue;
                }

                //相交，分为叶子节点和中间节点两种情况
                const auto & node = tree[index];
                if (node.primitiveCount > 0) {
                    //叶子节点
                    //遍历叶子中的所有图元，依次进行相交测试
                    for (size_t i = 0; i < node.primitiveCount; i++) {
                        const auto & pair = indexArray[node.index + i];
                        switch (pair.first) {
                            case PrimitiveType::SPHERE:
                                if (spheres[pair.second].hit(ray, currentRange, tempRecord)) {
                                    isHit = true;
                                    currentRange.max = tempRecord.t;
                                    record = tempRecord;
                                }
                                break;
                            case PrimitiveType::TRIANGLE:
                                if (triangles[pair.second].hit(ray, currentRange, tempRecord)) {
                                    isHit = true;
                                    currentRange.max = tempRecord.t;
                                    record = tempRecord;
                                }
                                break;
                            case PrimitiveType::PARALLELOGRAM:
                                if (parallelograms[pair.second].hit(ray, currentRange, tempRecord)) {
                                    isHit = true;
                                    currentRange.max = tempRecord.t;
                                    record = tempRecord;
                                }
                                break;
                            case PrimitiveType::TRANSFORM:
                                if (transforms[pair.second].hit(ray, currentRange, tempRecord)) {
                                    isHit = true;
                                    currentRange.max = tempRecord.t;
                                    record = tempRecord;
                                }
                                break;
                            case PrimitiveType::BOX:
                                if (boxes[pair.second].hit(ray, currentRange, tempRecord)) {
                                    isHit = true;
                                    currentRange.max = tempRecord.t;
                                    record = tempRecord;
                                }
                                break;
                            default:;
                        }
                    }
                } else {
                    //中间节点。将左右子节点的索引入栈
                    //优先处理更近的节点，将更远的子节点先入栈，后处理
                    const size_t leftID = node.index;
                    const size_t rightID = leftID + 1;

                    /*
                    if (ray.direction[0] < 0) { //如果光线朝x负方向，则x大的节点更近
                                                //交换左右顺序（这里假设node[left]的x比node[right]小）
                        std::swap(leftID, rightID);
                    }
                     */

                    double tLeft, tRight;
                    tree[leftID].boundingBox.hit(ray, currentRange, tLeft);
                    tree[rightID].boundingBox.hit(ray, currentRange, tRight);

                    //先推入t值大的节点下标
                    //预过滤：只有相交的节点才入栈，避免二次包围盒相交测试
                    const bool hitLeft = tree[leftID].boundingBox.hit(ray, currentRange, tLeft);
                    const bool hitRight = tree[rightID].boundingBox.hit(ray, currentRange, tRight);

                    if (hitLeft && hitRight) {
                        //先推入t值大的（远的）节点，后推入t值小的（近的）节点
                        if (tLeft > tRight) {
                            stack[topIndex++] = leftID;
                            stack[topIndex++] = rightID;
                        } else {
                            stack[topIndex++] = rightID;
                            stack[topIndex++] = leftID; //取消调用swap
                        }
                    } else if (hitLeft) {
                        stack[topIndex++] = leftID;
                    } else if (hitRight) {
                        stack[topIndex++] = rightID;
                    }
                }
            }
            return isHit;
        }
    };
}

#endif //RENDERERBUILD_BVHTREE_HPP
