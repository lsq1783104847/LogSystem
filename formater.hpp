#ifndef FORMATER_HPP
#define FORMATER_HPP

#include "level.hpp"

namespace log_system
{
    class Formater
    {
    public:
        Formater()
        {
            
        }
    private:
        std::string _format_str;
        std::string _result_str;
    };
}

#endif