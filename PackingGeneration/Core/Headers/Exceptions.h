// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Core_Headers_Exceptions_h
#define Core_Headers_Exceptions_h

#include <string>

namespace Core
{
    //Represents a basic exception.
    class Exception
    {
    private:
        std::string message;
    public:
        explicit Exception(std::string message);
        virtual ~Exception();
        std::string GetMessage() const;
    };

    class NotImplementedException :
        public Exception
    {
    public:
        explicit NotImplementedException(std::string message);
        virtual ~NotImplementedException();
    };

    class InvalidOperationException :
        public Exception
    {
    public:
        explicit InvalidOperationException(std::string message);
        virtual ~InvalidOperationException();
    };
}

#endif /* Core_Headers_Exceptions_h */
