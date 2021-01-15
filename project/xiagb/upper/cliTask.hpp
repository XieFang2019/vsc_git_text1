#ifndef cliTaskH
#define cliTaskH

#include "cli.hpp"
#include "can.hpp"

namespace xiagb
{
    namespace upper
    {
        namespace cliTaskNamespace
        {
            using xiagb::communicationNamespace::canNamespace::canClass;
            using xiagb::parserNamespace::cliNamespace::cliClass;
            
            extern canClass *canCommunication;
            extern cliClass *commandParser;
            
            void cliTaskStart();
        } // namespace cliTaskNamespace
    }     // namespace upper
} // namespace xiagb

#endif