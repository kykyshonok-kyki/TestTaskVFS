#include <string>
#include "VFS.hpp"

int main( int argc, char **argv )
{
	TestTask::File *vfs_file;
	TestTask::VFS vfs;

	vfs_file = vfs.Create("My_Top_File");
	return (0);
}