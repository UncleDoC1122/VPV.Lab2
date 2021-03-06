//*******************************************************************************************
// Пример исследования многопоточных реализации вычисления интеграла функции sin(x)/x
// Для каждого из релизов проводятся:
//  - оценка ошибки относительно эталонного значения;
//  - серия измерений затрат времени, результаты которых фильтруются и для них вычисляются 
//    минимальное, максимальное, среднее значения и средне-квадратичное отклонение
// Таблицы отчетов можно просматривать в диалоге, а также выводить в файл allResult.txt
// Для всех результатов может быть построены гистограммы в svg-формате
// Вызов: vpv-lab3  [slen:N1] [dmax:N2]
// где N1 - длина серии измерений; 
//     N2 - число удаляемых максимальных результатов серии при фильтрации; 
//     вокруг двоеточия не должно быть пробелов
//*******************************************************************************************

#include "init.h"
#include "report.h"
#include "profiler.h"
#include "integral.h"
#include "main.h"

using namespace std;

Config config(
    50,     // число замеров в серии измерений
    10,     // число удаляемых максимумов во время фильтрации результатов серии
    &report, // адрес отчета, куда складываются результаты измерений
    MAX_ERR, // максимально допустимая погрешность вычислений
    0.0     // эталонное значение интеграла для верификатора (вычисляется в init())
    );

int main(int argc, char* argv[])
{
    init(argc, argv, config);
    // Создание массива объектов тестирования
    Tester tests[COUNT_FUNCTION] = {
        Tester("integralThreadLock", integralThreadLock, config, 0),
        Tester("integralThreadStNoLock", integralThreadStNoLock, config, 1),
        Tester("integralThreadStLock", integralThreadStLock, config, 2),
        Tester("integralOpenMP", integralOpenMP, config, 3)
    };
    cout << endl << "Функциональное тестирование ..." << endl;
    for (Tester test : tests)
        test.verify();
    cout << endl << "Замеры времени(микросекунды) ..." << endl;
    // Цикл поддержки анализа повторяемости
    do {
        for (Tester test : tests)
            test.measure();
    } while (report.show());
    return 0;
}

