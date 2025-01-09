// Автор: Лукьянов Илья бИЦ-241
// Описание: Файл содержит реализацию системы мониторинга данных с печей для обжига керамики
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <locale.h>

typedef struct {
    char name[50];    // Имя печи
    double davl;      // Давление
    double temp;      // Температура
    char status[20];  // Статус
} pokazatel_t;

// Функция проверки отклонений температуры
int proverka_temp(const pokazatel_t* tempd, const pokazatel_t* pech, double time_seconds);
// Функция проверки давления
int proverka_davl(double current_davl, const pokazatel_t* davlen);
// Функция для изменения предельных значений температуры и давления
int change_limits(pokazatel_t* tempd, pokazatel_t* davlen);
// Функция для чтения данных из файла
int read_data_from_file(const char* filename, pokazatel_t* pech, long* file_position);
// Функция мониторинга печей
void monitor(const pokazatel_t* tempd, const pokazatel_t* davlen, double* time_seconds, const char* input_filename, int* rab, long* file_position);
// Функция проверки работы печей
int check_rab(const int* rab, int* not_working_pechi);
// Функция отображения информации о текущем состоянии печи
void display_info(double time_seconds, const pokazatel_t* pech);
// Функция обработки данных печи
void process_data(const pokazatel_t* tempd, const pokazatel_t* davlen, const pokazatel_t* pech, int* rab, double time_seconds);
// Функция счетчик работы этой печи
void check_rab_count(const pokazatel_t* pech, int* rab);
// Функция для чтения строки из файла
int read_line_from_file(FILE* file, pokazatel_t* pech);
// Функция поиска индекса печи по имени
int naiti_index(const char* name);
// Функция для вычисления среднего значения
double sred(const char* filename);
// Функция записи давления в файл
int write_davl_to_file(const char* filename, double davl, const char* name);
// Функция вычисляет ожидаемую температуру 
double grafot(double x) {
    return x * atan(x) - log(sqrt(1 + pow(x, 2)));
}

int main() {
    setlocale(LC_CTYPE, "RUS");

    pokazatel_t tempd = { "m", 0, 2, "k" };  // Данные для температуры
    pokazatel_t davlen = { "n", 0, 100, "r" };  // Данные для давления
    int rab[50] = { 0 };  // Массив для работы счетчиков печей
    long file_position = 0;  // Переменная для хранения позиции в файле
    double time_seconds = 0.0;

    int choice;
    char input_filename[260];  // Имя входного файла

    printf("=== Программа мониторинга печи для обжига ===\n");
    printf("Введите имя файла с данными для мониторинга: ");
    scanf("%s", input_filename);

    while (1) {
        printf("=== Меню ===\n");
        printf("1. Считать данные из файла и выполнить мониторинг\n");
        printf("2. Изменить допустимые пределы температуры и давления\n");
        printf("3. Какие печи не работали\n");
        printf("4. Считать среднее значение давления из файла\n");
        printf("5. Выход\n");
        printf("Введите ваш выбор: ");
        scanf("%d", &choice);

        switch (choice) {
        case 1:
            if (read_data_from_file(input_filename, &tempd, &file_position) == 0) {
                printf("Не удалось считать данные из файла.\n");
            }
            else {
                monitor(&tempd, &davlen, &time_seconds, input_filename, rab, &file_position);
            }
            break;

        case 2:
            if (change_limits(&tempd, &davlen) == 0) {
                printf("Допустимые пределы успешно изменены.\n");
            }
            else {
                printf("Ошибка при изменении пределов.\n");
            }
            break;

        case 3: {
            int not_working_pechi[50];
            int not_working_count = check_rab(rab, not_working_pechi);
            if (not_working_count > 0) {
                printf("Печи, которые не работали:\n");
                for (int i = 0; i < not_working_count; i++) {
                    printf("Печь под номером %d не работала.\n", not_working_pechi[i]);
                }
            }
            else {
                printf("Все печи работали.\n");
            }
            break;
        }

        case 4: {
            char filename[260];
            printf("Введите имя файла для вычисления среднего давления: ");
            scanf("%s", filename);
            double sred_davl = sred(filename);
            if (sred_davl != -1.0) {
                printf("Среднее значение давления: %.2f Па\n", sred_davl);
            }
            else {
                printf("Не удалось вычислить среднее значение давления.\n");
            }
            break;
        }

        case 5:
            printf("Выход из программы.\n");
            return 0;

        default:
            printf("Некорректный выбор. Попробуйте снова.\n");
        }
    }

    return 0;
}
/**
 * Функция мониторинга печей
 * tempd - указатель на структуру с допустимыми пределами температуры
 * davlen - указатель на структуру с допустимыми пределами давления
 * time_seconds - указатель на переменную с текущим временем
 * input_filename - имя файла с данными для мониторинга
 */
