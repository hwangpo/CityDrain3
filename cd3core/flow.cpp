#include "flow.h"
#include <boost/unordered/unordered_map.hpp>
#include <calculationunit.h>
#include <boost/foreach.hpp>
#include <cd3assert.h>

using namespace boost;

struct FlowDefinition {
	std::vector<std::string> names;
        unordered_map<const CalculationUnit *, std::vector<std::string> > unit_names;
        unordered_map<std::string, int> positions;
        unordered_map<std::string, const CalculationUnit *> units;
};

Flow::Flow() {
	f = boost::shared_ptr<FlowPriv>(new FlowPriv());
	fd = boost::shared_ptr<FlowDefinition>(new FlowDefinition());
}

Flow::Flow(const Flow &other) {
	f = other.f;
	fd = other.fd;
}

Flow Flow::nullFlow() {
	Flow f;
	f.addUnit("flow", CU::flow, 0.0);
	return f;
}

Flow::~Flow() {

}

Flow &Flow::operator =(const Flow &other) {
	f = other.f;
	fd = other.fd;
	return *this;
}

void Flow::copy() {
	copyData();
	copyDefinition();
}

void Flow::copyData() {
	if (!f.unique()) {
		FlowPriv *old = f.get();
		f = boost::shared_ptr<FlowPriv>(new FlowPriv(*old));
	}
}

void Flow::copyDefinition() {
	if (!fd.unique()) {
		FlowDefinition *old = fd.get();
		fd = boost::shared_ptr<FlowDefinition>(new FlowDefinition(*old));
	}
}

void Flow::addUnit(const std::string &name,
				 const CalculationUnit *unit,
				 double value) {
	cd3assert(fd->positions.find(name) == fd->positions.end(), "name already defined");
	cd3assert(unit, "unit is null");

	copyDefinition();
	copyData();
	f->push_back(value);
	fd->names.push_back(name);
	if (fd->unit_names.find(unit) == fd->unit_names.end())
		fd->unit_names[unit] = std::vector<std::string>();
	fd->unit_names[unit].push_back(name);
	fd->positions[name] = f->size()-1;
	fd->units[name] = unit;
}

void Flow::setValue(const std::string &name,
					double value) {
	cd3assert(fd->positions.find(name) != fd->positions.end(), "no such name");
	copyData();
	(*f)[fd->positions[name]] = value;
}

double Flow::getValue(const std::string &name) const {
	cd3assert(fd->positions.find(name) != fd->positions.end(), "no such name");
	return (*f)[fd->positions[name]];
}

const CalculationUnit *Flow::getUnit(const std::string &name) const {
	cd3assert(fd->positions.find(name) != fd->positions.end(), "no such name");
	return fd->units[name];
}

const std::vector<std::string> & Flow::getNames() const {
	return fd->names;
}

const std::vector<std::string> &
Flow::getUnitNames(const CalculationUnit *unit) const{
	cd3assert(unit, "null unit not allowed");
	cd3assert(fd->unit_names.find(unit) != fd->unit_names.end(), "no such unit");
	return fd->unit_names[unit];
}

bool Flow::hasName(const std::string &name) const {
	return std::count(fd->names.begin(), fd->names.end(), name) > 0;
}

bool Flow::empty() const {
	return fd->names.empty();
}

void Flow::dump() const {
	BOOST_FOREACH(std::string name, fd->names) {
		std::cout << "flow.dump.names " << name << std::endl;
	}
}

unsigned int Flow::countUnits(const CalculationUnit *unit) const {
	return fd->unit_names[unit].size();
}

void Flow::setIth(const CalculationUnit *unit, size_t i, double value) {
	cd3assert(fd->unit_names.find(unit) != fd->unit_names.end(), "no such unit");
	cd3assert(fd->unit_names[unit].size() > i, "ith is too much");
	copyData();
	(*f)[fd->positions[fd->unit_names[unit][i]]] = value;
}

double Flow::getIth(const CalculationUnit *unit, size_t i) const {
	cd3assert(fd->unit_names.find(unit) != fd->unit_names.end(), "no such unit");
	cd3assert(fd->unit_names[unit].size() > i, "ith is too much");
	return (*f)[fd->positions[fd->unit_names[unit][i]]];
}

void Flow::clear() {
	copyData();
	for (size_t i = 0; i < fd->names.size(); i++) {
		(*f)[i] = 0.0;
	}
}
