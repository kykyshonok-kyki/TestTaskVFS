#include <iostream>
#include "VFS.hpp"

std::ostream &operator<<( std::ostream& s, TestTask::File &f )
{
	s << std::hex \
	<< "Address: " << f.addr << std::endl \
	<< "Pointer: " << std::dec << f.p << std::endl \
	<< "Name:    " << f.name << std::endl \
	<< "Filled:  " << f.content.filled << std::endl \
	<< "Mod:     " << std::hex << (int)f.content.mod << std::endl \
	<< "Next:    " << f.content.next << std::endl \
	<< "Extra:   " << f.content.addr_extra << std::dec;
	return (s);
}

std::ostream &operator<<( std::ostream& s, TestTask::Content &c )
{
	s << std::dec \
	<< "Filled:  " << c.filled << std::endl \
	<< "Mod:     " << std::hex << (int)c.mod << std::endl \
	<< "Next:    " << c.next << std::endl \
	<< "Extra:   " << c.addr_extra << std::dec;
	return (s);
}

int main()
{
	// TestTask::File *vfs_file;
	TestTask::VFS vfs;
	std::vector<std::string> path;

	path = vfs._TrimCStr("/my/top/path", '/');

	std::vector<std::string>::iterator end = path.end();
	for (std::vector<std::string>::iterator it = path.begin(); it != end; ++it)
	{
		std::cout << *it << std::endl;
	}
	std::cout << "-----------" << std::endl;

	path = vfs._TrimCStr("/", '/');

	end = path.end();
	for (std::vector<std::string>::iterator it = path.begin(); it != end; ++it)
	{
		std::cout << *it << std::endl;
	}
	std::cout << "-----------" << std::endl;

	path = vfs._TrimCStr("/my", '/');

	end = path.end();
	for (std::vector<std::string>::iterator it = path.begin(); it != end; ++it)
	{
		std::cout << *it << std::endl;
	}
	std::cout << "-----------" << std::endl;

	path = vfs._TrimCStr("my//top/path", '/');

	end = path.end();
	for (std::vector<std::string>::iterator it = path.begin(); it != end; ++it)
	{
		std::cout << *it << std::endl;
	}

	return (0);
}
