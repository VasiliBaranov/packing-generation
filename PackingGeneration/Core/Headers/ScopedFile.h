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
    // See http://www.cplusplus.com/reference/cstdio/fopen/
    struct FileOpenMode
    {
        enum Type
        {
            Read = 1,
            Write = 2,
            Append = 4,
            Update = 8,
            Binary = 16
        };
    };

    // See http://stackoverflow.com/questions/1448396/how-to-use-enums-as-flags-in-c
    // TODO: create macros for this
    inline FileOpenMode::Type operator | (FileOpenMode::Type a, FileOpenMode::Type b) { return static_cast<FileOpenMode::Type>(static_cast<int>(a) | static_cast<int>(b)); }
    inline FileOpenMode::Type operator & (FileOpenMode::Type a, FileOpenMode::Type b) { return static_cast<FileOpenMode::Type>(static_cast<int>(a) & static_cast<int>(b)); }

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
        ScopedFile(const char* fileName, FileOpenMode::Type mode, int errorCode = 0)
        {
            Initialize(fileName, mode, errorCode);
        }

        ScopedFile(std::string fileName, FileOpenMode::Type mode, int errorCode = 0)
        {
            Initialize(fileName.c_str(), mode, errorCode);
        }

        ~ScopedFile()
        {
            Close();
        }

        void Close()
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
                file = NULL;
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
        void Initialize(const char* fileName, FileOpenMode::Type mode, int errorCode = 0)
        {
            this->errorCode = errorCode;
            this->fileName = fileName;

            std::string charMode = GetCharMode(mode);

            file = fopen(fileName, charMode.c_str());

            if (file == NULL)
            {
                std::string errorMessage("Could not open file ");
                errorMessage
                       .append(fileName)
                       .append(" with mode ")
                       .append(charMode.c_str())
                       .append("\n");
                errorHandler.HandleError(errorMessage.c_str(), errorCode);
            }
        }

        std::string GetCharMode(FileOpenMode::Type mode)
        {
            std::string charMode;

            if ((mode & FileOpenMode::Read) != 0)
            {
                charMode = "r";
            }
            else if ((mode & FileOpenMode::Write) != 0)
            {
                charMode = "w";
            }
            else if ((mode & FileOpenMode::Append) != 0)
            {
                charMode = "a";
            }

            if ((mode & FileOpenMode::Update) != 0)
            {
                charMode += "+";
            }

            if ((mode & FileOpenMode::Binary) != 0)
            {
                charMode += "b";
            }

            return charMode;
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
