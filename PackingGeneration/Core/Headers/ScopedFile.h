// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Core_Headers_ScopedFile_h
#define Core_Headers_ScopedFile_h

#include <cstdio>
#include <string>
#include "Macros.h"
#include "Exceptions.h"

namespace Core
{
    /**
     * This is a file class, that closes FILE* handle upon destruction.
     * errorCode parameter will be logged by FileErrorHandler in
     * case fopen or fclose fails. The FileErrorHandler should provide a
     * void HandleError(const char*, int) function (see, e.g., LogErrorHandler).
    */
    template<typename FileErrorHandler>
    class ScopedFile
    {
    private:
        FileErrorHandler errorHandler;
        FILE* file;
        std::string fileName;
        int errorCode;

    public:
        ScopedFile(const char* fileName, const char* mode, int errorCode = 0)
        {
            Initialize(fileName, mode, errorCode);
        }

        ScopedFile(std::string fileName, const char* mode, int errorCode = 0)
        {
            Initialize(fileName.c_str(), mode, errorCode);
        }

        ~ScopedFile()
        {
            if (file != NULL)
            {
                int result = fclose(file);
                if (result == EOF)
                {
                    std::string errorMessage("Could not close the file ");
                    errorMessage
                           .append(fileName)
                           .append("\n");
                    errorHandler.HandleError(errorMessage.c_str(), errorCode);
                }
            }
        }

        bool Exists()
        {
            return file != NULL;
        }

        operator FILE* ()
        {
            return file;
        }

    private:
        void Initialize(const char* fileName, const char* mode, int errorCode = 0)
        {
            this->errorCode = errorCode;
            this->fileName = fileName;
            file = fopen(fileName, mode);

            if (file == NULL)
            {
                std::string errorMessage("Could not open file ");
                errorMessage
                       .append(fileName)
                       .append(" with mode ")
                       .append(mode)
                       .append("\n");
                errorHandler.HandleError(errorMessage.c_str(), errorCode);
            }
        }

        DISALLOW_COPY_AND_ASSIGN(ScopedFile);
    };

    struct EmptyErrorHandler
    {
        EmptyErrorHandler() {}

        void HandleError(const char* errorMesssage, int errorCode)
        {
            // do nothing
        }

    private:
        DISALLOW_COPY_AND_ASSIGN(EmptyErrorHandler);
    };

    struct ExceptionErrorHandler
    {
        ExceptionErrorHandler() {}

        void HandleError(const char* errorMesssage, int errorCode)
        {
            // This is a hack, to disable a warning (see http://bit.ly/WbqBa0)
            fprintf(stderr, "%s", errorMesssage);
            throw InvalidOperationException(errorMesssage);
        }

    private:
        DISALLOW_COPY_AND_ASSIGN(ExceptionErrorHandler);
    };

    struct LogErrorHandler
    {
        LogErrorHandler() {}

        void HandleError(const char* errorMesssage, int errorCode)
        {
            // This is a hack, to disable a warning (see http://bit.ly/WbqBa0)
            fprintf(stderr, "%s", errorMesssage);
        }

    private:
        DISALLOW_COPY_AND_ASSIGN(LogErrorHandler);
    };

//    struct MpiAbortErrorHandler
//    {
//        void HandleError(const char* errorMesssage, int errorCode)
//        {
//            // This is a hack, to disable a warning (see http://bit.ly/WbqBa0)
//            fprintf(stderr, "%s", errorMesssage);
//
//            fprintf(stderr,
//                    "Going to abort the program now. See %s near line %d\n",
//                    __FILE__,
//                    __LINE__);
//            fflush(stderr);
//            MPI_Abort(MPI_COMM_WORLD, errorCode);
//        }
//
//    private:
//        DISALLOW_COPY_AND_ASSIGN(MpiAbortOnError);
//    };
}
#endif /*Core_Headers_ScopedFile_h*/
