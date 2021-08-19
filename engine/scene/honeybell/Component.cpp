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

#include "Component.hpp"

#include "Toy.hpp"

ctHoneybell::ComponentBase::ComponentBase(ConstructContext& ctx, class ToyBase* _toy) {
   pToy = _toy;
   pNextSiblingComponent = NULL;
   _toy->_RegisterComponent(this);
}

ctHoneybell::ComponentBase::~ComponentBase() {
}

ctResults ctHoneybell::ComponentBase::Begin(BeginContext& beginCtx) {
   return CT_SUCCESS;
}

class ctHoneybell::ToyBase* ctHoneybell::ComponentBase::GetToyPtr() {
   return pToy;
}

bool ctHoneybell::ComponentBase::hasTransform() const {
   return false;
}

ctTransform ctHoneybell::ComponentBase::GetWorldTransform() const {
   return ctTransform();
}

void ctHoneybell::ComponentBase::SetWorldTransform(ctTransform& v) {
}

void ctHoneybell::ComponentBase::CopyOwnerTransform() {
   if (!pToy) { return; }
   SetWorldTransform(pToy->GetWorldTransform());
}

ctBoundBox ctHoneybell::ComponentBase::GetWorldBounds() const {
   if (hasTransform()) {
      const ctTransform& transform = GetWorldTransform();
      return ctBoundBox(transform.position, transform.position);
   } else {
      return ctBoundBox(); /* Invalid bounds */
   }
}