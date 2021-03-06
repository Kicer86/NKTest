#include <QtWidgets/QApplication>
#include "NKTest.h"
#include <boost/filesystem.hpp>
#include "NKTestConstants.h"

int main(int argc, char** argv)
{
    if (!boost::filesystem::exists(nk_quiz_directory) || !boost::filesystem::is_directory(nk_quiz_directory))
        boost::filesystem::create_directories(nk_quiz_directory);

    QApplication a(argc, argv);
    NKTest w;
    w.show();
    return a.exec();
}