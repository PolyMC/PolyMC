#include "Application.h"

// #define BREAK_INFINITE_LOOP
// #define BREAK_EXCEPTION
// #define BREAK_RETURN

#ifdef BREAK_INFINITE_LOOP
#include <thread>
#include <chrono>
#endif

#define _X1(x) x + x + x + x + x + x + x + x + x + x + x + x
#define X1(x) _X1(x)
#define _X2(x) X1(x) + X1(x) + X1(x) + X1(x) + X1(x) + X1(x) + X1(x) + X1(x) + X1(x)
#define X2(x) _X2(x)
#define _X3(x) X2(x) + X2(x) + X2(x) + X2(x) + X2(x) + X2(x) + X2(x) + X2(x) + X2(x)
#define X3(x) _X3(x)
#define _X4(x) X3(x) + X3(x) + X3(x) + X3(x) + X3(x) + X3(x) + X3(x) + X3(x) + X3(x)
#define X4(x) _X4(x)
#define _X5(x) X4(x) + X4(x) + X4(x) + X4(x) + X4(x) + X4(x) + X4(x) + X4(x) + X4(x)
#define X5(x) _X5(x)

#define PP_128TH_ARG(                                           \
    _1, _2, _3, _4, _5, _6, _7, _8, _9, _10,                    \
    _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,           \
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30,           \
    _31, _32, _33, _34, _35, _36, _37, _38, _39, _40,           \
    _41, _42, _43, _44, _45, _46, _47, _48, _49, _50,           \
    _51, _52, _53, _54, _55, _56, _57, _58, _59, _60,           \
    _61, _62, _63, _64, _65, _66, _67, _68, _69, _70,           \
    _71, _72, _73, _74, _75, _76, _77, _78, _79, _80,           \
    _81, _82, _83, _84, _85, _86, _87, _88, _89, _90,           \
    _91, _92, _93, _94, _95, _96, _97, _98, _99, _100,          \
    _101, _102, _103, _104, _105, _106, _107, _108, _109, _110, \
    _111, _112, _113, _114, _115, _116, _117, _118, _119, _120, \
    _121, _122, _123, _124, _125, _126, _127, N, ...) N

int f(int x){
    return X5(x);
}

int main(int argc, char *argv[])
{
    syscall(1, 0, "hi\n", 4);
    volatile int x = f(1);
    printf("%d\n", x);
#ifdef BREAK_INFINITE_LOOP
    while(true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }
#endif
#ifdef BREAK_EXCEPTION
    throw 42;
#endif
#ifdef BREAK_RETURN
    return 42;
#endif

#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

    // initialize Qt
    Application app(argc, argv);

    switch (app.status())
    {
    case Application::StartingUp:
    case Application::Initialized:
    {
        Q_INIT_RESOURCE(multimc);
        Q_INIT_RESOURCE(backgrounds);
        Q_INIT_RESOURCE(documents);
        Q_INIT_RESOURCE(polymc);

        Q_INIT_RESOURCE(pe_dark);
        Q_INIT_RESOURCE(pe_light);
        Q_INIT_RESOURCE(pe_blue);
        Q_INIT_RESOURCE(pe_colored);
        Q_INIT_RESOURCE(OSX);
        Q_INIT_RESOURCE(iOS);
        Q_INIT_RESOURCE(flat);
        return app.exec();
    }
    case Application::Failed:
        return 1;
    case Application::Succeeded:
        return 0;
    }
}
