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

#include "Toy.hpp"

#include "Scene.hpp"

ctHoneybell::ToyBase::ToyBase(ConstructContext& ctx) {
   transform.position = ctx.spawn.transform.position;
   transform.rotation = ctx.spawn.transform.rotation;
   transform.scale = ctx.spawn.transform.scale;
   aabb = ctBoundBox();
}

ctHoneybell::ToyBase::~ToyBase() {
}

ctResults ctHoneybell::ToyBase::OnBegin(BeginContext& ctx) {
   return CT_SUCCESS;
}

ctResults ctHoneybell::ToyBase::OnTickSerial(TickContext& ctx) {
   return CT_SUCCESS;
}

ctResults ctHoneybell::ToyBase::OnTickParallel(TickContext& ctx) {
   return CT_SUCCESS;
}

ctResults ctHoneybell::ToyBase::OnFrameUpdate(FrameUpdateContext& ctx) {
   return CT_SUCCESS;
}

ctResults ctHoneybell::ToyBase::OnSignal(SignalContext& ctx) {
   return CT_SUCCESS;
}

ctResults ctHoneybell::ToyBase::OnTryPossess(PossessionContext& ctx) {
   return CT_FAILURE_INACCESSIBLE;
}

void ctHoneybell::ToyBase::SetWorldTransform(ctTransform v) {
   transform = v;
}

ctTransform ctHoneybell::ToyBase::GetWorldTransform() {
   return transform;
}

ctBoundBox ctHoneybell::ToyBase::GetAABB() {
   return aabb;
}

void ctHoneybell::ToyBase::SetAABB(ctBoundBox v) {
   aabb = v;
}

ctHandle ctHoneybell::ToyBase::GetIdentifier() {
   return identifier;
}

void ctHoneybell::ToyBase::_SetIdentifier(ctHandle hndl) {
   identifier = hndl;
}

ctHoneybell::ToyBase* ctHoneybell::ToyTypeRegistry::NewToy(ConstructContext& ctx) {
   ZoneScoped;
   uint64_t typeHash = ctxxHash64(ctx.typePath);
   ctAssert(typeHash != 0);
   ToyNewFunction* pCallback = _callbacks.FindPtr(typeHash);
   if (!pCallback) { return NULL; }
   return (*pCallback)(ctx);
}

ctResults ctHoneybell::ToyTypeRegistry::RegisterToyType(const char* typePath,
                                                        ToyNewFunction toyNewFunction) {
   ZoneScoped;
   uint64_t typeHash = ctxxHash64(typePath);
   ctAssert(typeHash != 0);
   if (!_callbacks.Insert(typeHash, toyNewFunction)) { return CT_FAILURE_UNKNOWN; }
   return CT_SUCCESS;
}
