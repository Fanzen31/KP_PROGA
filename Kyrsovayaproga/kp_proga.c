// �����: �������� ���� ���-241
// ��������: ���� �������� ���������� ������� ����������� ������ � ����� ��� ������ ��������
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <locale.h>

typedef struct {
    char name[50];    // ��� ����
    double davl;      // ��������
    double temp;      // �����������
    char status[20];  // ������
} pokazatel_t;

// ������� �������� ���������� �����������
int proverka_temp(const pokazatel_t* tempd, const pokazatel_t* pech, double time_seconds);
// ������� �������� ��������
int proverka_davl(double current_davl, const pokazatel_t* davlen);
// ������� ��� ��������� ���������� �������� ����������� � ��������
int change_limits(pokazatel_t* tempd, pokazatel_t* davlen);
// ������� ��� ������ ������ �� �����
int read_data_from_file(const char* filename, pokazatel_t* pech, long* file_position);
// ������� ����������� �����
void monitor(const pokazatel_t* tempd, const pokazatel_t* davlen, double* time_seconds, const char* input_filename, int* rab, long* file_position);
// ������� �������� ������ �����
int check_rab(const int* rab, int* not_working_pechi);
// ������� ����������� ���������� � ������� ��������� ����
void display_info(double time_seconds, const pokazatel_t* pech);
// ������� ��������� ������ ����
void process_data(const pokazatel_t* tempd, const pokazatel_t* davlen, const pokazatel_t* pech, int* rab, double time_seconds);
// ������� ������� ������ ���� ����
void check_rab_count(const pokazatel_t* pech, int* rab);
// ������� ��� ������ ������ �� �����
int read_line_from_file(FILE* file, pokazatel_t* pech);
// ������� ������ ������� ���� �� �����
int naiti_index(const char* name);
// ������� ��� ���������� �������� ��������
double sred(const char* filename);
// ������� ������ �������� � ����
int write_davl_to_file(const char* filename, double davl, const char* name);
// ������� ��������� ��������� ����������� 
double grafot(double x) {
    return x * atan(x) - log(sqrt(1 + pow(x, 2)));
}

int main() {
    setlocale(LC_CTYPE, "RUS");

    pokazatel_t tempd = { "m", 0, 2, "k" };  // ������ ��� �����������
    pokazatel_t davlen = { "n", 0, 100, "r" };  // ������ ��� ��������
    int rab[50] = { 0 };  // ������ ��� ������ ��������� �����
    long file_position = 0;  // ���������� ��� �������� ������� � �����
    double time_seconds = 0.0;

    int choice;
    char input_filename[260];  // ��� �������� �����

    printf("=== ��������� ����������� ���� ��� ������ ===\n");
    printf("������� ��� ����� � ������� ��� �����������: ");
    scanf("%s", input_filename);

    while (1) {
        printf("=== ���� ===\n");
        printf("1. ������� ������ �� ����� � ��������� ����������\n");
        printf("2. �������� ���������� ������� ����������� � ��������\n");
        printf("3. ����� ���� �� ��������\n");
        printf("4. ������� ������� �������� �������� �� �����\n");
        printf("5. �����\n");
        printf("������� ��� �����: ");
        scanf("%d", &choice);

        switch (choice) {
        case 1:
            if (read_data_from_file(input_filename, &tempd, &file_position) == 0) {
                printf("�� ������� ������� ������ �� �����.\n");
            }
            else {
                monitor(&tempd, &davlen, &time_seconds, input_filename, rab, &file_position);
            }
            break;

        case 2:
            if (change_limits(&tempd, &davlen) == 0) {
                printf("���������� ������� ������� ��������.\n");
            }
            else {
                printf("������ ��� ��������� ��������.\n");
            }
            break;

        case 3: {
            int not_working_pechi[50];
            int not_working_count = check_rab(rab, not_working_pechi);
            if (not_working_count > 0) {
                printf("����, ������� �� ��������:\n");
                for (int i = 0; i < not_working_count; i++) {
                    printf("���� ��� ������� %d �� ��������.\n", not_working_pechi[i]);
                }
            }
            else {
                printf("��� ���� ��������.\n");
            }
            break;
        }

        case 4: {
            char filename[260];
            printf("������� ��� ����� ��� ���������� �������� ��������: ");
            scanf("%s", filename);
            double sred_davl = sred(filename);
            if (sred_davl != -1.0) {
                printf("������� �������� ��������: %.2f ��\n", sred_davl);
            }
            else {
                printf("�� ������� ��������� ������� �������� ��������.\n");
            }
            break;
        }

        case 5:
            printf("����� �� ���������.\n");
            return 0;

        default:
            printf("������������ �����. ���������� �����.\n");
        }
    }

    return 0;
}
/**
 * ������� ����������� �����
 * tempd - ��������� �� ��������� � ����������� ��������� �����������
 * davlen - ��������� �� ��������� � ����������� ��������� ��������
 * time_seconds - ��������� �� ���������� � ������� ��������
 * input_filename - ��� ����� � ������� ��� �����������
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
            printf("������: �� ������� ������� ���� ��� ������.\n");
        }
        else if (result == -2) {
            printf("������: �� ������� �������� ������ � ����.\n");
        }
        else {
            printf("������ ������� �������� � ����.\n");
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
 * ������� ������ �������� � ����
 * filename - ��� ��������� �����
 * davl - �������� ��������
 * name - ��� ����
 */
 /**
 * ������� ������ �������� � ����
 * filename - ��� ��������� �����
 * davl - �������� ��������
 * name - ��� ����
 * return 0 - �������� ������, -1 - ������ ��� �������� �����, -2 - ������ ��� ������
 */
