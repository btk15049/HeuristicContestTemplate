#define DBG(x)                                                               \
    do {                                                                     \
        std::cerr << "[Line: " << __LINE__ << ", Function: " << __FUNCTION__ \
                  << "] " << x << endl;                                      \
    } while (0)

#define DBG_VAR(x) DBG(#x << " = " << x)
