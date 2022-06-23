#include <iostream>
#include "VFS.hpp"

std::ostream &operator<<( std::ostream& s, TestTask::File &f )
{
	s << "Address: " << f.addr << std::endl \
	  << "Pointer: " << f.p << std::endl \
	  << "Name:    " << f.name << std::endl \
	  << "Next:    " << f.content.next << std::endl \
	  << "Mod:     " << (int)f.content.mod << std::endl \
	  << "Extra:   " << f.content.addr_extra;
	return (s);
}

std::ostream &operator<<( std::ostream& s, TestTask::Content &c )
{
	s << std::hex \
	<< "Next:    " << c.next << std::endl \
	<< "Mod:     " << (int)c.mod << std::endl \
	<< "Extra:   " << c.addr_extra;
	return (s);
}

int main()
{
	TestTask::File *vfs_file;
	TestTask::VFS vfs;

	vfs_file = vfs.Create("My_Top_File");
	std::cout << *vfs_file << std::endl;

	std::cout << "-----------------" << std::endl;

	vfs_file = vfs.Open("My_Top_File");
	if (vfs_file)
		std::cout << *vfs_file << std::endl;
	else
		std::cout << "Error: File not found!" << std::endl;
	return (0);
}