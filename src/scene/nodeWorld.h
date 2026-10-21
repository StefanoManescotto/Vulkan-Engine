#pragma once

#include "scene/transform.h"
#include <fmt/base.h>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
// #define GLM_ENABLE_EXPERIMENTAL
// #include <glm/gtx/matrix_decompose.hpp>

class Node {
public:
	uint32_t meshId = 0;
	uint32_t parentId = 0;
	uint32_t nextSiblingId = 0;
	uint32_t firstChildId = 0;

	Transform& getTransform() {
		return m_transform;
	}

private:
	Transform m_transform;
};

class NodeWorld {
public:
	uint32_t rootNodeId = 0;
    uint32_t lastRootNodeId = 0;

	void initialize(const size_t maxNodes) {
		m_maxNodes = maxNodes;
		m_nodes.reserve(m_maxNodes); // TODO: Use a paged pool perhaps
	}

	size_t getMaxNodes() const { return m_maxNodes; }

	std::pair<Node &, uint32_t> createNode() {
		// fmt::print("Size: {}\nMaxNodes: {}\n", m_nodes.size(), m_maxNodes);
		assert(m_nodes.size() < m_maxNodes && "Node world is at capacity");
		m_nodes.push_back(Node{});
		uint32_t nodeId = m_nodes.size();
		return { m_nodes[nodeId - 1], nodeId };
	}

	Node &getNode(uint32_t nodeId) {
		assert(nodeId > 0 && "Tried retrieving a node with nil ID");
		return m_nodes[nodeId - 1];
	}

	// uint32_t getRootNodeId(){
	// 	return m_rootNodeId;
	// }

	// uint32_t getLastRootNodeId(){
	// 	return m_lastRootNodeId;
	// }

	// void setRootNodeId(){
	// 	return m_rootNodeId;
	// }

	// void setLastRootNodeId(){
	// 	return m_lastRootNodeId;
	// }

private:
	std::vector<Node> m_nodes;
	size_t m_maxNodes = 0;
};
