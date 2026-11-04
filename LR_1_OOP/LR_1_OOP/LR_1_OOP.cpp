#define _CRT_SECURE_NO_WARNINGS // разрешает strcpy, strcat, strtok в Visual Studio

#include <cstdio>    // getchar, puts
#include <cstdlib>   // malloc, free
#include <cstring>   // strlen, strcpy, strcat, strtok, strcmp



// Класс 1: ЗАКОДИРОВАННОЕ ПРЕДЛОЖЕНИЕ

class CodedSentence {
private:
    char* string;     // строка_закодированных_символов
    char** words;     // упорядоченный_набор_закодированных_слов
    int wordCount;

public:
    // создание_предложения_вводом_с_клавиатуры (конструктор)
    CodedSentence() {
        string = (char*)malloc(81 * sizeof(char));   // 80 символов + '\0'
        int i = 0;
        int ch;

        // посимвольный ввод через getchar()
        while ((ch = getchar()) != '\n' && ch != EOF) {
            if (i < 80) {
                string[i] = (char)ch;
                i++;
            }
        }
        string[i] = '\0';

        // strtok портит строку (ставит '\0' вместо пробелов),
        // поэтому разбиваем не саму строку, а её копию
        char* copy = (char*)malloc((strlen(string) + 1) * sizeof(char));

        // первый проход: считаем слова
        strcpy(copy, string);
        wordCount = 0;
        char* word = strtok(copy, " ");
        while (word != NULL) {
            wordCount++;
            word = strtok(NULL, " ");
        }

        // второй проход: копируем каждое слово в отдельную память
        words = (char**)malloc((wordCount + 1) * sizeof(char*));
        strcpy(copy, string);   // восстанавливаем копию после первого прохода
        int w = 0;
        word = strtok(copy, " ");
        while (word != NULL) {

            words[w] = (char*)malloc((strlen(word) + 1) * sizeof(char));
            strcpy(words[w], word);
            w++;
            word = strtok(NULL, " ");
        }

        free(copy);   // копия больше не нужна
    }

    // аннулирование_предложения (деструктор)
    ~CodedSentence() {
        if (words != NULL) {
            for (int i = 0; i < wordCount; i++) {
                if (words[i] != NULL) {
                    free(words[i]);
                    words[i] = NULL;
                }
            }
            free(words);
            words = NULL;
        }
        if (string != NULL) {
            free(string);
            string = NULL;
        }
    }

    // получение_копии_упорядоченного_набора_закодированных_слов
    char** GetWordSetCopy(int& count) const {
        count = wordCount;
        char** copy = (char**)malloc((wordCount + 1) * sizeof(char*));
        for (int i = 0; i < wordCount; i++) {
            copy[i] = (char*)malloc((strlen(words[i]) + 1) * sizeof(char));
            strcpy(copy[i], words[i]);
        }
        return copy;
    }

    // вывод_строки_закодированных_символов
    void Display() const {
        if (string != NULL) {
            puts(string);
        }
    }
};


// Класс 2: ОБЫЧНОЕ ПРЕДЛОЖЕНИЕ

class AbstractSentence {
private:
    char* string;     // строка_обычных_символов
    char** words;     // упорядоченный_набор_обычных_слов
    int wordCount;

public:
    // создание_предложения_по_набору_обычных_слов (конструктор)
    AbstractSentence(char** wordSet, int count) {
        wordCount = count;
        words = (char**)malloc((wordCount + 1) * sizeof(char*));

        int totalLen = 0;
        for (int i = 0; i < count; i++) {
            words[i] = (char*)malloc((strlen(wordSet[i]) + 1) * sizeof(char));
            strcpy(words[i], wordSet[i]);
            totalLen += (int)strlen(wordSet[i]) + 1;   // слово + пробел
        }

        // собираем строку: слова через один пробел
        string = (char*)malloc((totalLen + 1) * sizeof(char));
        string[0] = '\0';   // начинаем с пустой строки
        for (int i = 0; i < count; i++) {
            if (i > 0) {
                strcat(string, " ");
            }
            strcat(string, words[i]);
        }
    }

    // аннулирование_предложения (деструктор)
    ~AbstractSentence() {
        if (words != NULL) {
            for (int i = 0; i < wordCount; i++) {
                if (words[i] != NULL) {
                    free(words[i]);
                    words[i] = NULL;
                }
            }
            free(words);
            words = NULL;
        }
        if (string != NULL) {
            free(string);
            string = NULL;
        }
    }

    // вывод_строки_обычных_символов
    void Display() const {
        if (string != NULL) {
            puts(string);
        }
    }
};


// Основная программа

int main() {
    

    // Таблица из 10 символов и их 5-значные двоичные коды
    const char* codes[10] = {
        "00000", "00001", "00010", "00011", "00100",
        "00101", "00110", "00111", "01000", "01001"
    };
    // Буквы для кодов: А Б В Г Д Е Ё Ж З И
    // (каждая буква — отдельная строка: в UTF-8 русская буква
    // занимает 2 байта и в один char не помещается)
    const char* letters[10] = {
        "А", "Б", "В", "Г", "Д",
        "Е", "Ё", "Ж", "З", "И"
    };

    puts("Введите закодированное предложение (Разделители кодов '*', слов ' '):");

    // 1. Создать Исходное_закодированное_предложение конструктором с вводом с клавиатуры.
    CodedSentence codedSentence;

    // 2. Получить копию набора слов Исходного_закодированного_предложения
    int count = 0;
    char** codedWords = codedSentence.GetWordSetCopy(count);

    // 3. Сформировать набор обычных слов (расшифровка)
    char** normalWords = (char**)malloc((count + 1) * sizeof(char*));
    for (int i = 0; i < count; i++) {
        // расшифровка не длиннее закодированного слова:
        // код из 5 символов даёт букву из 1-2 байт
        normalWords[i] = (char*)malloc((strlen(codedWords[i]) + 1) * sizeof(char));
        normalWords[i][0] = '\0';   // начинаем с пустой строки

        // делим закодированное слово на коды по символу '*'
        char* code = strtok(codedWords[i], "*");
        while (code != NULL) {
            const char* letter = "?";   // если кода нет в таблице, выводим '?'
            for (int t = 0; t < 10; t++) {
                if (strcmp(code, codes[t]) == 0) {
                    letter = letters[t];
                }
            }
            strcat(normalWords[i], letter);   // дописываем букву в конец слова
            code = strtok(NULL, "*");
        }
    }

    // 4. Создать обычное предложение по набору слов.
    AbstractSentence normalSentence(normalWords, count);

    // 5. Вывести строку закодированного_предложения и строку обычного_предложения.
    puts("\nЗакодированное предложение:");
    codedSentence.Display();

    puts("Обычное предложение:");
    normalSentence.Display();

    // Очистка памяти, выделенной в main
    if (codedWords != NULL) {
        for (int i = 0; i < count; i++) {
            if (codedWords[i] != NULL) {
                free(codedWords[i]);
                codedWords[i] = NULL;
            }
        }
        free(codedWords);
        codedWords = NULL;
    }
    if (normalWords != NULL) {
        for (int i = 0; i < count; i++) {
            if (normalWords[i] != NULL) {
                free(normalWords[i]);
                normalWords[i] = NULL;
            }
        }
        free(normalWords);
        normalWords = NULL;
    }

    // 6. Завершить выполнение.
    return 0;
}