#include "utils.h"
#include "strategy.h"

int main(int argc, char **argv)
{
    Strategy strategy;
    try 
    {
        strategy.read_config(argv[1]);
        strategy.read_strategy();
        strategy.run_simulation();
    }
    catch (const char* msg)
    {
        LOG << msg << endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}