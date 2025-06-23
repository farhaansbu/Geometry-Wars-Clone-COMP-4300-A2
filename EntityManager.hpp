#ifndef ENTITYMANAGER_HPP
#define ENTITYMANAGER_HPP

#include "Entity.hpp"
#include <map>

using EntityVec = std::vector<std::shared_ptr<Entity>>;

class EntityManager
{
	EntityVec							m_entities;
	EntityVec							m_entitiesToAdd;
	std::map<std::string, EntityVec>	m_entityMap;
	size_t								m_totalEntities{ 0 };

	void removeDeadEntites(EntityVec& Vec)
	{
		for (auto it = Vec.begin(); it != Vec.end();)
		{
			// If no longer active, delete
			if ( !((*it)->isActive()) )
			{
				it = Vec.erase(it);
			}
			else
				++it;
		}
	}

public:
	EntityManager() = default;

	void update()
	{
		//	add entities from m_entitiesToAdd to proper locatoins

		for (auto it = m_entitiesToAdd.begin(); it != m_entitiesToAdd.end();)
		{
			m_entities.push_back(*it);
			it = m_entitiesToAdd.erase(it);
		}

		// remove dead entities from vector of all entities
		removeDeadEntites(m_entities);

		// C++20 way of iterating through [key, value] pairs in a map
		for (auto& [tag, entityVec] : m_entityMap)
		{
			removeDeadEntites(entityVec);
		}

	} // End of update function

	std::shared_ptr<Entity> addEntity(const std::string& tag)
	{
		// Create entity shared pointer
		auto entity = std::shared_ptr<Entity>(new Entity(m_totalEntities++, tag));

		//Add it to vec of entities to be added
		m_entitiesToAdd.push_back(entity);

		//Add to entity map
		if (m_entityMap.find(tag) == m_entityMap.end())
		{
			m_entityMap[tag] = EntityVec();
		}

		m_entityMap[tag].push_back(entity);

		return entity;
	}

	const EntityVec& getEntities()
	{
		return m_entities;
	}

	const EntityVec& getEntities(const std::string& tag)
	{
		if (m_entityMap.find(tag) == m_entityMap.end())
			m_entityMap[tag] = EntityVec();

		return m_entityMap[tag];
	}

	const std::map<std::string, EntityVec>& getEntityMap()
	{
		return m_entityMap;
	}

};

#endif // !ENTITYMANAGER_HPP
