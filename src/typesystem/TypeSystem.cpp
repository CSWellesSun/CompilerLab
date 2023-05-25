#include "typesystem/TypeSystem.h"
#include "common/Defs.h"
#include "lexer/Token.h"
using namespace minisolc;

void TypeSystem::setType(std::string identifier, Type type) {
	auto map = m_maps.back();
	if (map.find(identifier) == map.end()) {
		// Don't find
		map.insert({identifier, type});
	} else {
		LOG_ERROR("Redefinition!");
	}
}

Type TypeSystem::getType(std::string identifier) {
	for (auto it = m_maps.rbegin(); it != m_maps.rend(); ++it) {
		auto map = *it;
		if (map.find(identifier) != map.end()) {
			// Find
			return map.at(identifier);
		}
	}

	// Don't find
	LOG_ERROR("Don't Find!");
	return Type::UNKNOWN;
}
void TypeSystem::analyze() {}
