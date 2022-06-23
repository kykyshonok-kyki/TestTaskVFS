#include "VFS.hpp"
#include <iostream>
#include <unistd.h>

#define FTB_SIZE 32 // При изменении важно проверить методы меняющие данные
#define FB_SIZE 4096
#define NAME_SIZE 23

#define READM 0x1
#define WRITEM 0x10
#define FOLDERM 0xFF
#define CONTENTM 0xF0

namespace TestTask
{
VFS::VFS()
{
	// Попытка открыть файлы, если не получилось, создать, закрыть, открыть в нужном режиме
	_file.open("VFS_File", std::ios::out | std::ios::in | std::ios::binary);
	if (!_file.is_open())
	{
		_file.open("VFS_File", std::ios::app);
		_file.close();
		_file.open("VFS_File", std::ios::out | std::ios::in | std::ios::binary);
	}
	_ftable.open("VFS_Table", std::ios::out | std::ios::in | std::ios::binary);
	if (!_ftable.is_open())
	{
		_ftable.open("VFS_Table", std::ios::app);
		_ftable.close();
		_ftable.open("VFS_Table", std::ios::out | std::ios::in | std::ios::binary);
	}

	// Если файлы не открыты, проброс исключения
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
	bzero(zstr, 4096);
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
	std::streampos end;

	_ftable.clear();
	_ftable.seekg(0, std::ios::end);
	end = _ftable.tellg();
	return (end / FTB_SIZE);
}

File *VFS::_FindFile( const char *name ) // Поиск файла по имени
{
	File *res = nullptr;
	uint32_t bcnt = _TakeBlocksCount();

	for (uint32_t i = 0; i < bcnt && res == nullptr; ++i)
	{
		// Прохождение по всем записям пока не будет найден нужный файл или записи не закончатся
		res = _TakeFileInfo(i);
		if (!res)
			return (nullptr);
		if (strcmp(res->name, name))
			res = nullptr;
	}
	return (res);
}

File *VFS::_TakeFileInfo( uint32_t addr ) // Возврат файла по адресу
{
	File *res;
	uint32_t bcnt = _TakeBlocksCount();
	res = new File;

	if (addr >= bcnt)
		return (nullptr);

	// Поиск и чтение блока
	_ftable.clear();
	_ftable.seekp(addr * FTB_SIZE);
	_ftable.read(res->name, NAME_SIZE);
	res->name[NAME_SIZE] = 0;
	_ftable.read(reinterpret_cast<char *>(&res->content), sizeof(res->content));
	res->addr = addr;
	res->p = 0;

	return (res);
}

void VFS::_SetMod( File *file ) // Обновление поля mod в VFS_Table
{
	_ftable.clear();
	_ftable.seekp(file->addr * FTB_SIZE + NAME_SIZE + 1);
	_ftable.write(&file->content.mod, 1);
}

File *VFS::_NewBlock( File* prevf, const char *name ) // Выделение пустого блока
{
	File *res;
	uint32_t lblock = _TakeBlocksCount();

	// Обновление информации о блоке в res
	if (!prevf)
	{
		res = new File();
		strcpy(res->name, name);
		res->content.next = 0;
		res->content.mod = 0;
		res->content.addr_extra = lblock;
		res->p = 0;
	} else
	{
		res = prevf;
		res->content.mod = CONTENTM;
		res->p = 0;
	}
	res->addr = lblock;

	// Запись из res в VFS_Table
	_ftable.write(res->name, NAME_SIZE);
	_ftable.write(reinterpret_cast<char *>(&res->content), sizeof(res->content));

	// Запись NULL в VFS_File
	_file.clear();
	_file.seekp(res->addr * FB_SIZE);
	_file.write(zstr, 4096);
	return (res);
}

File *VFS::Open( const char *name ) // Открыть файл в readonly режиме. Если нет такого файла или же он открыт во writeonly режиме - вернуть nullptr
{
	if (strlen(name) > NAME_SIZE)
		return (nullptr);
	File *res = _FindFile(name);

	if (!res)
		return (nullptr);

	if (res->content.mod != READM && res->content.mod != 0)
	{
		delete (res);
		return (nullptr);
	}
	if (res->content.mod == READM)
		return (res);

	// Файл не открыт и существует, т.к. прошлыми if-ами отсеились прочие условия
	res->content.mod = READM;
	_SetMod(res);
	return (res);
}

File *VFS::Create( const char *name ) // Открыть или создать файл в writeonly режиме. Если нужно, то создать все нужные поддиректории, упомянутые в пути. Вернуть nullptr, если этот файл уже открыт в readonly режиме.
{
	if (strlen(name) > NAME_SIZE)
		return (nullptr);

	File *f;

	f = _FindFile(name);
	if (f)
	{
		// Файл существует
		if (f->content.mod != WRITEM && f->content.mod != 0)
			return (nullptr);
		else if (f->content.mod == WRITEM)
			return (f);
		else
		{
			// Смена режима
			return (f);
		}
	} else // Файл не существует
		return (_NewBlock(nullptr, name));
}

size_t VFS::Read( File *f, char *buff, size_t len ) // Прочитать данные из файла. Возвращаемое значение - сколько реально байт удалось прочитать
{
	// if (len > FB_SIZE - f->p)
	// 	len = FB_SIZE - f->p;
	// _file.clear();
	// _file.seekg(f->addr + f->p);
	// _file.read(buff, len);
	// if (f->p == FB_SIZE)
	// 	return (_file.gcount());
	return (0);
}

size_t VFS::Write( File *f, char *buff, size_t len ) // Записать данные в файл. Возвращаемое значение - сколько реально байт удалось записать
{
	(void)f;
	(void)buff;
	(void)len;
	return (0);
}

void VFS::Close( File *f ) // Закрыть файл
{
	f->content.mod = 0;
	_SetMod(f);
	delete (f);
}

}