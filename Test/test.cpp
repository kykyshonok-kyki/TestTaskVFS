#include <iostream>
#include "VFS.hpp"
#include "unistd.h"

TestTask::VFS vfs;

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

void OCWOTest()
{
	TestTask::File *vfs_file;


	vfs_file = vfs.Open("home/My_Top_File2");
	if (vfs_file)
	{
		std::cout << *vfs_file << std::endl;
		vfs.Close(vfs_file);
	}
	else
		std::cerr << "Error: Open: File is not available!" << std::endl;


	std::cout << "-----------------" << std::endl;


	vfs_file = vfs.Create("home/My_Top_File2");
	
	if (!vfs_file)
	{
		std::cerr << "Error: Create: File is not available!" << std::endl;
		return ;
	}
	char str[4096];
	memset(str, '\3', 4096);
	std::cout << "Write: " << vfs.Write(vfs_file, str, 4000) << std::endl;
	std::cout << "Write: " << vfs.Write(vfs_file, str, 100) << std::endl;
	std::cout << "-----------------" << std::endl;

	std::cout << *vfs_file << std::endl;
	vfs.Close(vfs_file);

	std::cout << "-----------------" << std::endl;

	vfs_file = vfs.Open("home/My_Top_File2");
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
		std::cerr << "Error: Open: File is not available!" << std::endl;
}

void FoldersTest()
{
	TestTask::File *vfs_file;

	vfs_file = vfs.Create("/home/file1");
	if (!vfs_file)
		std::cerr << "Error: Create: Cannt create file!" << std::endl;
	else
	{
		char str[4096];
		memset(str, '\1', 4096);
		vfs.Write(vfs_file, str, 4096);
		std::cout << *vfs_file << std::endl << "------------" << std::endl;
		vfs.Close(vfs_file);
	}


	vfs_file = vfs.Create("/file2");
	if (!vfs_file)
		std::cerr << "Error: Create: Cannt create file!" << std::endl;
	else
	{
		char str[4096];
		memset(str, '\2', 4096);
		vfs.Write(vfs_file, str, 4096);
		std::cout << *vfs_file << std::endl << "------------" << std::endl;
		vfs.Close(vfs_file);
	}


	vfs_file = vfs.Create("/test/file3");
	if (!vfs_file)
		std::cerr << "Error: Create: Cannt create file!" << std::endl;
	else
	{
		char str[4096];
		memset(str, '\3', 4096);
		vfs.Write(vfs_file, str, 4096);
		std::cout << *vfs_file << std::endl << "------------" << std::endl;
		vfs.Close(vfs_file);
	}


	vfs_file = vfs.Open("/home/file1");
	if (!vfs_file)
		std::cerr << "Error: Open: Cannt open file!" << std::endl;
	else
	{
		std::cout << *vfs_file << std::endl << "------------" << std::endl;
		vfs.Close(vfs_file);
	}

	vfs_file = vfs.Open("/file2");
	if (!vfs_file)
		std::cerr << "Error: Open: Cannt open file!" << std::endl;
	else
	{
		std::cout << *vfs_file << std::endl << "------------" << std::endl;
		vfs.Close(vfs_file);
	}

	vfs_file = vfs.Open("/test/file3");
	if (!vfs_file)
		std::cerr << "Error: Open: Cannt open file!" << std::endl;
	else
	{
		std::cout << *vfs_file << std::endl;
		vfs.Close(vfs_file);
	}
}

void ReadTest()
{
	TestTask::File *vfs_file;
	char str[4096];

	memset(str, 'a', 4096);

	vfs_file = vfs.Create("/text");

	// 1 блок
	vfs.Write(vfs_file, str, 100);
	memset(str, 'b', 4096);
	vfs.Write(vfs_file, str, 10000);
	// 2 блок
	memset(str, 'a', 20);
	vfs.Write(vfs_file, str, 20);
	memset(str, 'c', 4000);
	vfs.Write(vfs_file, str, 4000);
	memset(str, 'f', 4000);
	vfs.Write(vfs_file, str, 4000);
	// 3 блок
	strcpy(str, "Hello!!!!!!!!!!!!!");
	vfs.Write(vfs_file, str, 10);
	vfs.Read(vfs_file, str, 1);

	vfs.Close(vfs_file);

	vfs_file = vfs.Open("/text");
	std::cout << vfs.Read(vfs_file, str, 200) << std::endl;
	str[200] = 0;
	std::cout << vfs.Read(vfs_file, str, 5000) << std::endl;
	std::cout << vfs.Read(vfs_file, str, 5000) << std::endl;
	str[vfs.Read(vfs_file, str, 200)] = 0;
	std::cout << str << std::endl;
	vfs.Close(vfs_file);
}

typedef struct s_targs
{
	pthread_mutex_t m_cout;
} t_targs;

void *TCreate(void *raw_args)
{
	t_targs *args = reinterpret_cast<t_targs *>(raw_args);
	TestTask::File *f;
	try
	{
		f = vfs.Create("Threads_test_file");
		if (!f)
		{
			pthread_mutex_lock(&args->m_cout);
			std::cerr << "Error: Create: Cannt open file!" << std::endl;
			pthread_mutex_unlock(&args->m_cout);
			return (nullptr);
		}
		pthread_mutex_lock(&args->m_cout);
		std::cout << "(File *) " << f << std::endl;
		pthread_mutex_unlock(&args->m_cout);
		vfs.Close(f);
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}
	return (nullptr);
}

