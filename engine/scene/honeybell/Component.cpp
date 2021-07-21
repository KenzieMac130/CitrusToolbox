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

ctHoneybell::ComponentBase::ComponentBase(ComponentFactoryBase* _factory,
                                          class ToyBase* _toy) {
   pToy = _toy;
   pFactory = _factory;
}

ctHoneybell::ComponentBase::~ComponentBase() {
}

class ctHoneybell::ToyBase* ctHoneybell::ComponentBase::GetToyPtr() {
   return pToy;
}

class ctHoneybell::ComponentFactoryBase* ctHoneybell::ComponentBase::GetFactoryPtr() {
   return pFactory;
}

bool ctHoneybell::ComponentBase::hasTransform() {
   return false;
}

ctVec3 ctHoneybell::ComponentBase::GetWorldPosition() {
   return ctVec3(0.0f);
}

void ctHoneybell::ComponentBase::SetWorldPosition(ctVec3 v) {
}

ctQuat ctHoneybell::ComponentBase::GetWorldRotation() {
   return ctQuat();
}

void ctHoneybell::ComponentBase::SetWorldRotation(ctQuat v) {
}

ctVec3 ctHoneybell::ComponentBase::GetWorldScale() {
   return ctVec3(1.0f);
}

void ctHoneybell::ComponentBase::SetWorldScale(ctVec3 v) {
}

void ctHoneybell::ComponentBase::CopyOwnerTransform() {
   if (!pToy) { return; }
   SetWorldPosition(pToy->GetWorldPosition());
   SetWorldRotation(pToy->GetWorldRotation());
   SetWorldScale(pToy->GetWorldScale());
}

ctBoundBox ctHoneybell::ComponentBase::GetAABB() {
   if (hasTransform()) {
      return ctBoundBox(ctVec3(0.0f), ctVec3(0.0f));
   } else {
      return ctBoundBox(); /* Invalid bounds */
   }
}

ctResults ctHoneybell::ComponentFactoryBase::Startup() {
   return CT_SUCCESS;
}

ctResults ctHoneybell::ComponentFactoryBase::Shutdown() {
   return CT_SUCCESS;
}

ctHoneybell::ComponentBase*
ctHoneybell::ComponentFactoryBase::NewComponent(class ToyBase* _owner) {
   /* If this ever get's called you probably messed up */
   ctDebugWarning("Pure component base class created.");
   return new ComponentBase(this, _owner);
}

void ctHoneybell::ComponentFactoryBase::DeleteComponent(ComponentBase* _component) {
   delete _component;
}
