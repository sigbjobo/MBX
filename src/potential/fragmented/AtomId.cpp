#include <string>
#include <iostream>

#include "potential/fragmented/AtomId.h"

AtomId::AtomId(const std::string id) : id(id) {

}

std::string AtomId::get_id() const {
    return this->id;
}

bool AtomId::operator==(const AtomId other) const {
    return this->id.compare(other.id) == 0;
}

bool AtomId::operator!=(const AtomId other) const {
    return this->id.compare(other.id) != 0;
}

std::ostream& operator<<(std::ostream& os, const AtomId& id) {
	os << id.id;
    return os;
}
