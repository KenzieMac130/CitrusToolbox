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
#include "core/ModuleBase.hpp"

namespace ctHoneybell {

/* Base class for a component which can be used by a toy
 * Components should contain internal data, the results of queries and any external
 * state updates MUST (by gameplay assumption) be performed in a threadsafe manner.
 * Wherever possible try to deffer external state updates to factory updates. */
class CT_API ComponentBase {
public:
   ComponentBase(class ComponentFactoryBase* _factory, class ToyBase* _toy);
   virtual ~ComponentBase();

   /* Pointer to the tow which owns the component */
   class ToyBase* GetToyPtr();
   /* Pointer to the factory which created the component */
   class ComponentFactoryBase* GetFactoryPtr();

   /* ---- Component World Transforms ---- */
   /* Does the component have transforms? */
   virtual bool hasTransform() const;
   /* Get component world transform (if applicable) */
   virtual ctTransform GetWorldTransform() const;
   /* Set component world transform (if applicable) */
   virtual void SetWorldTransform(ctTransform& v);
   /* Copy the world transforms of the owning toy */
   void CopyOwnerTransform();

   /* Get the bounds of the component (if applicable) */
   virtual ctBoundBox GetWorldBounds() const;

protected:
   class ToyBase* pToy;
   class ComponentFactoryBase* pFactory;
};

/* Base class for which all components of a given type can be created from.
 * Factories can manage batch updates of components and their memory allocation.
 * Major rule is that memory allocation should prevent false sharing and never relocate.
 */
class CT_API ComponentFactoryBase : public ctModuleBase {
public:
   virtual ctResults Startup();
   virtual ctResults Shutdown();

   virtual ComponentBase* NewComponent(class ToyBase* _owner);
   virtual void DeleteComponent(ComponentBase* _component);
};

/* A toy can use this to reference components and auto cleanup */
template<class T>
class CT_API ComponentPtr {
public:
   ComponentPtr() {
      ptr = NULL;
   }
   inline bool isValid() {
      return ptr != NULL;
   }
   inline void operator=(T* _ptr) {
      ptr = _ptr;
   }
   inline T* operator->() {
      return ptr;
   }
   inline ~ComponentPtr() {
      if (ptr) {
         ComponentFactoryBase* pFactory = ptr->GetFactoryPtr();
         if (pFactory) {}
         pFactory->DeleteComponent(ptr);
         ptr = NULL;
      }
   }

private:
   T* ptr;
};

/* Registers all component factory instances, this is handled by the scene */
class CT_API ComponentRegistry {
public:
   /* Create a new component of a given type */
   template<class CMPTYPE>
   inline CMPTYPE* NewComponent(class ToyBase* _owner);

   /* Register a new component factory with this registry */
   template<class CMPTYPE>
   inline ctResults RegisterComponentFactory(ComponentFactoryBase* pComponentFactory);

private:
   ctHashTable<ComponentFactoryBase*, size_t> _factoriesTyped;
};

template<class CMPTYPE>
inline ctResults
ComponentRegistry::RegisterComponentFactory(ComponentFactoryBase* pComponentFactory) {
   ZoneScoped;
   size_t typeHash = typeid(CMPTYPE).hash_code();
   ctAssert(typeHash != 0);
   _factoriesTyped.Insert(typeHash, pComponentFactory);
   return CT_SUCCESS;
}

template<class CMPTYPE>
inline CMPTYPE* ComponentRegistry::NewComponent(ToyBase* _owner) {
   ZoneScoped;
   size_t typeHash = typeid(CMPTYPE).hash_code();
   ctAssert(typeHash != 0);
   ComponentFactoryBase** ppFactory = _factoriesTyped.FindPtr(typeHash);
   if (!ppFactory) { return NULL; }
   if (!*ppFactory) { return NULL; }
   return (CMPTYPE*)((*ppFactory)->NewComponent(_owner));
}
}