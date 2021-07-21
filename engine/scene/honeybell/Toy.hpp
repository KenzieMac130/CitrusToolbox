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
#include "Component.hpp"

namespace ctHoneybell {

struct CT_API SpawnData {
   ctVec3 position;
   ctQuat rotation;
   ctVec3 scale;
   const char* command;
}; /* Position, Rotation, Scale, Command */

struct CT_API PrefabData {
   ctJSONReader* pJson;
}; /* File paths, key-values, gltf, etc */

struct CT_API ConstructContext {
   class Scene* pOwningScene;
   class ComponentRegistry* pComponentRegistry;
   const char* typePath;
   SpawnData spawn;
   PrefabData prefab;
};

/* Callback registry */
struct CT_API BeginContext {
   bool canTickSerial;
   bool canTickParallel;
   bool canFrameUpdate;
};

/* Time, Signal output */
struct CT_API TickContext {
   double deltaTime;
};

struct CT_API TickParallelContext {
   double deltaTime;
};

struct CT_API FrameUpdateContext {
   double deltaTime;
};

struct CT_API SignalContext {
   const char* path;
   ctVec3 position;
   float value;
   uint32_t flags;
   ctHandle originToy;
};

/* A toy is a scene object which can be dynamically updated by game code.
 * Gameplay will often create different types of toys that serve different purposes
 * in the game, toys can use inheritence and components to construct functionality.
 *
 * In other software this might similar to (entities, gameobjects, actors, etc.)
 * The toy may differ in approach in a few distinct ways:
 *
 * Nameless:
 * Toys do not have any name assosiation with them by default, a handle is used to
 * recieve a unique reference to them. Toys can be optionally tagged with text
 * in the scene system to make locating known toys by global or scripts easier.
 *
 * World space all the time:
 * Rather than battle different coordinate systems and parent-child relationships
 * the toy assumes a world space output, the gameplay programmer is responsible for
 * Creating and maintaining transform relationships using their own math. This may
 * seem like a complication, but in other engines the code will often have to needlessly
 * compute or invert the chain of transforms in order to get their desired coordinate
 * space, making it actually more complex to work inside of an existing transform tree.
 * Without a forced transform tree the user will only ever get transforms they ask for.
 *
 * Isolated objects:
 * Toys do not interact directly with the outside world, they should be designed like
 * their own program with their own isolated unit of execution and recieve and send
 * impulses via the signal system. Think of a somone trying to pick up an apple, the
 * brain does NOT have a direct reference to the etherial presense of the apple they
 * are trying to pick up, they are simply reacting to external stimuli. In this case
 * the apple may be a source of a constant signal to nearby toys which the person can
 * decide to act upon and send a request to pick up the object. The pickup in response
 * may accept the request, query it's scene removal and send a signal to the toy that
 * it should be added to the item's inventory. This transaction may happen over multiple
 * Ticks and the signal model might seem limiting, but just remember, nothing in real life
 * happens instantaneously either. Complex/tightly locked communication will be tackled
 * by another system at a later date.
 *
 * The key objective of the toy/component system isn't to be the fastest scene system in
 * the world that tunnels the gameplay programmer into to a purely data oriented mindset.
 * The goal is to give the programmer a starting point they can use to create interesting
 * gameplay features in an easy code structure with decent performance and minimal mental
 * overhead. This is why the class is referred to the name "toy": It should be fun to use.
 */
class CT_API ToyBase {
public:
   ToyBase(ConstructContext& ctx);
   virtual ~ToyBase();

   /* ---- Game Logic ---- */
   /* Called when spawning to register callbacks and components (threadsafe) */
   virtual ctResults OnBegin(BeginContext& ctx);
   /* OnTick is called every tick in an undefined but threadsafe order */
   virtual ctResults OnTickSerial(TickContext& ctx);
   /* OnTickParallel is called every tick and run in the job system */
   virtual ctResults OnTickParallel(TickParallelContext& ctx);
   /* OnUIUpdate is called every frame and should be used for UI/debug draw/input */
   virtual ctResults OnFrameUpdate(FrameUpdateContext& ctx);
   /* OnSignal is called to inform the toy about the outside world (non-threaddsafe) */
   virtual ctResults OnSignal(SignalContext& ctx);

   /* ---- Getters/Setters ---- */
   /* Get a unique handle identifier for this object */
   ctHandle GetIdentifier();
   /* Get world position */
   virtual ctVec3 GetWorldPosition();
   /* Set world position */
   virtual void SetWorldPosition(ctVec3 v);
   /* Get world rotation */
   virtual ctQuat GetWorldRotation();
   /* Set world rotation */
   virtual void SetWorldRotation(ctQuat v);
   /* Get world scale */
   virtual ctVec3 GetWorldScale();
   /* Set world scale */
   virtual void SetWorldScale(ctVec3 v);
   /* Get bounds (pre-transform) */
   virtual ctBoundBox GetAABB();
   /* Set bounds (pre-transform) */
   virtual void SetAABB(ctBoundBox v);

   /* NEVER CALL THIS DIRECTLY! */
   void _SetIdentifier(ctHandle hndl);

private:
   ctVec3 position;
   ctHandle identifier;
   ctQuat rotation;
   ctVec3 scale;
   ctBoundBox aabb;
};

/* Function to allocate */
typedef ToyBase* (*ToyNewFunction)(ConstructContext&);

/* Registers all toy factory instances */
class CT_API ToyTypeRegistry {
public:
   ToyBase* NewToy(ConstructContext& ctx);

   /* Register a new component factory with this registry */
   ctResults RegisterToyType(const char* typePath, ToyNewFunction toyNewFunction);

private:
   ctHashTable<ToyNewFunction, uint64_t> _callbacks;
};

}