#include "VFS.hpp"
#include <iostream>
#include <unistd.h>

#define FTB_SIZE 32 // При изменении важно проверить методы меняющие данные
#define FTB_HEAD 4
#define FB_SIZE 4096

#define READM 0x1
#define WRITEM 0x10
#define FOLDERM 0xFF
#define CONTENTM 0xF0

namespace TestTask
{
VFS::VFS()
{
	_file.open("VFS_File", std::ios::app | std::ios::in | std::ios::binary);
	_ftable.open("VFS_Table", std::ios::app | std::ios::in | std::ios::binary);
	if (_file.is_open() && !_ftable.is_open())
	{
		_file.close();
		throw std::logic_error("Cannt open VFS_Table");
	} else if (!_file.is_open() && _ftable.is_open())
	{
		_ftable.close();
		throw std::logic_error("Cannt open VFS_File");
	} else if (!_file.is_open() || !_ftable.is_open())
	{
		throw std::logic_error("Cannt open VFS_File");
	}
	// Проверка на наличие хедера файла и его добавление, если пусто
	_ftable.seekg(0);
	_ftable.read(zstr, 4);
	zstr[4] = 0;
	std::cout << *((unsigned int *)zstr) << std::endl;
	bzero(zstr, 4096);
	if (_ftable.gcount() != 4)
	{
		std::cout << "Here!" << std::endl;
		_ftable.clear();
		_ftable.write(zstr, 4);
	}
}

VFS::~VFS()
{
	if (_ftable.is_open())
		_ftable.close();
	if (_file.is_open())
		_file.close();
}

uint32_t VFS::_TakeBlocksCount() // Получение количества блоков
{
	char buf[5];

	_ftable.seekg(0);
	_ftable.read(buf, 4);
	buf[4] = 0;
	return (buf[0] << 24 | buf[1] << 16 | buf[2] << 8 | buf[3]);
}

File *VFS::_FindFile( const char *name ) // Поиск файла по имени
{
	File *res = nullptr;
	uint32_t bcnt = _TakeBlocksCount();

	for (uint32_t i = 0; i < bcnt || res == nullptr; ++i)
	{
		res = _TakeFileInfo(i);
		if (res->name.compare(name))
			res = nullptr;
	}
	return (res);
}

File *VFS::_TakeFileInfo( uint32_t addr ) // Возврат файла по адресу
{
	char buf[FTB_SIZE];
	File *res;
	uint32_t bcnt = _TakeBlocksCount();

	if (addr > bcnt)
		return (nullptr);

	// Поиск и чтение блока
	_ftable.seekg(addr * FTB_SIZE + 4);
	_ftable.read(buf, FTB_SIZE);

	// Заполнение File
	res = new File;
	res->name = buf;
	res->name.erase(24, 9);
	res->next = buf[23] << 24 | buf[24] << 16 | buf[25] << 8 | buf[26];
	res->mod = buf[27];
	res->addr_extra = buf[28] << 24 | buf[29] << 16 | buf[30] << 8 | buf[31];
	res->addr = addr;
	res->p = 0;
	return (res);
}



void VFS::_SetMod( File *file ) // Обновление поля mod в VFS_Table
{
	_ftable.seekp(file->addr * FTB_SIZE + 28 + 4);
	_ftable.write(&file->mod, 1);
}

File *VFS::Open( const char *name ) // Открыть файл в readonly режиме. Если нет такого файла или же он открыт во writeonly режиме - вернуть nullptr
{
	File *res = _FindFile(name);

	if (res->mod != READM && res->mod != 0)
	{
		delete (res);
		return (nullptr);
	}
	if (res->mod == READM)
		return (res);
	res->mod = READM;
	_SetMod(res);
	return (res);
}

File *VFS::_NewBlock( File* prevf, const char *name ) // Выделение пустого блока
{
	File *res;
	uint32_t lblock = _TakeBlocksCount();

	// Обновление информации о блоке в res
	if (!prevf)
	{
		res = new File();
		res->name = name;
		res->next = 0;
		res->mod = 0;
		res->addr_extra = lblock;
		res->p = 0;
	} else
	{
		res = prevf;
		res->mod = CONTENTM;
		res->p = 0;
	}
	res->addr = lblock;

	// Запись из res в VFS_Table
	char addr[17];
	_ftable.clear();
	_ftable.seekp(res->addr * FTB_SIZE + 4, std::ios::beg);
	_ftable.write(res->name.c_str(), 23);
	_ftable.write(zstr, 4);
	_ftable.write(&res->mod, 1);
	_ftable.write(reinterpret_cast<char *>(&res->addr_extra), 4);
	_ftable.clear();
	_ftable.flush();
	_ftable.seekp(0);
	++lblock;
	_ftable.write(reinterpret_cast<char *>(&lblock), 4);

	std::cout << lblock << std::endl;
	// Запись NULL в VFS_File
	_file.clear();
	_file.seekp(res->addr * FB_SIZE, std::ios::beg);
	_file.write(zstr, 4096);
	return (res);
}

File *VFS::Create( const char *name ) // Открыть или создать файл в writeonly режиме. Если нужно, то создать все нужные поддиректории, упомянутые в пути. Вернуть nullptr, если этот файл уже открыт в readonly режиме.
{
	return (_NewBlock(0, name));
}

size_t VFS::Read( File *f, char *buff, size_t len ) // Прочитать данные из файла. Возвращаемое значение - сколько реально байт удалось прочитать
{
	return (0);
}

size_t VFS::Write( File *f, char *buff, size_t len ) // Записать данные в файл. Возвращаемое значение - сколько реально байт удалось записать
{
	return (0);
}

void VFS::Close( File *f ) // Закрыть файл
{
	f->mod = 0;
	_SetMod(f);
	delete (f);
}

}