void monitor(const pokazatel_t* tempd, const pokazatel_t* davlen, double* time_seconds, const char* input_filename, int* rab, long* file_position) {
    int line_count = 0;
    while (1) {
        pokazatel_t pech;
        if (read_data_from_file(input_filename, &pech, file_position) == 0) {
            break;
        }


        display_info(*time_seconds, &pech);
        process_data(tempd, davlen, &pech, rab, *time_seconds);


        int result = write_davl_to_file("output.txt", pech.davl, pech.name);
        if (result == -1) {
            printf("Ошибка: Не удалось открыть файл для записи.\n");
        }
        else if (result == -2) {
            printf("Ошибка: Не удалось записать данные в файл.\n");
        }
        else {
            printf("Данные успешно записаны в файл.\n");
        }


        line_count++;
        if (line_count == 50) {
            *time_seconds += 3.0;
            line_count = 0;
            break;
        }
    }
}
/**
 * Функция записи давления в файл
 * filename - имя выходного файла
 * davl - значение давления
 * name - имя печи
 */
 /**
 * Функция записи давления в файл
 * filename - имя выходного файла
 * davl - значение давления
 * name - имя печи
 * return 0 - успешная запись, -1 - ошибка при открытии файла, -2 - ошибка при записи
 */
int write_davl_to_file(const char* filename, double davl, const char* name) {
    static int first_call = 1;  // Флаг для проверки первого вызова функции
    FILE* output_file;

    // Открытие файла в зависимости от первого вызова
    if (first_call) {
        output_file = fopen(filename, "w");
        first_call = 0;  // После первого вызова меняем флаг
    }
    else {
        output_file = fopen(filename, "a");
    }

    // Проверка на успешное открытие файла
    if (output_file == NULL) {
        perror("Ошибка при открытии файла для записи давления");
        return -1;
    }

    // Запись данных в файл
    if (fprintf(output_file, "%f %s\n", davl, name) < 0) {
        perror("Ошибка при записи данных в файл");
        fclose(output_file);
        return -2;
    }

    fclose(output_file);
    return 0;  // Успешное завершение
}

/**
 * Функция чтения данных из файла
 * filename - имя файла для чтения
 * pech - указатель на структуру для хранения данных печи
 * file_position - указатель на переменную для хранения текущей позиции в файле
 * return 1 - успешное чтение, 0 - ошибка
 */
int read_data_from_file(const char* filename, pokazatel_t* pech, long* file_position) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        perror("Ошибка при открытии файла");
        return 0;
    }


    fseek(file, *file_position, SEEK_SET);

    if (read_line_from_file(file, pech) == 0) {
        fclose(file);
        return 0;
    }

    *file_position = ftell(file);  // Обновляет позицию в файле
    fclose(file);
    return 1;
}
/**
 * Функция чтения строки из файла
 * file - указатель на файл
 * pech - указатель на структуру для хранения данных печи
 * return 1 - успешное чтение, 0 - ошибка
 */
int read_line_from_file(FILE* file, pokazatel_t* pech) {
    char line[255];

    if (fgets(line, sizeof(line), file) == NULL) {
        return 0;
    }

    // Разбиваем строку по '-'
    char* token = strtok(line, "-");
    if (token != NULL) strcpy(pech->name, token);

    token = strtok(NULL, "-");
    if (token != NULL) pech->temp = atof(token);

    token = strtok(NULL, "-");
    if (token != NULL) pech->davl = atof(token);

    token = strtok(NULL, "-");
    if (token != NULL) {
        strcpy(pech->status, token);
        pech->status[strcspn(pech->status, "\n")] = '\0'; // Убирает лишний символ новой строки
    }

    return 1;
}
/**
 * Функция отображения информации о текущем состоянии печи
 * time_seconds - текущее время в секундах
 * pech - указатель на структуру с данными печи
 */
void display_info(double time_seconds, const pokazatel_t* pech) {
    printf("\nВремя: %.2f с\n", time_seconds);
    printf("Оборудование: %s\n", pech->name);
    printf("Температура: %.2f°C (ожидаемая: %.2f°C)\nДавление: %.2fПа\nСостояние: %s\n",
        pech->temp, grafot(time_seconds), pech->davl, pech->status);
}
/**
 * Функция обработки данных печи
 * tempd - указатель на структуру с допустимыми пределами температуры
 * davlen - указатель на структуру с допустимыми пределами давления
 * pech - указатель на структуру с данными текущей печи
 * rab - массив счетчиков работы печей
 * time_seconds - текущее время в секундах
 */
