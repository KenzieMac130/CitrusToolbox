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
#include "Toy.hpp"

namespace ctHoneybell {

class CT_API ComponentBase {
public:
   friend class ToyBase;
   ComponentBase(ConstructContext& ctx, class ToyBase* _toy);
   virtual ~ComponentBase();

   /* Called when the component becomes an active participant in the scene */
   virtual ctResults Begin(BeginContext& beginCtx);

   /* Pointer to the tow which owns the component */
   class ToyBase* GetToyPtr();

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

   /* Get the name of the component type */
   virtual const char* GetTypeName() = 0;

   /* Overload for component allocator */
   // void* operator new(size_t size);
   // void operator delete(void* ptr);

protected:
   ComponentBase* pNextSiblingComponent;
   class ToyBase* pToy;
};
}