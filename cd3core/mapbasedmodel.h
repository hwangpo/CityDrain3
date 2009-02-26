#ifndef MAPBASEMODEL_H
#define MAPBASEMODEL_H

#include "model.h"

#include <map>
#include <set>

typedef std::map<std::string, Node *> node_map;
typedef std::map<Node *, std::vector<next_node_type > > connection_type;


class MapBasedModel : public IModel
{
public:
	MapBasedModel();
	~MapBasedModel();

	std::string serialize() { //TODO implement
		return "";
	}

	void deserialize(const std::string &serialid) { //TODO implement
		(void) serialid;
	}

	void addNode(const std::string &name, Node *node);
	void addConnection(const std::string &src_node,
					   const std::string &src_port,
					   const std::string &sin_node,
					   const std::string &sin_port);


	//call before adding Connections
	void initNodes(const SimulationParameters &);

	node_set_type sourceNodes();
	node_set_type sinkNodes();

	std::vector<next_node_type> forward(Node *n);
	std::vector<next_node_type> backward(Node *n);

	const node_set_type *getNodes() const;

private:
	node_set_type all_nodes;
	node_set_type sink_nodes;
	node_set_type source_nodes;
	node_map names_nodes;
	connection_type fwd_connections;
	connection_type bwd_connections;

};

#endif // MAPBASEMODEL_H