#pragma once

#include <vector>
#include <iostream>

#include "potential/fragmented/AtomId.h"
#include "potential/fragmented/ConnectivityMatrix.h"

class LocalSystem
{

    public:

        /*
        * Constructor for LocalSystem.
        *
        * @param
        * @return a new LocalSystem.
        *
        */
        LocalSystem(std::vector<AtomId*> ids, ConnectivityMatrix con_matrix);

        int get_num_atoms() const;

        std::vector<AtomId*> get_atom_ids() const;
        ConnectivityMatrix get_con_matrix() const;

        std::vector<std::vector<int> > find_isomporphic_subgraphs(const LocalSystem other) const;

        bool operator==(LocalSystem other) const;
            
        friend std::ostream& operator<<(std::ostream& os, const LocalSystem& local_sys);

    private:

        std::vector<AtomId*> ids;
        ConnectivityMatrix con_matrix;

        std::vector<std::vector<int> > find_isomorphic_subgraphs_helper(const LocalSystem other, std::vector<int> pairs) const;
        bool is_potential_isomorphic_subgraph(const LocalSystem other, std::vector<int> pairs) const;

};
