// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Generation_PackingServices_Headers_IContextDependentService_h
#define Generation_PackingServices_Headers_IContextDependentService_h

namespace Model { class ModellingContext; }

namespace PackingServices
{
    class IContextDependentService
    {
    public:
        virtual void SetContext(const Model::ModellingContext& context) = 0;

        virtual ~IContextDependentService(){ };
    };
}


#endif /* Generation_PackingServices_Headers_IContextDependentService_h */
