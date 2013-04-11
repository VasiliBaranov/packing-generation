// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#include "../Headers/MpiManager.h"
#include "../Headers/Exceptions.h"
#include <vector>

using namespace std;

namespace Core
{
#ifdef PARALLEL

    MpiManager* MpiManager::instance = new MpiManager();

    MpiManager::MpiManager(void)
    {
        numberOfProcesses = 1;
        processID = 0;
    }

    MpiManager::~MpiManager(void)
    {
        Finalize();
    }

    void MpiManager::Initialize(int *argc, char ***argv)
    {
        int argumentsInitialized = MPI_Init(argc, argv);
        initialized = (argumentsInitialized == 0);

        MPI_Comm_size(MPI_COMM_WORLD, &numberOfProcesses);
        MPI_Comm_rank(MPI_COMM_WORLD, &processID);
        communicator = MPI_COMM_WORLD;
    }

    void MpiManager::Finalize()
    {
        if (initialized) 
        {
            MPI_Finalize();
            initialized = false;
        }
    }

    int MpiManager::GetNumberOfProcesses() const
    {
        return numberOfProcesses;
    }

    int MpiManager::GetCurrentRank() const
    {
        return processID;
    }

    int MpiManager::GetMasterRank() const
    {
        return 0;
    }

    bool MpiManager::IsMaster() const
    {
        return processID == 0;
    }

    FLOAT_TYPE MpiManager::GetTime() const
    {
        CheckInitialization();
        return MPI_Wtime();
    }

    MpiManager* MpiManager::GetInstance()
    {
        return instance;
    }

    int MpiManager::SendReceive(void* sendBuffer, int sendCount, MPI_Datatype sendType,
                     int destinationRank, int sendTag, void* receiveBuffer, int receiveCount,
                     MPI_Datatype receiveType, int sourceRank, int receiveTag, MPI_Status* status)
    {
        CheckInitialization();
        return MPI_Sendrecv(sendBuffer, sendCount, sendType,
                     destinationRank, sendTag, receiveBuffer, receiveCount,
                     receiveType, sourceRank, receiveTag, communicator, status);
    }

    int MpiManager::Receive(void* buffer, int count, MPI_Datatype dataType,
        int sourceRank, int tag, MPI_Status *status)
    {
        CheckInitialization();
        return MPI_Recv(buffer, count, dataType, sourceRank, tag, communicator, status);
    }

    int MpiManager::Send(void* buffer, int count, MPI_Datatype dataType,
        int destinationRank, int tag)
    {
        CheckInitialization();
        return MPI_Send(buffer, count, dataType, destinationRank, tag, communicator);
    }

    int MpiManager::Gather(void* sendBuffer, int sendCount, MPI_Datatype sendType,
            void* receiveBuffer, int receiveCount, MPI_Datatype receiveType, int root)
    {
        CheckInitialization();
        return MPI_Gather(sendBuffer, sendCount, sendType,
                            receiveBuffer, receiveCount, receiveType, root, communicator);
    }

    //a wrapper over MPI_GatherV function
    int MpiManager::GatherVaryingBuffers(void* sendBuffer, int sendCount, MPI_Datatype sendType,
                    void* receiveBuffer, int* receiveCounts, MPI_Datatype receiveType, int root)
    {
        CheckInitialization();

        vector<int> displacements(numberOfProcesses);
        displacements[0] = 0;
        for (int i = 1; i < numberOfProcesses; i++)
        {
            displacements[i] = displacements[i - 1] + receiveCounts[i - 1];
        }

        int* displacementsArray = &displacements[0];
        int result = MPI_Gatherv(sendBuffer, sendCount, sendType,
                            receiveBuffer, receiveCounts, displacementsArray, receiveType, root, communicator);

        return result;
    }

    int MpiManager::Barrier()
    {
        CheckInitialization();

        return MPI_Barrier(communicator);
    }

    void MpiManager::CheckInitialization() const
    {
        if (!initialized)
        {
            throw InvalidOperationException("Mpi not initialized.");
        }
    }

//no PARALLEL
#else

    MpiManager* MpiManager::instance = new MpiManager();

    MpiManager::MpiManager(void)
    {
        numberOfProcesses = 1;
        processID = 0;
        communicator = 0;
    }

    MpiManager::~MpiManager(void)
    {
        Finalize();
    }

    void MpiManager::Initialize(int *argc, char ***argv)
    {
    }

    void MpiManager::Finalize()
    {
    }

    int MpiManager::GetNumberOfProcesses() const
    {
        return numberOfProcesses;
    }

    int MpiManager::GetCurrentRank() const
    {
        return processID;
    }

    int MpiManager::GetMasterRank() const
    {
        return 0;
    }

    bool MpiManager::IsMaster() const
    {
        return processID == 0;
    }

    FLOAT_TYPE MpiManager::GetTime() const
    {
        //TODO: enhance
        return 0;
    }

    MpiManager* MpiManager::GetInstance()
    {
        return instance;
    }

    int MpiManager::SendReceive(void* sendBuffer, int sendCount, MPI_Datatype sendType,
                     int destinationRank, int sendTag, void* receiveBuffer, int receiveCount,
                     MPI_Datatype receiveType, int sourceRank, int receiveTag, MPI_Status* status)
    {
        return 0;
    }

    int MpiManager::Receive(void* buffer, int count, MPI_Datatype dataType,
        int sourceRank, int tag, MPI_Status *status)
    {
        return 0;
    }

    int MpiManager::Send(void* buffer, int count, MPI_Datatype dataType,
        int destinationRank, int tag)
    {
        return 0;
    }

    int MpiManager::Gather(void* sendBuffer, int sendCount, MPI_Datatype sendType,
            void* receiveBuffer, int receiveCount, MPI_Datatype receiveType, int root)
    {
        return 0;
    }

    //a wrapper over MPI_GatherV function
    int MpiManager::GatherVaryingBuffers(void* sendBuffer, int sendCount, MPI_Datatype sendType,
                    void* receiveBuffer, int* receiveCounts, MPI_Datatype receiveType, int root)
    {
        return 0;
    }

    int MpiManager::Barrier()
    {
        return 0;
    }

#endif

}
