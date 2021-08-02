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

#include "DebugShapeComponent.hpp"

#include "im3d/im3d.h"

ctHoneybell::DebugShapeComponent::DebugShapeComponent(ComponentFactoryBase* _factory,
                                                      ToyBase* _toy) :
    ComponentBase::ComponentBase(_factory, _toy) {
   rgba = CT_COLOR_WHITE;
}

bool ctHoneybell::DebugShapeComponent::hasTransform() const {
   return true;
}

void ctHoneybell::DebugShapeComponent::SetWorldTransform(ctTransform& v) {
   transform = v;
}

ctTransform ctHoneybell::DebugShapeComponent::GetWorldTransform() const {
   return transform;
}

void ctHoneybell::DebugShapeComponent::SetLocalBounds(ctBoundBox& v) {
   bounds = v;
}

ctBoundBox ctHoneybell::DebugShapeComponent::GetLocalBounds() const {
   return bounds;
}

ctHoneybell::ComponentBase*
ctHoneybell::DebugShapeComponentFactory::NewComponent(ToyBase* _owner) {
   ctHoneybell::ComponentBase* result = new DebugShapeComponent(this, _owner);
   shapes.Append((DebugShapeComponent*)result);
   return result;
}

void ctHoneybell::DebugShapeComponentFactory::DeleteComponent(ComponentBase* _component) {
   shapes.Remove((DebugShapeComponent*)_component);
}

void ctHoneybell::DebugShapeComponentFactory::Im3dDrawAll() {
#if CITRUS_IM3D
   Im3d::PushDrawState();
   for (size_t i = 0; i < shapes.Count(); i++) {
      if (shapes[i]) {
         const DebugShapeComponent& shape = *shapes[i];
         const ctTransform& transform = shape.GetWorldTransform();
         const ctBoundBox bounds = shape.GetLocalBounds();
         Im3d::SetColor(shape.rgba.r, shape.rgba.g, shape.rgba.b, shape.rgba.a);
         ctMat4 mat = ctMat4Identity();
         ctMat4Translate(mat, transform.position);
         ctMat4Rotate(mat, transform.rotation);
         ctMat4Scale(mat, transform.scale);
         Im3d::PushMatrix(ctMat4ToIm3d(mat));
         Im3d::DrawAlignedBoxFilled(ctVec3ToIm3d(bounds.min), ctVec3ToIm3d(bounds.max));
         Im3d::PopMatrix();
      }
   }
   Im3d::PopDrawState();
#endif
}
