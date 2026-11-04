#define _CRT_SECURE_NO_WARNINGS

#include <cstdio>    // fopen, fclose, fgets, fread, fwrite, fseek, ftell
#include <cstdlib>   // malloc, free, qsort, bsearch, exit
#include <cstring>   // strcpy, strchr, strcmp

// Запись <номер_зачётной_книжки, фамилия_студента>
struct StudentRecord {
    unsigned kard;   // номер_зачётной_книжки
    char name[21];   // фамилия_студента
};

// Запись с позицией в двоичном файле
struct PositionRecord {
    long position;           // позиция_записи
    StudentRecord student;   // запись
};

// Сравнение записей по номеру зачётной книжки (для qsort и bsearch)
int CompareByKard(const void* first, const void* second) {
    const PositionRecord* firstRecord = (const PositionRecord*)first;
    const PositionRecord* secondRecord = (const PositionRecord*)second;

    if (firstRecord->student.kard < secondRecord->student.kard) {
        return -1;
    }
    if (firstRecord->student.kard > secondRecord->student.kard) {
        return 1;
    }
    return 0;
}

// Сравнение записей по фамилии (для qsort)
int CompareByName(const void* first, const void* second) {
    const PositionRecord* firstRecord = (const PositionRecord*)first;
    const PositionRecord* secondRecord = (const PositionRecord*)second;

    return strcmp(firstRecord->student.name, secondRecord->student.name);
}

// Класс 1: ТЕКСТОВЫЙ ФАЙЛ
class TextFile {
private:
    char fileName[256];   // имя_файла
    FILE* file;           // поток_чтения_файла
    char line[256];       // буфер_ввода_строки

public:
    // создание_файла_для_чтения_по_заданному_имени (конструктор)
    TextFile(const char* name) {
        strcpy(fileName, name);

        file = fopen(fileName, "r");
        if (file == NULL) {
            printf("Ошибка: не удалось открыть файл %s\n", fileName);
            exit(1);
        }
    }

    // аннулирование_файла (деструктор)
    ~TextFile() {
        if (file != NULL) {
            fclose(file);
            file = NULL;
        }
    }

    // чтение_строки
    char* ReadLine() {
        if (fgets(line, 256, file) == NULL) {
            return NULL;
        }

        // удаление '\n' в конце строки
        char* newLine = strchr(line, '\n');
        if (newLine != NULL) {
            *newLine = '\0';
        }
        return line;
    }
};

// Класс 2: ДВОИЧНЫЙ ФАЙЛ ЗАПИСЕЙ
class RecordFile {
private:
    char fileName[256];   // имя_файла
    FILE* file;           // поток_файла
    int recordCount;      // количество_записей

public:
    // создание_файла_по_текстовому_файлу (конструктор)
    RecordFile(const char* name, TextFile& textFile) {
        strcpy(fileName, name);
        recordCount = 0;

        // запись в двоичный файл
        file = fopen(fileName, "wb");
        if (file == NULL) {
            printf("Ошибка: не удалось создать файл %s\n", fileName);
            exit(1);
        }

        char* line = textFile.ReadLine();
        while (line != NULL) {
            // разбор строки «номер фамилия»
            StudentRecord student;
            if (sscanf(line, "%u %20s", &student.kard, student.name) == 2) {
                fwrite(&student, sizeof(StudentRecord), 1, file);
                recordCount++;
            }
            line = textFile.ReadLine();
        }
        fclose(file);

        // открытие для чтения
        file = fopen(fileName, "rb");
        if (file == NULL) {
            printf("Ошибка: не удалось открыть файл %s\n", fileName);
            exit(1);
        }
    }

    // аннулирование_файла (деструктор)
    ~RecordFile() {
        if (file != NULL) {
            fclose(file);
            file = NULL;
        }
    }

    // получение_копии_набора_записей_с_позициями
    PositionRecord* GetRecordSetCopy(int& count) {
        count = recordCount;
        PositionRecord* copy =
            (PositionRecord*)malloc(recordCount * sizeof(PositionRecord));

        // чтение записей с начала файла
        fseek(file, 0, SEEK_SET);
        for (int i = 0; i < recordCount; i++) {
            copy[i].position = ftell(file);
            fread(&copy[i].student, sizeof(StudentRecord), 1, file);
        }
        return copy;
    }

    // чтение_записи_по_позиции
    StudentRecord ReadRecord(long position) {
        StudentRecord student;
        fseek(file, position, SEEK_SET);
        fread(&student, sizeof(StudentRecord), 1, file);
        return student;
    }
};

