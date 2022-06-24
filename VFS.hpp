#ifndef VFS_HPP
#define VFS_HPP

#include "IVFS.hpp"
#include <string>
#include <fstream>
#include "stdlib.h"

namespace TestTask
{
struct Content
{
	unsigned char mod; // 0 - закрыт, 0x1 - открыт на чтение, 0x10 - открыт на запись, 0xFF - папка
	unsigned int filled;
	uint32_t next;
	uint32_t addr_extra; // Для папок - адрес следующиего файла, для файлов адрес начального блока
};

struct File
{
	Content content;
	char name[24];
	size_t p;
	uint32_t addr;
};

class VFS : public IVFS
{
public:
	VFS();
	~VFS();

	File *Open( const char *name ); // Открыть файл в readonly режиме. Если нет такого файла или же он открыт во writeonly режиме - вернуть nullptr
	File *Create( const char *name ); // Открыть или создать файл в writeonly режиме. Если нужно, то создать все нужные поддиректории, упомянутые в пути. Вернуть nullptr, если этот файл уже открыт в readonly режиме.
	size_t Read( File *f, char *buff, size_t len ); // Прочитать данные из файла. Возвращаемое значение - сколько реально байт удалось прочитать
	size_t Write( File *f, char *buff, size_t len ); // Записать данные в файл. Возвращаемое значение - сколько реально байт удалось записать
	void Close( File *f ); // Закрыть файл

private:
	std::fstream _file;
	std::fstream _ftable;
	char zstr[4096];

	// void _SetMod(File *file); // Обновление поля mod в VFS_Table
	// void _SetFilled( File *f ); // Обновление всего контента (кроме имени файла) в VFS_Table
	void _UpdateBlock( File &f ); // Запись изменений существующего блока в VFS_Table
	void _ReadFileInfo( File &f, size_t p ); // Чтение изменений блока в существующий объект File
	File *_FindFile( const char *name ); // Поиск стартового блока файла по имени
	File *_TakeFileInfo( uint32_t addr ); // Возврат файла по адресу
	void _NewBlock( File **f, const char *name ); // Выделение пустого блока
	void _MoveBlock( File *f, bool create_mod ); // Переход к следующему блоку
	uint32_t _TakeBlocksCount(); // Получение количества блоков
};

}

#endif