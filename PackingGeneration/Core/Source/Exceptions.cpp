// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/Exceptions.h"

namespace Core
{
    Exception::Exception(std::string message)
    {
        this->message = message;
    }
    Exception::~Exception()
    {
    }
    
    std::string Exception::GetMessage() const
    {
        return message;
    }

    NotImplementedException::NotImplementedException(std::string message) : Exception(message)
    {

    }

    NotImplementedException::~NotImplementedException()
    {
    }

    InvalidOperationException::InvalidOperationException(std::string message) : Exception(message)
    {

    }

    InvalidOperationException::~InvalidOperationException()
    {

    }
}
