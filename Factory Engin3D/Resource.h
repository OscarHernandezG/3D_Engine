#ifndef __Resource_H__
#define __Resource_H__

#include <string>

enum ResourceType
{
	mesh,
	texture
};
class Resource
{
public:
	Resource() {}
	Resource(ResourceType type, const char* path);
	~Resource();

	bool operator==(Resource comp);

public:
	std::string name;
	std::string file;
	ResourceType type;

	int usage = 0;
};
#endif //__Resource_H__