void process_data(const pokazatel_t* tempd, const pokazatel_t* davlen, const pokazatel_t* pech, int* rab, double time_seconds) {
    int temp_status = proverka_temp(tempd, pech, time_seconds); // Проверка температуры
    int davl_status = proverka_davl(pech->davl, davlen);  // Проверка давления с использованием текущего давления

    if (temp_status) {
        printf("!!! Уведомление: Отклонение температуры превышает допустимый порог!\n");
    }
    else {
        printf("Температура в пределах нормы.\n");
    }

    if (davl_status) {
        printf("!!! Уведомление: Давление вне допустимого диапазона!\n");
    }
    else {
        printf("Давление в пределах нормы.\n");
    }

    check_rab_count(pech, rab);
}
/**
 * Функция проверки отклонений температуры
 * tempd - указатель на структуру с допустимыми пределами температуры
 * pech - указатель на структуру с текущими данными печи
 * time_seconds - текущее время в секундах
 * return 1 - если отклонение температуры превышает предел, 0 - если в пределах нормы
 */
int proverka_temp(const pokazatel_t* tempd, const pokazatel_t* pech, double time_seconds) {
    double expected_temp = grafot(time_seconds);
    double deviation = fabs(pech->temp - expected_temp);
    return deviation > tempd->temp ? 1 : 0; // Если отклонение превышает допустимый предел, возвращаем 1
}
/**
 * Функция проверки давления
 * current_davl - текущее давление
 * davlen - указатель на структуру с допустимыми пределами давления
 * return 1 - если давление вне диапазона, 0 - если в пределах нормы
 */
int proverka_davl(double current_davl, const pokazatel_t* davlen) {
    return (current_davl < davlen->davl || current_davl > davlen->temp) ? 1 : 0; // 1 - вне диапазона, 0 - норма
}
/**
 * Функция проверки работы конкретной печи и обновления счетчика
 * pech - указатель на структуру с данными текущей печи
 * rab - массив счетчиков работы печей
 */
void check_rab_count(const pokazatel_t* pech, int* rab) {
    // Проверка на статус "Ok" и увеличение счетчика работы печи
    if (strcmp(pech->status, "Ok") == 0) {
        int ind = naiti_index(pech->name) - 1;
        if (ind >= 0 && ind < 50) {
            rab[ind] += 1;
        }
    }
}
/**
 * Функция изменения предельных значений температуры и давления
 * tempd - указатель на структуру с допустимыми пределами температуры
 * davlen - указатель на структуру с допустимыми пределами давления
 * return 0 - успешное изменение, -1 - ошибка ввода
 */
int change_limits(pokazatel_t* tempd, pokazatel_t* davlen) {
    printf("\n=== Изменение допустимых значений ===\n");

    printf("Текущие пределы температуры: %.2f°C\n", tempd->temp);
    printf("Введите новое значение допустимого отклонения температуры: ");
    if (scanf("%lf", &tempd->temp) != 1) {
        printf("Ошибка ввода для температуры.\n");
        return -1;
    }
    printf("Текущие минимальное и максимальное давление: %.2fПа, %.2fПа\n", davlen->davl, davlen->temp);
    printf("Введите новое минимальное давление: ");
    if (scanf("%lf", &davlen->davl) != 1) {
        printf("Ошибка ввода для минимального давления.\n");
        return -1;
    }
    printf("Введите новое максимальное давление: ");
    if (scanf("%lf", &davlen->temp) != 1) {
        printf("Ошибка ввода для максимального давления.\n");
        return -1;
    }
    return 0;
}
/**
 * Функция проверки работы печей
 * rab - массив счетчиков работы печей
 * not_working_pechi - массив для хранения номеров неработающих печей
 * return количество неработающих печей
 */
int check_rab(const int* rab, int* not_working_pechi) {
    int not_working_count = 0;

    for (int k = 0; k < 50; k++) {
        if (rab[k] == 0) {
            not_working_pechi[not_working_count] = k + 1;  // Запоминаем номер неработающей печи
            not_working_count++;  // Увеличивает счетчик неработающих печей
        }
    }

    return not_working_count;  // Возвращает количество неработающих печей
}
/**
 * Функция поиска индекса печи по имени
 * name - имя печи
 * return индекс печи или -1, если не удалось найти
 */
int naiti_index(const char* name) {
    int index = -1;
    if (sscanf(name, "Name%d", &index) == 1) {
        return index;
    }
    return -1;
}
/**
 * Функция вычисления среднего значения давления
 * filename - имя файла для чтения данных
 * return среднее значение давления или -1.0 в случае ошибки
 */
double sred(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        perror("Ошибка при открытии файла для чтения");
        return -1.0;
    }

    double sumsred = 0.0;
    int count = 0;
    double davl;

    while (fscanf(file, "%lf\n", &davl) == 1) {
        sumsred += davl;
        count++;
    }

    fclose(file);

    if (count > 0) {
        return sumsred / count;
    }
    else {
        return -1.0;
    }
}