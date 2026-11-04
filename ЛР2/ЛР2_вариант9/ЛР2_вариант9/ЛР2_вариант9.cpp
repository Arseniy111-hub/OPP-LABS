#define _CRT_SECURE_NO_WARNINGS

#include <cstdio>    // fopen, fclose, fgets, printf, puts
#include <cstdlib>   // malloc, realloc, free, qsort
#include <cstring>   // strlen, strchr
#include <search.h>  // _lfind

// =========================================================================
// Запись <длина_строки, количество_строк>
// =========================================================================
struct LineRecord {
    int length;   // длина_строки
    int count;    // количество_строк
};

// =========================================================================
// Сравнение записей по длине строки (для qsort и _lfind)
// =========================================================================
int CompareByLength(const void* first, const void* second) {
    const LineRecord* firstRecord = (const LineRecord*)first;
    const LineRecord* secondRecord = (const LineRecord*)second;

    return firstRecord->length - secondRecord->length;
}

// =========================================================================
// Класс 1: ТЕКСТОВЫЙ ФАЙЛ
// =========================================================================
class TextFile {
private:
    char* fileName;   // имя_файла
    FILE* file;       // поток_чтения_файла
    char* line;       // буфер_ввода_строки

public:
    // открытие_файла_для_чтения_по_имени_введённому_с_клавиатуры
    // (конструктор)
    TextFile() {
        fileName = (char*)malloc(256 * sizeof(char));
        line = (char*)malloc(257 * sizeof(char));

        // ввод имени файла
        fileName[0] = '\0';
        fgets(fileName, 256, stdin);

        // удаление '\n' в конце имени
        char* newLine = strchr(fileName, '\n');
        if (newLine != NULL) {
            *newLine = '\0';
        }

        // открытие файла для чтения
        file = fopen(fileName, "r");
    }

    // закрытие_и_аннулирование_файла (деструктор)
    ~TextFile() {
        if (file != NULL) {
            fclose(file);
            file = NULL;
        }
        if (line != NULL) {
            free(line);
            line = NULL;
        }
        if (fileName != NULL) {
            free(fileName);
            fileName = NULL;
        }
    }

    // проверка_успешности_открытия_файла
    bool IsOpen() const {
        return file != NULL;
    }

    // чтение_очередной_строки
    char* ReadLine() {
        if (fgets(line, 257, file) == NULL) {
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

// =========================================================================
// Класс 2: МАССИВ ЗАПИСЕЙ
// =========================================================================
class RecordArray {
private:
    LineRecord* records;   // набор_записей
    int recordCount;       // количество_записей

public:
    // создание_массива_записей_по_текстовому_файлу (конструктор)
    RecordArray(TextFile& textFile) {
        records = NULL;
        recordCount = 0;

        char* line = textFile.ReadLine();
        while (line != NULL) {
            // запись для прочитанной строки
            LineRecord newRecord;
            newRecord.length = (int)strlen(line);
            newRecord.count = 1;

            // поиск записи с такой же длиной
            unsigned int number = (unsigned int)recordCount;
            LineRecord* found = (LineRecord*)_lfind(&newRecord, records,
                &number, sizeof(LineRecord), CompareByLength);

            if (found != NULL) {
                // длина уже есть: увеличение количества строк
                found->count++;
            }
            else {
                // новая длина: добавление записи в конец массива
                records = (LineRecord*)realloc(records,
                    (recordCount + 1) * sizeof(LineRecord));
                records[recordCount] = newRecord;
                recordCount++;
            }

            line = textFile.ReadLine();
        }
    }

    // создание_массива_записей_по_набору_записей (конструктор)
    RecordArray(LineRecord* recordSet, int count) {
        recordCount = count;
        records = (LineRecord*)malloc(recordCount * sizeof(LineRecord));
        for (int i = 0; i < recordCount; i++) {
            records[i] = recordSet[i];
        }
    }

    // аннулирование_массива_записей (деструктор)
    ~RecordArray() {
        if (records != NULL) {
            free(records);
            records = NULL;
        }
    }

    // получение_копии_набора_записей
    LineRecord* GetRecordSetCopy(int& count) const {
        count = recordCount;
        LineRecord* copy =
            (LineRecord*)malloc(recordCount * sizeof(LineRecord));
        for (int i = 0; i < recordCount; i++) {
            copy[i] = records[i];
        }
        return copy;
    }

    // вывод_набора_записей_на_экран
    void Display() const {
        puts("Длина строки | Количество строк");
        for (int i = 0; i < recordCount; i++) {
            printf("%12d | %d\n", records[i].length, records[i].count);
        }
    }
};

// =========================================================================
// Основная программа
// =========================================================================
int main() {
    puts("Введите имя текстового файла:");

    // 1. Создать Исходный_текстовый_файл конструктором
    // с вводом имени с клавиатуры.
    TextFile sourceFile;

    // 2. Если Исходный_текстовый_файл не открыт, вывести сообщение
    // об ошибке и завершить выполнение.
    if (!sourceFile.IsOpen()) {
        puts("Ошибка: не удалось открыть файл.");
        return 1;
    }

    // 3. Создать Исходный_массив_записей по Исходному_текстовому_файлу.
    RecordArray sourceArray(sourceFile);

    // 4. Получить копию набора записей Исходного_массива_записей.
    int count = 0;
    LineRecord* recordSet = sourceArray.GetRecordSetCopy(count);

    // 5. Упорядочить копию набора записей по возрастанию длин строк
    // функцией qsort.
    qsort(recordSet, count, sizeof(LineRecord), CompareByLength);

    // 6. Создать Упорядоченный_массив_записей
    // по упорядоченному набору записей.
    RecordArray sortedArray(recordSet, count);

    // 7. Вывести набор записей Исходного_массива_записей
    // и Упорядоченного_массива_записей.
    puts("\nИсходный массив записей:");
    sourceArray.Display();

    puts("\nУпорядоченный массив записей:");
    sortedArray.Display();

    // освобождение копии набора записей
    if (recordSet != NULL) {
        free(recordSet);
        recordSet = NULL;
    }

    // 8. Завершить выполнение.
    return 0;
}
