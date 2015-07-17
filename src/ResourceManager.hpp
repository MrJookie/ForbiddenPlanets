#ifndef RESOURCEMANAGER_HPP_
#define RESOURCEMANAGER_HPP_

#include <SFML/Graphics.hpp>

#include <string>
#include <map>

template <class T>
class ResourceManager
{
public:
	ResourceManager<T>() { };

	void load(const std::string& name, const std::string& filename);
	T& get(const std::string& texture);

private:
	std::map<std::string, T> resources;
};

#endif /* RESOURCEMANAGER_HPP_ */
