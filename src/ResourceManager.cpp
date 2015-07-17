#include "ResourceManager.hpp"

template <class T>
void ResourceManager<T>::load(const std::string& name, const std::string& filename)
{
	T resource;
	resource.loadFromFile(filename);

	this->resources[name] = resource;

	return;
}

template <class T>
T& ResourceManager<T>::get(const std::string& texture)
{
	return this->resources.at(texture);
}

//template instantiation
template class ResourceManager<sf::Texture>;
template class ResourceManager<sf::Font>;