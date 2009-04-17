#ifndef MODEL_H
#define MODEL_H

#include <string>
#include <vector>
#include <boost/unordered/unordered_map.hpp>
#include <boost/unordered/unordered_set.hpp>
#include <boost/tuple/tuple.hpp>
#include <cd3globals.h>

class Node;
struct SimulationParameters;

using namespace boost;

typedef unordered_map<std::string, Node *> name_node_map;
typedef unordered_set<Node *> node_set_type;
typedef std::pair<Node *, std::string> end_point_type;
typedef tuple<std::string, Node *, std::string> next_node_type;

class CD3_PUBLIC IModel {
public:
	virtual ~IModel(){}

	virtual std::string serialize() = 0;
	virtual void deserialize(const std::string &serialid) = 0;

	virtual void addNode(const std::string &name, Node *node) = 0;
	virtual void addConnection(const std::string &src_node,
					   const std::string &src_port,
					   const std::string &sin_node,
					   const std::string &sin_port) = 0;

	virtual void initNodes(const SimulationParameters &) = 0;
	virtual node_set_type getSourceNodes() = 0;
	virtual node_set_type getSinkNodes() = 0;

	virtual std::vector<next_node_type> forward(Node *n) = 0;
	virtual std::vector<next_node_type> backward(Node *n) = 0;

	virtual const node_set_type *getNodes() const = 0;
	virtual name_node_map getNamesAndNodes() const = 0;
	virtual Node *getNode(const std::string &name) const = 0;
	virtual bool connected() const = 0;
};

#endif // MODEL_H
