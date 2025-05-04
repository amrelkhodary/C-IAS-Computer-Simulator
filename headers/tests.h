#ifndef H_TESTS
    #define H_TESTS
    typedef enum Result {
        TEST_SUCCESSFUL, 
        TEST_FAILED 
    } Result;

    int runtests();
    int test_isNegative(IAS* ias);
    int test_negative(IAS* ias);
    int test_absoluteval(IAS* ias);
    int test_loadmq(IAS* ias);
    int test_loadmqmx(IAS* ias);
    int test_stormx(IAS* ias);
    int test_loadmx(IAS* ias);
    int test_loadnmx(IAS* ias);
    int test_loadamx(IAS* ias);
    int test_loadnamx(IAS* ias);
    int test_jumplmx(IAS* ias);
    int test_jumprmx(IAS* ias);
    int test_cjumplmx(IAS* ias);
    int test_cjumprmx(IAS* ias);
    int test_storlmx(IAS* ias);
    int test_storrmx(IAS* ias);
    int test_arithmetic(IAS* ias);
#endif