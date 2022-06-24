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
	TestTask::File *vfs_file;
	TestTask::VFS vfs;


	vfs_file = vfs.Open("My_Top_File2");
	if (vfs_file)
	{
		std::cout << *vfs_file << std::endl;
		vfs.Close(vfs_file);
	}
	else
		std::cout << "Error: Open: File is not available!" << std::endl;


	std::cout << "-----------------" << std::endl;


	vfs_file = vfs.Create("My_Top_File2");
	
	if (!vfs_file)
	{
		std::cout << "Error: Create: File is not available!" << std::endl;
		return (0);
	}
	char str[4096];
	memset(str, '\3', 4096);
	std::cout << "Write: " << vfs.Write(vfs_file, str, 4000) << std::endl;
	std::cout << "Write: " << vfs.Write(vfs_file, str, 1000) << std::endl;
	std::cout << "-----------------" << std::endl;

	std::cout << *vfs_file << std::endl;
	vfs.Close(vfs_file);

	std::cout << "-----------------" << std::endl;

	vfs_file = vfs.Open("My_Top_File2");
	if (vfs_file)
	{
		std::cout << *vfs_file << std::endl;

		vfs.Read(vfs_file, str, 100);
		for (size_t i = 0; i < 100; ++i)
		{
			if (!(i % 10))
				std::cout << std::endl;
			std::cout << static_cast<int>(str[i]) << " ";
		}
		std::cout << std::endl;		

		vfs.Close(vfs_file);
	}
	else
		std::cout << "Error: Open: File is not available!" << std::endl;
	return (0);
}
