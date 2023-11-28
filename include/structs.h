#ifndef STRUCTS_H
#define STRUCTS_H

#include <glm/glm.hpp>
#include <map>
#include <functional>

struct MyUniforms {
    glm::mat4 projectionMatrix;
    glm::mat4 viewMatrix;
    glm::mat4 modelMatrix;
    glm::mat4 model2Matrix;
};

struct CameraState{
    glm::vec3 pos;
    glm::vec3 front;
    glm::vec3 up;
};

struct Edge{
    ///Node A
    int a;
    ///Node B
    int b;
    ///Opposite vertex
    int o;

    Edge(int _a, int _b, int _o) : a(_a), b(_b), o(_o){}
};

struct EdgeComparer{
    bool operator()(const Edge& e1, const Edge& e2)const
    {
        return !((e1.a == e2.a && e1.b == e2.b) || (e1.a == e2.b && e1.b == e2.a));
    }
};

namespace std {
    template <>
    struct hash<Edge> {
        std::size_t operator()(const Edge& e) const {
            // Use a combination of hash values for individual members
            std::size_t hashValue = 17;
            hash_combine(hashValue, std::min(e.a, e.b));
            hash_combine(hashValue, std::max(e.a, e.b));
            return hashValue;
        }

        // Helper function to combine hash values
        template <class T>
        void hash_combine(std::size_t& seed, const T& value) const {
            std::hash<T> hasher;
            seed ^= hasher(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }
    };
}

#endif