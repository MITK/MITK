#include <stdio.h>

extern int    test_1();
extern int    test_2();
extern int    test_3();
extern int    test_4();
extern int    test_5();
extern int    test_6();
extern int    test_7();
extern int    test_8();
extern int    test_9();
extern int    test_10();
extern int    test_11();
extern int    test_12();

int main(int, char**) {
    int ret = 0;
    ret = test_1();
    if (ret == 0)
        printf("test 1 success\n");
    ret = test_2();
    if (ret == 0)
        printf("test 2 success\n");
    ret = test_3();
    if (ret == 0)
        printf("test 3 success\n");
    ret = test_4();
    if (ret == 0)
        printf("test 4 success\n");
    ret = test_5();
    if (ret == 0)
        printf("test 5 success\n");
    ret = test_6();
    if (ret == 0)
        printf("test 6 success\n");
    ret = test_7();
    if (ret == 0)
        printf("test 7 success\n");
    ret = test_8();
    if (ret == 0)
        printf("test 8 success\n");
    ret = test_9();
    if (ret == 0)
        printf("test 9 success\n");
    ret = test_10();
    if (ret == 0)
        printf("test 10 success\n");
    ret = test_11();
    if (ret == 0)
        printf("test 11 success\n");
    ret = test_12();
    if (ret == 0)
        printf("test 12 success\n");
    ret = printf("** all test success.\n");
    return 0;
}

