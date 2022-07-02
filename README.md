# TestTaskVFS
Тестовое задание по реализации VFS.

## Реализация
Для хранения виртуальных файлов создаются 2 физических: VFS_Table и VFS_File. В этих файлах хранение данных осуществляется блоками фиксированных размеров. В первом записывается информация о файле/папке, он используется чтобы найти адрес запрашиваемого блока данных. Во втором хранится содержимое виртуальных файлов таким образом, что адреса блоков этого файла соответствуют адресам блоков в VFS_Table.

В TestTask namespace создан класс VFS для работы с виртуальными файлами, а также структура File для хранения и передачи данных об открытом файле классу.
Доступные методы:
```
File *Open( const char *name ); // Открыть файл в readonly режиме. Если нет такого файла или же он открыт во writeonly режиме - вернуть nullptr
File *Create( const char *name ); // Открыть или создать файл в writeonly режиме. Если нужно, то создать все нужные поддиректории, упомянутые в пути. Вернуть nullptr, если этот файл уже открыт в readonly режиме.
size_t Read( File *f, char *buff, size_t len ); // Прочитать данные из файла. Возвращаемое значение - сколько реально байт удалось прочитать
size_t Write( File *f, char *buff, size_t len ); // Записать данные в файл. Возвращаемое значение - сколько реально байт удалось записать
void Close( File *f ); // Закрыть файл
```

Стоит обратить внимание на то, что при открытии файла указатель ставится на начало этого файла.

## Test
Для тестирования написана небольшая программка, которая компилируется с помощью Makefile. Реализованы следующие правила:
```
all	- компиляция программы тестирования в исполняемый файл vfs_test
func	- компиляция программы тестирования функции
clean	- удаление VFS_Table и VFS_File в каталогах проекта
fclean 	- удаление vfs_test и func_test
re	- выполнение правил fclean и all
```
Для изменения набора тестов необходимо отредактировать main функцию в файле test.cpp (в конце файла).