// Класс 3: ИНДЕКСНЫЙ ФАЙЛ
class IndexFile {
private:
    char fileName[256];   // имя_файла
    FILE* file;           // поток_файла
    long* indices;        // упорядоченный_набор_индексов
    int indexCount;       // количество_индексов

public:
    // создание_файла_по_упорядоченному_набору_записей (конструктор)
    IndexFile(const char* name, PositionRecord* recordSet, int count) {
        strcpy(fileName, name);
        indexCount = count;
        indices = NULL;

        // запись позиций в индексный файл
        file = fopen(fileName, "wb");
        if (file == NULL) {
            printf("Ошибка: не удалось создать файл %s\n", fileName);
            exit(1);
        }
        for (int i = 0; i < indexCount; i++) {
            fwrite(&recordSet[i].position, sizeof(long), 1, file);
        }
        fclose(file);

        // открытие для чтения
        file = fopen(fileName, "rb");
        if (file == NULL) {
            printf("Ошибка: не удалось открыть файл %s\n", fileName);
            exit(1);
        }
    }

    // аннулирование_файла (деструктор)
    ~IndexFile() {
        if (file != NULL) {
            fclose(file);
            file = NULL;
        }
        if (indices != NULL) {
            free(indices);
            indices = NULL;
        }
    }

    // чтение_упорядоченного_набора_индексов
    void ReadIndices() {
        if (indices == NULL) {
            indices = (long*)malloc(indexCount * sizeof(long));
        }
        fseek(file, 0, SEEK_SET);
        fread(indices, sizeof(long), indexCount, file);
    }

    // поиск_фамилии_по_номеру_зачётной_книжки
    void FindNameByKard(RecordFile& recordFile, unsigned kard) {
        ReadIndices();

        // записи в порядке индексов
        PositionRecord* records =
            (PositionRecord*)malloc(indexCount * sizeof(PositionRecord));
        for (int i = 0; i < indexCount; i++) {
            records[i].position = indices[i];
            records[i].student = recordFile.ReadRecord(indices[i]);
        }

        // двоичный поиск по номеру
        PositionRecord key;
        key.position = 0;
        key.student.kard = kard;
        key.student.name[0] = '\0';
        PositionRecord* found = (PositionRecord*)bsearch(&key, records,
            indexCount, sizeof(PositionRecord), CompareByKard);

        if (found != NULL) {
            printf("Фамилия студента: %s\n", found->student.name);
        }
        else {
            puts("Студент с таким номером не найден.");
        }

        if (records != NULL) {
            free(records);
            records = NULL;
        }
    }

    // вывод_фамилий_в_порядке_индексов
    void PrintNames(RecordFile& recordFile) {
        ReadIndices();

        for (int i = 0; i < indexCount; i++) {
            StudentRecord student = recordFile.ReadRecord(indices[i]);
            printf("%d. %s\n", i + 1, student.name);
        }
    }
};

// Основная программа
int main() {
    // 1. Создать Текстовый_файл по заданному имени.
    TextFile textFile("students.txt");

    // 2. Создать Двоичный_файл_записей по Текстовому_файлу.
    RecordFile recordFile("students.dat", textFile);

    // 3. Получить копию набора записей с позициями.
    int count = 0;
    PositionRecord* recordSet = recordFile.GetRecordSetCopy(count);

    // 4. Упорядочить копию по номеру зачётной книжки функцией qsort
    // и создать Индексный_файл_по_номеру.
    qsort(recordSet, count, sizeof(PositionRecord), CompareByKard);
    IndexFile kardIndexFile("kard.idx", recordSet, count);

    // 5. Упорядочить копию по фамилии функцией qsort
    // и создать Индексный_файл_по_фамилии.
    qsort(recordSet, count, sizeof(PositionRecord), CompareByName);
    IndexFile nameIndexFile("name.idx", recordSet, count);

    // освобождение копии набора записей
    if (recordSet != NULL) {
        free(recordSet);
        recordSet = NULL;
    }

    // 6. Меню.
    char input[32];
    int choice = -1;
    while (choice != 0) {
        puts("\n1 - найти фамилию по номеру зачётной книжки");
        puts("2 - вывести фамилии по алфавиту");
        puts("0 - выход");
        printf("Выберите пункт: ");

        if (fgets(input, 32, stdin) == NULL) {
            break;
        }
        if (sscanf(input, "%d", &choice) != 1) {
            choice = -1;
        }

        if (choice == 1) {
            // поиск по номеру
            unsigned kard = 0;
            printf("Введите номер зачётной книжки: ");
            if (fgets(input, 32, stdin) == NULL) {
                break;
            }
            if (sscanf(input, "%u", &kard) == 1) {
                kardIndexFile.FindNameByKard(recordFile, kard);
            }
            else {
                puts("Ошибка: номер должен быть числом.");
            }
        }
        else if (choice == 2) {
            // вывод по алфавиту
            puts("Фамилии студентов по алфавиту:");
            nameIndexFile.PrintNames(recordFile);
        }
        else if (choice != 0) {
            puts("Нет такого пункта меню.");
        }
    }

    // 7. Завершить выполнение.
    return 0;
}