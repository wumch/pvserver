
#include "Portal.hpp"

int main(int argc, char* argv[])
{
    pecar::Portal::init(argc, argv);
    return pecar::Portal().run();
}
