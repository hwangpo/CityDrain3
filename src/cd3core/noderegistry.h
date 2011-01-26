#ifndef NODEREGISTRY_H
#define NODEREGISTRY_H
#ifndef PYTHON_DISABLED
#include <Python.h>
#endif

#include <map>
#include <string>
#include <vector>
#include <cd3globals.h>
#include <boost/shared_ptr.hpp>
#include <flowdefinition.h>
using namespace boost;

class INodeFactory;
class Node;

typedef std::map<std::string, INodeFactory *> reg_node_type;

class CD3_PUBLIC NodeRegistry
{
public:
	NodeRegistry();
	~NodeRegistry();

	bool addNodeFactory(INodeFactory *factory);
	void addNativePlugin(const std::string &plugin_path);

#ifndef PYTHON_DISABLED
	void addPythonPlugin(const std::string &script);
	static void addToPythonPath(std::string p);
#endif

	std::vector<std::string> getRegisteredNames() const;

	std::map<std::string, std::vector<std::string> >
	getRegisteredNamesAndSources() const;

	Node *createNode(const std::string &name) const;
	bool contains(const std::string &name) const;

private:
	reg_node_type registered_nodes;
#ifndef PYTHON_DISABLED
	static std::vector<std::string> python_paths;
	PyObject *main_namespace;
#endif
};

typedef void (*regNodeFunProto) (NodeRegistry *reg);

#endif // NODEREGISTRY_H
