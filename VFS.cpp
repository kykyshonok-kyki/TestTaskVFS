#include "VFS.hpp"
#include <iostream>
#include <unistd.h>

#define FTB_SIZE 39
#define FB_SIZE 4096
#define NAME_SIZE 23

#define READM 0x1
#define WRITEM 0x10
#define FOLDERM 0xFF
#define CONTENTM 0xF0
#define SEARCHERRM 0xF

namespace TestTask
{
Content::Content() {}

File::File()
{
	pthread_mutex_init(&_m_struct, NULL);
}

File::~File()
{
	pthread_mutex_destroy(&_m_struct);
}

VFS::VFS()
{
	// Попытка открыть файлы, если не получилось, создать, закрыть, открыть в нужном режиме
	_file.open("VFS_File", std::ios::out | std::ios::in | std::ios::binary);
	_ftable.open("VFS_Table", std::ios::out | std::ios::in | std::ios::binary);
	if (!_file.is_open() || !_ftable.is_open())
	{
		_file.open("VFS_File", std::ios::app);
		_file.close();
		_file.open("VFS_File", std::ios::out | std::ios::in | std::ios::binary);
		_ftable.open("VFS_Table", std::ios::app);
		_ftable.close();
		_ftable.open("VFS_Table", std::ios::out | std::ios::in | std::ios::binary);
		File *f = nullptr;
		_NewBlock(&f, "");
		delete (f);
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
	pthread_mutex_init(&_m_file, NULL);
	pthread_mutex_init(&_m_table, NULL);
	bzero(zstr, 4096);
}

VFS::~VFS()
{
	if (_ftable.is_open())
		_ftable.close();
	if (_file.is_open())
		_file.close();
	pthread_mutex_destroy(&_m_file);
	pthread_mutex_destroy(&_m_table);
}

std::vector<std::string> VFS::_TrimCStr( const char *str, char delim )
{
	std::vector<std::string> path;
	const char *ldelim; // Позиция после последнего разделителя
	char file[24];

	if (*str == '/')
		++str;
	ldelim = str;
	while (*str)
	{
		if (*str == delim)
		{
			strlcpy(file, ldelim, str - ldelim + 1);
			if (str - ldelim)
				path.push_back(file);
			ldelim = str + 1;
		}
		++str;
	}
	if (ldelim != str)
	{
		strlcpy(file, ldelim, str - ldelim + 1);
		path.push_back(file);
	}
	return (path);
}

uint32_t VFS::_TakeBlocksCount() // Получение количества блоков
{
	std::streampos end;

	pthread_mutex_lock(&_m_table);
	_ftable.clear();
	_ftable.seekg(0, std::ios::end);
	end = _ftable.tellg();
	pthread_mutex_unlock(&_m_table);
	return (end / FTB_SIZE);
}

File *VFS::_FindFile( const char *name ) // Поиск стартового блока файла по имени
{
	std::vector<std::string> path = _TrimCStr(name, '/');

	if (path.size() == 0)
		return (nullptr);

	uint32_t bcnt = _TakeBlocksCount();
	std::vector<std::string>::iterator it = path.begin();
	std::vector<std::string>::iterator end = path.end();
	File *res = new File;

	res->addr = 1;
	while (it != end)
	{
		// Прохождение по записям пока не будет найден нужный файл или подходящие записи не закончатся
		while (res->addr != 0 && res->addr < bcnt)
		{
			_ReadFileInfo(*res, 0);
			if (res->content.mod != CONTENTM && strcmp(res->name, it->c_str()) == 0) // Файл подошел
			{
				if (it + 1 == end) // Найден последний файл пути
					return (res);
				else if (res->content.mod != FOLDERM) // Не последний файл пути, а текущий найденный оказался не папкой
				{
					delete (res);
					return (nullptr);
				}
				res->addr = res->content.next; // Это папка -> вход внутрь
				break;
			} else
				res->addr = res->content.addr_extra; 
		}
		if (res->addr == 0)
			break;
		++it;
	}
	delete (res);
	return (nullptr);
}

File *VFS::_FindLastFolder( std::vector<std::string> &path,
							std::vector<std::string>::iterator &it ) // Поиск последнего файла существующей папки пути
{
	if (path.size() == 0)
		return (nullptr);

	uint32_t bcnt = _TakeBlocksCount();
	std::vector<std::string>::iterator end = path.end();
	File *res = new File;

	res->addr = 1;
	if (bcnt != 1)
	{
		while (it != end)
		{
			// Прохождение по записям пока не будет найден нужный файл или подходящие записи не закончатся
			while (res->addr != 0 && res->addr < bcnt)
			{
				_ReadFileInfo(*res, 0);
				if (res->content.mod != CONTENTM && strcmp(res->name, it->c_str()) == 0) // Файл подошел
				{
					if (it + 1 == end) // Найден последний файл пути
						return (res);
					else if (res->content.mod != FOLDERM) // Не последний файл пути, а текущий найденный оказался не папкой
					{
						delete (res);
						return (nullptr);
					}
					res->addr = res->content.next; // Это папка -> вход внутрь
					break;
				} else
				{
					if (res->content.addr_extra == 0) // Последний файл папки и он не папка
						return (res);
					res->addr = res->content.addr_extra;
				}
			}
			if (res->addr == 0)
				break;
			++it;
		}
		--it;
	}
	delete (res);
	res = 0;
	_NewBlock(&res, it->c_str());
	res->content.mod = SEARCHERRM;
	++it;
	return (res);
}

void VFS::_ReadFileInfo( File &f, size_t p ) // Чтение изменений блока в существующий объект File
{
	pthread_mutex_lock(&_m_table);
	_ftable.clear();
	_ftable.seekg(f.addr * (NAME_SIZE + sizeof(f.content)));
	_ftable.read(f.name, NAME_SIZE);
	_ftable.read(reinterpret_cast<char *>(&f.content), sizeof(f.content));
	pthread_mutex_unlock(&_m_table);
	f.name[NAME_SIZE] = 0;
	f.p = p;
}

File *VFS::_TakeFileInfo( uint32_t addr ) // Возврат файла по адресу
{
	File *res;
	uint32_t bcnt = _TakeBlocksCount();
	res = new File;

	if (addr >= bcnt)
		return (nullptr);

	res->addr = addr;
	_ReadFileInfo(*res, 0);

	return (res);
}

void VFS::_UpdateBlock( File &f ) // Запись изменений существующего блока в VFS_Table
{
	pthread_mutex_lock(&_m_table);
	_ftable.clear();
	_ftable.seekp(f.addr * (NAME_SIZE + sizeof(f.content)));
	_ftable.write(f.name, NAME_SIZE);
	_ftable.write(reinterpret_cast<char *>(&f.content), sizeof(f.content));
	pthread_mutex_unlock(&_m_table);
}

void VFS::_MoveBlock( File *f, bool create_mod ) // Переход к следующему блоку если такой есть. Если нет, он создается только в create_mod
{
	uint32_t lblock = _TakeBlocksCount();

	if (!f || f->addr > lblock)
		return;

	if (f->content.next) // Если есть сл. блок, обновление содержимого структуры на него
	{
		f->addr = f->content.next;
		_ReadFileInfo(*f, 0);
	}
	else if (create_mod) // Блока нет. Создается новый, если create_mod = true
		_NewBlock(&f, f->name);
}

void VFS::_NewBlock( File **f, const char *name ) // Выделение пустого блока
{
	uint32_t lblock = _TakeBlocksCount();

	// Обновление информации о блоке
	if (!*f)
	{
		*f = new File();
		bzero((*f)->name, NAME_SIZE + 1);
		strcpy((*f)->name, name);
		(*f)->content.next = 0;
		(*f)->content.mod = 0;
		(*f)->content.addr_extra = 0;
		(*f)->content.filled = 0;
		(*f)->p = 0;
	} else
	{
		(*f)->content.next = lblock;
		_UpdateBlock(**f);
		(*f)->content.next = 0;
		if ((*f)->content.mod != CONTENTM)
		{
			(*f)->content.addr_extra = (*f)->addr;
			(*f)->content.mod = CONTENTM;
		}
		(*f)->content.filled = 0;
		(*f)->p = 0;
	}
	(*f)->addr = lblock;
	_UpdateBlock(**f);

	// Запись NULL в VFS_File
	pthread_mutex_lock(&_m_file);
	_file.clear();
	_file.seekp((*f)->addr * FB_SIZE);
	_file.write(zstr, 4096);
	pthread_mutex_unlock(&_m_file);
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
	_UpdateBlock(*res);
	return (res);
}

File *VFS::Create( const char *name ) // Открыть или создать файл в writeonly режиме. Если нужно, то создать все нужные поддиректории, упомянутые в пути. Вернуть nullptr, если этот файл уже открыт в readonly режиме.
{
	if (strlen(name) > NAME_SIZE)
		return (nullptr);

	File *f = nullptr;
	std::vector<std::string> path = _TrimCStr(name, '/');

	if (path.size() == 0)
		return (nullptr);

	std::vector<std::string>::iterator it = path.begin();
	std::vector<std::string>::iterator end = path.end();

	f = _FindLastFolder(path, it);
	if (f && it + 1 == end && !strcmp(f->name, it->c_str())) // Файл существует
	{
		if (f->content.mod != 0)
		{
			delete (f);
			return (nullptr);
		}
		else
		{
			f->content.mod = WRITEM;
			_UpdateBlock(*f);
			return (f);
		}
	} else // Файл не существует
	{
		File *nf = nullptr;

		if (!f) // В пути попался файл
			return (nullptr);
		// Создается запись как папка и редактируется addr_extra предыдущего файла
		while (it != end)
		{
			_NewBlock(&nf, it->c_str());
			if (f->content.mod == SEARCHERRM)
			{
				f->content.mod = FOLDERM;
				f->content.next = nf->addr;
			}
			else
				f->content.addr_extra = nf->addr;
			_UpdateBlock(*f);
			delete (f);
			nf->content.mod = SEARCHERRM;
			f = nf;
			nf = nullptr;
			++it;
		}
		// Последний файл пути - файл, проставляется режим открыт для записи
		f->content.mod = WRITEM;
		_UpdateBlock(*f);
		return (f);
	}
}

size_t VFS::Read( File *f, char *buff, size_t len ) // Прочитать данные из файла. Возвращаемое значение - сколько реально байт удалось прочитать
{
	pthread_mutex_lock(&f->_m_struct);
	size_t read = 0;

	if (!f->addr)
	{
		pthread_mutex_unlock(&f->_m_struct);
		return (0);
	}
	if (f->content.mod == CONTENTM) // Если текущий блок - продолжение, режим файла смотрится в главном блоке
	{
		File *nf = new File;
		nf->addr = f->content.addr_extra;
		_ReadFileInfo(*nf, 0);
		if (nf->content.mod != READM)
		{
			pthread_mutex_unlock(&f->_m_struct);
			delete (nf);
			return (0);
		}
		delete (nf);
	}
	else if (f->content.mod != READM)
	{
		pthread_mutex_unlock(&f->_m_struct);
		return (0);
	}

	if (len > FB_SIZE - f->p)
		len = FB_SIZE - f->p;
	if (len > f->content.filled - f->p)
		len = f->content.filled - f->p;

	// Чтение данных
	pthread_mutex_lock(&_m_file);
	_file.clear();
	_file.seekg(f->addr * FB_SIZE + f->p);
	_file.read(buff, len);
	read = _file.gcount();
	pthread_mutex_unlock(&_m_file);

	// Установка указателя и переход к новому блоку если это необходимо
	f->p += read;
	if (f->p == FB_SIZE)
		_MoveBlock(f, false);
	pthread_mutex_unlock(&f->_m_struct);
	return (read);
}

size_t VFS::Write( File *f, char *buff, size_t len ) // Записать данные в файл. Возвращаемое значение - сколько реально байт удалось записать
{
	pthread_mutex_lock(&f->_m_struct);
	std::streampos start;
	size_t res;

	if (f->content.mod == CONTENTM) // Если текущий блок - продолжение, режим файла смотрится в главном блоке
	{
		File *nf = new File;
		nf->addr = f->content.addr_extra;
		_ReadFileInfo(*nf, 0);
		if (nf->content.mod != WRITEM)
		{
			pthread_mutex_unlock(&f->_m_struct);
			delete (nf);
			return (0);
		}
		delete (nf);
	}
	else if (f->content.mod != WRITEM)
	{
		pthread_mutex_unlock(&f->_m_struct);
		return (0);
	}

	if (len > FB_SIZE - f->p)
		len = FB_SIZE - f->p;

	// Запись данных
	pthread_mutex_lock(&_m_file);
	_file.clear();
	_file.seekp(f->addr * FB_SIZE + f->p);
	start = _file.tellp();
	_file.write(buff, len);
	res = _file.tellp() - start;
	pthread_mutex_unlock(&_m_file);

	// Установка указателей и переход к новому блоку если это необходимо
	f->content.filled += res;
	_UpdateBlock(*f);
	f->p += res;
	if (f->p == FB_SIZE)
		_MoveBlock(f, true);
	pthread_mutex_unlock(&f->_m_struct);
	return (res);
}

void VFS::Close( File *f ) // Закрыть файл
{
	pthread_mutex_lock(&f->_m_struct);
	if (f->content.mod == CONTENTM)
	{
		// Контентный блок, поэтому адрес устанавливается на родителя и обновляется информация
		f->addr = f->content.addr_extra;
		_ReadFileInfo(*f, 0);
	}
	f->content.mod = 0;
	_UpdateBlock(*f);
	pthread_mutex_unlock(&f->_m_struct);
	delete (f);
}

}