void *TWrite(void *raw_args)
{
	t_targs *args = reinterpret_cast<t_targs *>(raw_args);
	TestTask::File *f;
	try
	{
		char str[17];
		strcpy(str, "Hello, threads!");
		f = vfs.Create("Threads_test_file");
		if (!f)
		{
			pthread_mutex_lock(&args->m_cout);
			std::cerr << "Error: Create: Cannt open file!" << std::endl;
			pthread_mutex_unlock(&args->m_cout);
			return (nullptr);
		}
		pthread_mutex_lock(&args->m_cout);
		std::cout << "Write " << vfs.Write(f, str, 16) << " bytes" << std::endl;
		pthread_mutex_unlock(&args->m_cout);
		vfs.Close(f);
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}
	return (nullptr);
}

void *TRead(void *raw_args)
{
	t_targs *args = reinterpret_cast<t_targs *>(raw_args);
	TestTask::File *f;
	try
	{
		char str[10000];
		size_t read;
		f = vfs.Open("Threads_test_file");
		if (!f)
		{
			pthread_mutex_lock(&args->m_cout);
			std::cerr << "Error: Open: Cannt open file!" << std::endl;
			pthread_mutex_unlock(&args->m_cout);
			return (nullptr);
		}
		read = vfs.Read(f, str, 10000);
		str[read] = 0;
		pthread_mutex_lock(&args->m_cout);
		std::cout << "Read " << read << " bytes: " << str << std::endl;
		pthread_mutex_unlock(&args->m_cout);
		vfs.Close(f);
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}
	return (nullptr);
}

void ThreadsTest()
{
	pthread_t create, read, write;
	t_targs args;

	pthread_mutex_init(&args.m_cout, NULL);

	pthread_create(&create, nullptr, TCreate, &args);
	pthread_detach(create);
	pthread_create(&write, nullptr, TWrite, &args);
	pthread_detach(write);
	pthread_create(&read, nullptr, TRead, &args);
	pthread_detach(read);

	sleep(1);
	pthread_mutex_destroy(&args.m_cout);
}

typedef struct s_targs2
{
	pthread_mutex_t m_cout;
	TestTask::File *f;
} t_targs2;

void *T2Write(void *raw_args)
{
	t_targs2 *args2 = reinterpret_cast<t_targs2 *>(raw_args);
	try
	{
		char str[17];
		strcpy(str, "Hello, threads!");
		size_t wrote = vfs.Write(args2->f, str, 16);
		pthread_mutex_lock(&args2->m_cout);
		std::cout << "Write " << wrote << " bytes" << std::endl;
		pthread_mutex_unlock(&args2->m_cout);
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}
	return (nullptr);
}

void Threads2WriteTest()
{
	pthread_t write1, write2, write3;
	t_targs2 args2;

	pthread_mutex_init(&args2.m_cout, NULL);
	args2.f = vfs.Create("some/way/to/Threads2");

	pthread_create(&write1, nullptr, T2Write, &args2);
	pthread_detach(write1);
	pthread_create(&write2, nullptr, T2Write, &args2);
	pthread_detach(write2);
	pthread_create(&write3, nullptr, T2Write, &args2);
	pthread_detach(write3);

	sleep(1);
	pthread_mutex_destroy(&args2.m_cout);
	vfs.Close(args2.f);
}

void CopyFile(const char *file1, const char *file2)
{
	TestTask::File *f1, *f2;
	char buff[5001];
	size_t read = 5000;
	size_t wrote = 5000;

	f1 = vfs.Open(file1);
	if (!f1)
	{
		std::cerr << "Error: Open: Cannt open file!" << std::endl;
		return;
	}
	f2 = vfs.Create(file2);
	if (!f2)
	{
		std::cerr << "Error: Create: Cannt open file!" << std::endl;
		vfs.Close(f1);
		return;
	}
	while (read)
	{
		read = vfs.Read(f1, buff, 4096);
		buff[read] = 0;
		std::cout << "Read: " << read << " bytes" << std::endl << buff << std::endl << std::endl;
		if (read != 0)
		{
			wrote = vfs.Write(f2, buff, read);
			std::cout << "Write: " << wrote << " bytes" << std::endl;
			if (wrote > read)
				std::cerr << "!!! WRITE MORE THAN READ !!!" << std::endl;

			// i показывает сколько уже записано байт
			for (size_t i = wrote; wrote && i < read; i += wrote)
			{
				wrote = vfs.Write(f2, &buff[i], read - i);
				std::cout << "Write: " << wrote << " bytes" << std::endl;
				if (i > read)
					std::cerr << "!!! WRITE MORE THAN READ !!!" << std::endl;
			}
		}
	}
	vfs.Close(f1);
	vfs.Close(f2);
}

int main()
{
	// OCWOTest();
	// FoldersTest();
	ReadTest();
	// ThreadsTest();
	// Threads2WriteTest();
	CopyFile("new/file2", "new/f");

	return (0);
}