int write_davl_to_file(const char* filename, double davl, const char* name) {
    static int first_call = 1;  // ���� ��� �������� ������� ������ �������
    FILE* output_file;

    // �������� ����� � ����������� �� ������� ������
    if (first_call) {
        output_file = fopen(filename, "w");
        first_call = 0;  // ����� ������� ������ ������ ����
    }
    else {
        output_file = fopen(filename, "a");
    }

    // �������� �� �������� �������� �����
    if (output_file == NULL) {
        perror("������ ��� �������� ����� ��� ������ ��������");
        return -1;
    }

    // ������ ������ � ����
    if (fprintf(output_file, "%f %s\n", davl, name) < 0) {
        perror("������ ��� ������ ������ � ����");
        fclose(output_file);
        return -2;
    }

    fclose(output_file);
    return 0;  // �������� ����������
}

/**
 * ������� ������ ������ �� �����
 * filename - ��� ����� ��� ������
 * pech - ��������� �� ��������� ��� �������� ������ ����
 * file_position - ��������� �� ���������� ��� �������� ������� ������� � �����
 * return 1 - �������� ������, 0 - ������
 */
int read_data_from_file(const char* filename, pokazatel_t* pech, long* file_position) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        perror("������ ��� �������� �����");
        return 0;
    }


    fseek(file, *file_position, SEEK_SET);

    if (read_line_from_file(file, pech) == 0) {
        fclose(file);
        return 0;
    }

    *file_position = ftell(file);  // ��������� ������� � �����
    fclose(file);
    return 1;
}
/**
 * ������� ������ ������ �� �����
 * file - ��������� �� ����
 * pech - ��������� �� ��������� ��� �������� ������ ����
 * return 1 - �������� ������, 0 - ������
 */
int read_line_from_file(FILE* file, pokazatel_t* pech) {
    char line[255];

    if (fgets(line, sizeof(line), file) == NULL) {
        return 0;
    }

    // ��������� ������ �� '-'
    char* token = strtok(line, "-");
    if (token != NULL) strcpy(pech->name, token);

    token = strtok(NULL, "-");
    if (token != NULL) pech->temp = atof(token);

    token = strtok(NULL, "-");
    if (token != NULL) pech->davl = atof(token);

    token = strtok(NULL, "-");
    if (token != NULL) {
        strcpy(pech->status, token);
        pech->status[strcspn(pech->status, "\n")] = '\0'; // ������� ������ ������ ����� ������
    }

    return 1;
}
/**
 * ������� ����������� ���������� � ������� ��������� ����
 * time_seconds - ������� ����� � ��������
 * pech - ��������� �� ��������� � ������� ����
 */
void display_info(double time_seconds, const pokazatel_t* pech) {
    printf("\n�����: %.2f �\n", time_seconds);
    printf("������������: %s\n", pech->name);
    printf("�����������: %.2f�C (���������: %.2f�C)\n��������: %.2f��\n���������: %s\n",
        pech->temp, grafot(time_seconds), pech->davl, pech->status);
}
/**
 * ������� ��������� ������ ����
 * tempd - ��������� �� ��������� � ����������� ��������� �����������
 * davlen - ��������� �� ��������� � ����������� ��������� ��������
 * pech - ��������� �� ��������� � ������� ������� ����
 * rab - ������ ��������� ������ �����
 * time_seconds - ������� ����� � ��������
 */
