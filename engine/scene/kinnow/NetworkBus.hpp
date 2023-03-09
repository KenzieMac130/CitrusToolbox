/*
   Copyright 2021 MacKenzie Strand

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#pragma once

#include "utilities/Common.h"

typedef struct ctSceneNetworkBusT* ctSceneNetworkBus;

/* client entities: for a client side only object the client will dole out it's entity slots,
server and client entities: for a server side entity the server will be contacted with a reserve request and spawning will be deferred 
server entities: similar to client entities but on the server side */

enum ctResults ctSceneNetworkBusCreate(ctSceneNetworkBus* pBus);
enum ctResults ctSceneNetworkBusDestroy(ctSceneNetworkBus bus);

enum ctResults ctSceneNetworkBusSend(ctSceneNetworkBus bus, uint32_t signalTypeId, size_t packetSize, void* pPacket);