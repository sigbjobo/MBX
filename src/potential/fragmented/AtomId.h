#pragma once

#include <string>
#include <iostream>

class AtomId
{

    public:

        /*
        * Constructs a new AtomId.
        *
        * @param id The name of this atom type. ("H", "C", "O", etc.)
        *
        * @return A new AtomId.
        */
        AtomId(const std::string id);

        std::string get_id() const;
        
        bool operator==(const AtomId other) const;
        bool operator!=(const AtomId other) const;

        friend std::ostream& operator<<(std::ostream& os, const AtomId& id);

    private:
        std::string id;

};