void process_data(const pokazatel_t* tempd, const pokazatel_t* davlen, const pokazatel_t* pech, int* rab, double time_seconds) {
    int temp_status = proverka_temp(tempd, pech, time_seconds); // �������� �����������
    int davl_status = proverka_davl(pech->davl, davlen);  // �������� �������� � �������������� �������� ��������

    if (temp_status) {
        printf("!!! �����������: ���������� ����������� ��������� ���������� �����!\n");
    }
    else {
        printf("����������� � �������� �����.\n");
    }

    if (davl_status) {
        printf("!!! �����������: �������� ��� ����������� ���������!\n");
    }
    else {
        printf("�������� � �������� �����.\n");
    }

    check_rab_count(pech, rab);
}
/**
 * ������� �������� ���������� �����������
 * tempd - ��������� �� ��������� � ����������� ��������� �����������
 * pech - ��������� �� ��������� � �������� ������� ����
 * time_seconds - ������� ����� � ��������
 * return 1 - ���� ���������� ����������� ��������� ������, 0 - ���� � �������� �����
 */
int proverka_temp(const pokazatel_t* tempd, const pokazatel_t* pech, double time_seconds) {
    double expected_temp = grafot(time_seconds);
    double deviation = fabs(pech->temp - expected_temp);
    return deviation > tempd->temp ? 1 : 0; // ���� ���������� ��������� ���������� ������, ���������� 1
}
/**
 * ������� �������� ��������
 * current_davl - ������� ��������
 * davlen - ��������� �� ��������� � ����������� ��������� ��������
 * return 1 - ���� �������� ��� ���������, 0 - ���� � �������� �����
 */
int proverka_davl(double current_davl, const pokazatel_t* davlen) {
    return (current_davl < davlen->davl || current_davl > davlen->temp) ? 1 : 0; // 1 - ��� ���������, 0 - �����
}
/**
 * ������� �������� ������ ���������� ���� � ���������� ��������
 * pech - ��������� �� ��������� � ������� ������� ����
 * rab - ������ ��������� ������ �����
 */
void check_rab_count(const pokazatel_t* pech, int* rab) {
    // �������� �� ������ "Ok" � ���������� �������� ������ ����
    if (strcmp(pech->status, "Ok") == 0) {
        int ind = naiti_index(pech->name) - 1;
        if (ind >= 0 && ind < 50) {
            rab[ind] += 1;
        }
    }
}
/**
 * ������� ��������� ���������� �������� ����������� � ��������
 * tempd - ��������� �� ��������� � ����������� ��������� �����������
 * davlen - ��������� �� ��������� � ����������� ��������� ��������
 * return 0 - �������� ���������, -1 - ������ �����
 */
int change_limits(pokazatel_t* tempd, pokazatel_t* davlen) {
    printf("\n=== ��������� ���������� �������� ===\n");

    printf("������� ������� �����������: %.2f�C\n", tempd->temp);
    printf("������� ����� �������� ����������� ���������� �����������: ");
    if (scanf("%lf", &tempd->temp) != 1) {
        printf("������ ����� ��� �����������.\n");
        return -1;
    }
    printf("������� ����������� � ������������ ��������: %.2f��, %.2f��\n", davlen->davl, davlen->temp);
    printf("������� ����� ����������� ��������: ");
    if (scanf("%lf", &davlen->davl) != 1) {
        printf("������ ����� ��� ������������ ��������.\n");
        return -1;
    }
    printf("������� ����� ������������ ��������: ");
    if (scanf("%lf", &davlen->temp) != 1) {
        printf("������ ����� ��� ������������� ��������.\n");
        return -1;
    }
    return 0;
}
/**
 * ������� �������� ������ �����
 * rab - ������ ��������� ������ �����
 * not_working_pechi - ������ ��� �������� ������� ������������ �����
 * return ���������� ������������ �����
 */
int check_rab(const int* rab, int* not_working_pechi) {
    int not_working_count = 0;

    for (int k = 0; k < 50; k++) {
        if (rab[k] == 0) {
            not_working_pechi[not_working_count] = k + 1;  // ���������� ����� ������������ ����
            not_working_count++;  // ����������� ������� ������������ �����
        }
    }

    return not_working_count;  // ���������� ���������� ������������ �����
}
/**
 * ������� ������ ������� ���� �� �����
 * name - ��� ����
 * return ������ ���� ��� -1, ���� �� ������� �����
 */
int naiti_index(const char* name) {
    int index = -1;
    if (sscanf(name, "Name%d", &index) == 1) {
        return index;
    }
    return -1;
}
/**
 * ������� ���������� �������� �������� ��������
 * filename - ��� ����� ��� ������ ������
 * return ������� �������� �������� ��� -1.0 � ������ ������
 */
double sred(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        perror("������ ��� �������� ����� ��� ������");
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