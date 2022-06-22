#include "VFS.hpp"

#define FTB_SIZE 32 // При изменении важно проверить методы меняющие данные
#define FB_SIZE 4096

#define READM 0x1
#define WRITEM 0x10
#define FOLDERM 0xFF

namespace TestTask
{
VFS::VFS() : _file("VFS_File", std::ios::binary | std::ios::in | std::ios::out), _ftable("VFS_Table", std::ios::binary | std::ios::in | std::ios::out)
{
	if (_file.is_open() && !_ftable.is_open())
	{
		_file.close();
		throw std::logic_error("Cannt open VSF_File");
	} else if (!_file.is_open() && _ftable.is_open())
	{
		_ftable.close();
		throw std::logic_error("Cannt open VSF_Table");
	} else if (!_file.is_open() || !_ftable.is_open())
	{
		throw std::logic_error("Cannt open VFS_File");
	}
}

VFS::~VFS()
{
	if (_file.is_open())
		_file.close();
	if (_ftable.is_open())
		_ftable.close();
}

File *VFS::_FindFile( const char *name )
{
	File *res = nullptr;
	uint32_t b_addr = 0;
	char buf[5];
	uint32_t bcnt;

	// Получение количества блоков
	_ftable.seekg(0);
	_ftable.read(buf, 4);
	buf[5] = 0;
	bcnt = buf[0] << 24 | buf[1] << 16 | buf[2] << 8 | buf[3];

	for (uint32_t i = 0; i < bcnt || res == nullptr; ++i)
	{
		res = _TakeFileInfo(b_addr);
		if (res->name.compare(name))
			res = nullptr;
	}
	return (res);
}

File *VFS::_TakeFileInfo( uint32_t addr )
{
	char buf[FTB_SIZE];
	File *res;
	char *end;
	uint32_t bcnt;

	// Получение количества блоков
	_ftable.seekg(0);
	_ftable.read(buf, 4);
	buf[5] = 0;
	bcnt = buf[0] << 24 | buf[1] << 16 | buf[2] << 8 | buf[3];
	if (addr > bcnt)
		return (nullptr);

	// Поиск и чтение блока
	_ftable.seekg(addr * FTB_SIZE);
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



void VFS::_SetMod( File *file )
{
	_ftable.seekp(file->addr * FTB_SIZE + 28);
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

File *VFS::Create( const char *name ) // Открыть или создать файл в writeonly режиме. Если нужно, то создать все нужные поддиректории, упомянутые в пути. Вернуть nullptr, если этот файл уже открыт в readonly режиме.
{
	return (nullptr);
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