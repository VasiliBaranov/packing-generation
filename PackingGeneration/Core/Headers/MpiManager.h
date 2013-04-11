// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Core_Headers_MpiManager_h
#define Core_Headers_MpiManager_h

#include "Types.h"
#include "Macros.h"

#ifdef PARALLEL
#include "mpi.h"
#else
// Copied from the mpich source. Now we can build the program without including any mpi.h headers.
typedef int MPI_Comm;

typedef int MPI_Datatype;

typedef struct MPI_Status {
    int count;
    int cancelled;
    int MPI_SOURCE;
    int MPI_TAG;
    int MPI_ERROR;

} MPI_Status;
#endif

namespace Core
{
    //Represents a gateway over MPI interface (see http://martinfowler.com/eaaCatalog/gateway.html).
    //When program works in a single-processor mode, it is substituted with a dummy implementation.
    //TODO: rewrite through C++ MPI to make exceptions consistent.
    class MpiManager
    {
    private:
        static MpiManager* instance;

        int numberOfProcesses;
        int processID;
        bool initialized;
        MPI_Comm communicator;
    public:

        ///Singleton pattern.
        static MpiManager* GetInstance();

        /// Initializes the mpi manager.
        void Initialize(int *argc, char ***argv);

        void Finalize();

        /// Returns the number of processes.
        int GetNumberOfProcesses() const;

        /// Returns the process ID.
        int GetCurrentRank() const;

        /// Returns process ID of main processor.
        int GetMasterRank() const;

        /// Tells whether current processor is main processor.
        bool IsMaster() const;

        /// Returns a universal MPI-time in seconds.
        FLOAT_TYPE GetTime() const;

        //Mpi wrappers. All of them use the internal topology communicator. 
        //See MPI reference for method descriptions.

        ///Wrapper over Mpi send-receive.
        int SendReceive(void* sendBuffer, int sendCount, MPI_Datatype sendType,
                         int destinationRank, int sendTag, void* receiveBuffer, int receiveCount,
                         MPI_Datatype receiveType, int sourceRank, int receiveTag, MPI_Status* status);

        int Receive(void* buffer, int count, MPI_Datatype dataType,
            int sourceRank, int tag, MPI_Status* status);

        int Send(void* buffer, int count, MPI_Datatype dataType,
            int destinationRank, int tag);

        int Gather(void* sendBuffer, int sendCount, MPI_Datatype sendType,
                    void* receiveBuffer, int receiveCount, MPI_Datatype receiveType, int root);

        //a wrapper over MPI_GatherV function
        int GatherVaryingBuffers(void* sendBuffer, int sendCount, MPI_Datatype sendType,
                    void* receiveBuffer, int* receiveCounts, MPI_Datatype receiveType, int root);

        int Barrier();

    private:
        MpiManager(void);

        virtual ~MpiManager(void);

        void CheckInitialization() const;

        DISALLOW_COPY_AND_ASSIGN(MpiManager);
    };
}

#endif /* Core_Headers_MpiManager_h */
