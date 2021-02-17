#include <vector>
#include <iostream>
#include <algorithm>

#include "potential/fragmented/LocalSystem.h"

LocalSystem::LocalSystem(std::vector<AtomId*> ids, ConnectivityMatrix con_matrix) :
    con_matrix(con_matrix) {

    this->ids = ids;

}

int LocalSystem::get_num_atoms() const {
	return this->ids.size();
}

std::vector<AtomId*> LocalSystem::get_atom_ids() const {
    return ids;
}

ConnectivityMatrix LocalSystem::get_con_matrix() const {
    return con_matrix;
}

std::vector<std::vector<int> > LocalSystem::find_isomporphic_subgraphs(const LocalSystem other) const {
	return find_isomorphic_subgraphs_helper(other, std::vector<int>());
}

std::vector<std::vector<int> > LocalSystem::find_isomorphic_subgraphs_helper(const LocalSystem other, std::vector<int> pairs) const {
	
	std::vector<std::vector<int> > isomorphic_subgraphs;

	int other_index = pairs.size();

	if(other_index == other.ids.size()) {
		isomorphic_subgraphs.push_back(pairs);
		return isomorphic_subgraphs;
	}

	// Loop over each atom_id in this graph
	for(int atom_id_index = 0; atom_id_index < this->ids.size(); atom_id_index++) {
		
		// If the current atom_id has already been used in a pair, skip it.
		if(std::find(pairs.begin(), pairs.end(), atom_id_index) != pairs.end()) continue;

		pairs.push_back(atom_id_index);

		if(this->is_potential_isomorphic_subgraph(other, pairs)) {
			for(std::vector<int> isomorphic_subgraph: this->find_isomorphic_subgraphs_helper(other, pairs)) {
				isomorphic_subgraphs.push_back(isomorphic_subgraph);
			}
		}
		pairs.pop_back();

	}

	// TODO: make this more efficient
	// Eliminate all isomorphic subgraphs that are duplicates

	std::vector<std::vector<int> > sorted_subgraphs;
	std::vector<std::vector<int> > final_isomorphic_subgraphs;


	for(std::vector<int> isomorphic_subgraph: isomorphic_subgraphs) {
		std::vector<int> sorted_subgraph = isomorphic_subgraph;
		std::sort(sorted_subgraph.begin(),sorted_subgraph.end());

		if(std::find(sorted_subgraphs.begin(), sorted_subgraphs.end(), sorted_subgraph) == sorted_subgraphs.end()) {
			sorted_subgraphs.push_back(sorted_subgraph);
			final_isomorphic_subgraphs.push_back(isomorphic_subgraph);
		}
	}

	return final_isomorphic_subgraphs;
}

bool LocalSystem::is_potential_isomorphic_subgraph(const LocalSystem other, std::vector<int> pairs) const {

	for(int other_index = 0; other_index < pairs.size(); other_index++) {
		int this_index = pairs[other_index];

		if(*(this->ids[this_index]) != *(other.ids[other_index])) return false;

	}

	for(int other_index1 = 0; other_index1 < pairs.size(); other_index1++) {
		int this_index1 = pairs[other_index1];

		for(int other_index2 = 0; other_index2 < pairs.size(); other_index2++) {
			int this_index2 = pairs[other_index2];

			if(this->con_matrix.has_connection(this_index1, this_index2) != other.con_matrix.has_connection(other_index1, other_index2)) return false;

		}

	}

	return true;
}

bool LocalSystem::operator==(LocalSystem other) const {
	if(this->ids.size() != other.ids.size()) return false;

	for(int atom_id_index = 0; atom_id_index < this->ids.size(); atom_id_index++) {
		if(*(this->ids[atom_id_index]) != *(other.ids[atom_id_index])) return false;
	}
    // TODO might need to check values of pointers in ids.
    return this->con_matrix == other.con_matrix;

}

std::ostream& operator<<(std::ostream& os, const LocalSystem& local_sys) {
	os << "Num Atoms: " << local_sys.ids.size() << std::endl;
	for(int id_index = 0; id_index < local_sys.ids.size(); id_index++) {
		os << *(local_sys.ids[id_index]) << " ";
	}
	os << std::endl;
	os << "Connectivity Matrix: " << std::endl << local_sys.con_matrix;
	return os;
}
